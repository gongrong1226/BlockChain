#include <network/nodeserver.h>
namespace network {

p2p_server::p2p_server():timer_ping(m_timer_manager), timer_clear_addr(m_timer_manager) {
	// Initialize Asio Transport
	m_server.init_asio();

	// Register handler callbacks
	m_server.set_open_handler(bind(&p2p_server::on_open, this, _1));
	m_server.set_close_handler(bind(&p2p_server::on_close, this, _1));
	m_server.set_message_handler(bind(&p2p_server::on_message, this, _1, _2));
	m_server.set_ping_handler(bind(&p2p_server::recv_ping, this, _1, _2));
	m_server.set_pong_handler(bind(&p2p_server::recv_pong, this, _1, _2));
	m_server.set_pong_timeout_handler(bind(&p2p_server::pong_timeout, this, _1, _2));
	m_server.set_reuse_addr(true);

	timer_ping.init(bind(&p2p_server::brocastPing, this), PING_INTERVAL_MS);
	//timer_ping.stop();
	timer_clear_addr.init(bind(&p2p_server::clrTimeoutCnnt,this), 1000);
}


p2p_server::~p2p_server() {

}


void p2p_server::brocastPing() {
	for (auto hdl = m_connections.begin(); hdl != m_connections.end(); ++hdl) {
		std::cout << __FUNCTION__ << std::endl;
		m_server.ping(*hdl, "ping test\n");
		//m_server.send(*it, a.msg);
	}
}

//应该改成在数据库中的修改，最好是有自动更新
void p2p_server::clrTimeoutCnnt() {
	lock_guard<mutex> guard(m_timer_lock);
	for (auto it = map_connet_timer.begin(); it != map_connet_timer.end(); it++) {
		it->second -= 1;
		std::cout << __FUNCTION__ << "seconde: " << it->second << std::endl;
		if (it->second <= 0) {
			std::cout << __FUNCTION__ << "timeout " << it->second << std::endl;
			lock_guard<mutex> guard1(m_connection_lock);
			auto hdl = map_conn_addr[it->first];
			map_conn_addr.erase(it->first);
			map_connet_timer.erase(it->first);
			m_connections.erase(hdl);
		}
	}
}

void p2p_server::run(uint16_t port) {
	// listen on specified port
	//m_server.listen(port); //[::ffff:127.0.0.1]:55774
	m_server.set_reuse_addr(true);
	m_server.listen(boost::asio::ip::tcp::v4(), port);//127.0.0.1:55772
	// Start the server accept loop
	m_server.start_accept();

	// Start the ASIO io_service run loop
	try {
		m_timer_manager.start();
		m_server.run();
	}
	catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	}
}

bool p2p_server::recv_ping(connection_hdl hdl, std::string payload) {
	std::cout << "receive a ping\n" << payload << std::endl;
	{
		lock_guard<mutex> guard(m_action_lock);
		m_actions.push(action(RECV_PING, hdl));
	}
	return true;//true for replaying pong;
}

void p2p_server::recv_pong(connection_hdl hdl, std::string payload) {
	{
		lock_guard<mutex> guard(m_action_lock);
		m_actions.push(action(RECV_PONG, hdl));
	}
	m_action_cond.notify_one();
}

void p2p_server::pong_timeout(connection_hdl hdl, std::string payload) {
	{
		lock_guard<mutex> guard(m_action_lock);
		m_actions.push(action(TIMEOUT, hdl));
	}
	m_action_cond.notify_one();
}

void p2p_server::on_open(connection_hdl hdl) {
	{
		lock_guard<mutex> guard(m_action_lock);
		m_actions.push(action(SUBSCRIBE, hdl));
	}
	m_action_cond.notify_one();

}

void p2p_server::on_close(connection_hdl hdl) {
	{
		//作用域结束自动上锁、解锁
		lock_guard<mutex> guard(m_action_lock);
		m_actions.push(action(UNSUBSCRIBE, hdl));
	}
	m_action_cond.notify_one();
}

void p2p_server::on_message(connection_hdl hdl, server::message_ptr msg) {
	// queue message up for sending by processing thread
	{
		lock_guard<mutex> guard(m_action_lock);

		Json::Reader reader;
		Json::Value recv_js;
		if (reader.parse(msg->get_payload(), recv_js)) {
			std::cout << "recv: " << recv_js.toStyledString() << std::endl;
			m_actions.push(action(MESSAGEJSON, hdl, recv_js));
		}
		else {
			std::cout << "recv(not json): " << msg->get_payload() << std::endl;
			m_actions.push(action(MESSAGESTR, hdl, msg));
		}
	}
	m_action_cond.notify_one();
}

