#include <network/nodeclient.h>

namespace network {
/********************websocket_endpoint********************/
set<string> initset()
{
	set<string> tmp;
	return tmp;
}
int websocket_endpoint::p2p_conn_count = 0;
std::set<string> websocket_endpoint::m_addr_list(initset());
websocket_endpoint::websocket_endpoint():m_next_id(0),seed_id(-1), is_full(false){
	m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
	m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
	m_endpoint.init_asio();
	m_endpoint.start_perpetual();
	m_endpoint.set_reuse_addr(true);

	/*m_endpoint.listen(2200);
	boost::system::error_code e;
	m_endpoint.get_local_endpoint(e);*/
	///？这里为什么要开线程没看懂
	m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
}

websocket_endpoint::~websocket_endpoint() {
	m_endpoint.stop_perpetual();
	lock_guard<mutex> guard(m_connection_lock);
	for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
		if (it->second->get_status() != OPEN) {
			// Only close open connections
			continue;
		}

		std::cout << "> Closing connection " << it->second->get_id() << std::endl;

		websocketpp::lib::error_code ec;
		m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
		if (ec) {
			std::cout << "> Error closing connection " << it->second->get_id() << ": "
				<< ec.message() << std::endl;
		}
	}

	m_thread->join();
}

void websocket_endpoint::req_holes(endpoint_addr_t addr) {
	Json::Value root;
	root["head"] = REQUESTADDR;
	root["body"]["addr"] = addr;

	auto&& hdl = m_connection_list.find(seed_id)->second->get_hdl();
	m_endpoint.send(hdl, root.toStyledString(), websocketpp::frame::opcode::value::text);
}

void websocket_endpoint::init() {
	int i = 0;
	seed_id = connect(SEED_URI_ARR[i], i);
	if (seed_id < 0) {
		tangle::log::error(__FUNCTION__) << "init failed: no seed server";
		return;
	}
}

int websocket_endpoint::connect(std::string const & uri, int seed_index) {
	websocketpp::lib::error_code ec;
	//a connection request is created
	client::connection_ptr con = m_endpoint.get_connection(uri, ec);
	con->get_port();
	if (ec) {
		std::cout << "> Connect initialization error: " << ec.message() << std::endl;
		return -1;
	}

	lock_guard<mutex> guard(m_connection_lock);
	int new_id = m_next_id++;
	connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(new_id, con->get_handle(), uri, seed_index);
	m_connection_list[new_id] = metadata_ptr;
	//metadata_ptr->set_push_addr_handler();
	con->set_socket_init_handler(websocketpp::lib::bind(
		&connection_metadata::init_socket,
		metadata_ptr,
		CLIENT_PORT,
		websocketpp::lib::placeholders::_1,
		websocketpp::lib::placeholders::_2
		
	));
	con->set_open_handler(websocketpp::lib::bind(
		&connection_metadata::on_open,
		metadata_ptr,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
	));
	con->set_fail_handler(websocketpp::lib::bind(
		&connection_metadata::on_fail,
		metadata_ptr,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
	));
	con->set_close_handler(websocketpp::lib::bind(
		&connection_metadata::on_close,
		metadata_ptr,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
	));
	con->set_message_handler(websocketpp::lib::bind(
		&connection_metadata::on_message,
		metadata_ptr,
		websocketpp::lib::placeholders::_1,
		websocketpp::lib::placeholders::_2
	)); 
	client::timer_ptr m_timer_ptr =con->set_timer(PING_PERIOD_MS,websocketpp::lib::bind(
		&connection_metadata::timer,
		metadata_ptr,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
	));
	con->set_pong_handler(websocketpp::lib::bind(
		&connection_metadata::on_pong,
		metadata_ptr,
		websocketpp::lib::placeholders::_1,
		websocketpp::lib::placeholders::_2
	));
	m_endpoint.connect(con);
	return new_id;
}

void websocket_endpoint::close(int id, websocketpp::close::status::value code, std::string reason) {
	websocketpp::lib::error_code ec;
	
	lock_guard<mutex> guard(m_connection_lock);
	con_list::iterator metadata_it = m_connection_list.find(id);
	if (metadata_it == m_connection_list.end()) {
		std::cout << "> No connection found with id " << id << std::endl;
		return;
	}
	m_endpoint.close(metadata_it->second->get_hdl(), code, reason, ec);//关闭连接
	metadata_it->second.reset(); //放弃托管
	m_connection_list.erase(id);
	if (ec) {
		std::cout << "> Error initiating close: " << ec.message() << std::endl;
	}
}

void websocket_endpoint::send(int id, std::string message) {
	websocketpp::lib::error_code ec;

	con_list::iterator metadata_it = m_connection_list.find(id);
	if (metadata_it == m_connection_list.end()) {
		std::cout << "> No connection found with id " << id << std::endl;
		return;
	}

	m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
	if (ec) {
		std::cout << "> Error sending message: " << ec.message() << std::endl;
		return;
	}

	metadata_it->second->record_sent_message(message);
}

connection_metadata::ptr websocket_endpoint::get_metadata(int id) const {
	con_list::const_iterator metadata_it = m_connection_list.find(id);
	if (metadata_it == m_connection_list.end()) {
		return connection_metadata::ptr();
	}
	else {
		return metadata_it->second;
	}
}
}