void p2p_server::process_messages() {
	while (1) {
		//定义action锁，此刻并没有上锁？
		unique_lock<mutex> lock(m_action_lock);
		while (m_actions.empty()) {
			m_action_cond.wait(lock);
		}
		action a = m_actions.front();
		server::connection_ptr s = m_server.get_con_from_hdl(a.hdl);
		endpoint_addr_t addr = s->get_remote_endpoint();
		m_actions.pop();
		lock.unlock();			//action解锁

		if (a.type == SUBSCRIBE) {
			lock_guard<mutex> guard(m_connection_lock);
			lock_guard<mutex> guard1(m_timer_lock);
			map_conn_addr.insert(map_conn_addr_t::value_type(addr, a.hdl));//连接句柄和地址
			map_connet_timer.insert(map_connet_timer_t::value_type(addr, TIME_OUT_SECONDS));//连接超时初始化
			m_connections.insert(a.hdl);
		}
		else if (a.type == UNSUBSCRIBE) {
			lock_guard<mutex> guard(m_connection_lock);
			lock_guard<mutex> guard1(m_timer_lock);
			map_conn_addr.erase(addr);
			map_connet_timer.erase(addr);
			m_connections.erase(a.hdl);
		}
		else if (a.type == MESSAGESTR) {
			lock_guard<mutex> guard(m_connection_lock);
			con_list::iterator it;
			for (it = m_connections.begin(); it != m_connections.end(); ++it) {
				m_server.send(*it, a.msg);
			}
		}
		else if (a.type == MESSAGEJSON) {
			lock_guard<mutex> guard(m_connection_lock);
			{
				lock_guard<mutex> guard(m_request_lock);
				auto& json = a.recv_json;
				m_requests.push(std::make_pair(addr, json));
			}
			m_request_cond.notify_one();
		}
		else if (a.type == RECV_PONG) { //收到pong
			lock_guard<mutex> guard(m_connection_lock);
			lock_guard<mutex> guard1(m_timer_lock);
			map_connet_timer[addr] = TIME_OUT_SECONDS;

		}
		else if (a.type == RECV_PING) { //收到ping
			lock_guard<mutex> guard(m_connection_lock);
			lock_guard<mutex> guard1(m_timer_lock);
			map_connet_timer[addr] = TIME_OUT_SECONDS;
		}
		else if (a.type == TIMEOUT) { //接收pong超时
			lock_guard<mutex> guard(m_connection_lock);
			lock_guard<mutex> guard1(m_timer_lock);
			//直接删除连接，不给机会
			map_conn_addr.erase(addr);
			map_connet_timer.erase(addr);
			m_connections.erase(a.hdl);

		}
		else {
			// undefined.
		}
	}
}

void p2p_server::process_json() {
	while (true) {
		//定义action锁，此刻并没有上锁？已经上锁，wait的时候会解锁
		unique_lock<mutex> lock(m_request_lock);
		while (m_requests.empty()) {
			m_request_cond.wait(lock);
		}
		request req = m_requests.front();
		m_requests.pop();
		lock.unlock();

		const auto& sour_addr = req.first;
		const Json::Value& root = req.second;
		//auto& sour_hdl = map_conn_addr.find(sour_addr)->second;
		const auto& sour_hdl = map_conn_addr[sour_addr];

		request_type reqCode = request_type(root["head"].asInt());
		switch (reqCode)
		{
		case REQUESTADDR: {
			Json::Value responsJson;
			lock_guard<mutex> guard(m_connection_lock);
			responsJson["head"] = RESPONSADDR;
			auto i = map_conn_addr.begin();
			std::for_each(map_conn_addr.begin(), map_conn_addr.end(),
				[&](std::pair<endpoint_addr_t, connection_hdl> addr) {
				//static int index = 0;
				responsJson["body"]["addr"].append(addr.first);
			});
			//返回IP地址
			m_server.send(sour_hdl, responsJson.toStyledString(), websocketpp::frame::opcode::value::text);
			break;
		}
		case RESPONSADDR: { //收到返回的地址。P2PServer暂时不用

			break;
		}
		case REQUESTHOLES: {
			Json::Value addrJson = root["body"]["addr"];
			Json::Value requestJson;
			requestJson["head"] = REQUESTHOLES;
			std::string&& requestStr = requestJson.asString();
			for (auto i = 0; i < addrJson.size(); i++)//遍历数组[]
			{
				for (auto sub = addrJson[i].begin(); sub != addrJson[i].end(); sub++)//遍历对象{}
				{
					endpoint_addr_t addr = addrJson[i][sub.name()].asString();
					connection_hdl dest_hdl;
					{
						lock_guard<mutex> guard(m_connection_lock);
						//dest_hdl = map_conn_addr.find(addr)->second; //得到句柄
						dest_hdl = map_conn_addr[addr];
					}
					//传送打洞请求
					m_server.send(dest_hdl, requestStr, websocketpp::frame::opcode::value::text);
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

}