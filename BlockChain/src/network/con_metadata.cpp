#include <network/con_metadata.h>

namespace network {

void connection_metadata::init_socket(uint16_t port, connection_hdl hdl, lib::asio::ip::tcp::socket& socket){
	socket.open(boost::asio::ip::tcp::v4());
	//这里的endpoint和client m_endpoint好像是两个概念？
	auto s = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port);
	socket.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
}

void connection_metadata::on_open(client * c, websocketpp::connection_hdl hdl) {
	m_status = OPEN;
	boost::system::error_code e;
	auto s = c->get_local_endpoint(e);
	cout << "[address] " << s.address() << ":" << s.port() << endl;
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	//con->add_subprotocol();
	cout << "[host]" << con->get_host() << endl; //127.0.0.1
	auto uri = con->get_uri();
	uri->get_host_port();
	uri->get_port();
	
	cout << "[get_host_port()] " << uri->get_host_port() << endl;
	cout << "[get_port()] " << uri->get_port() << endl;
	
	
	m_server = con->get_response_header("Server");
	if (seed_index_ >= 0) { //是种子连接就请求地址
		req_all_addr(c);
	}
}

void connection_metadata::req_all_addr(client * c) {
	if (seed_index_ < 0) {
		tangle::log::error(__FUNCTION__) << "this connection is not seed";
		return;
	}
	Json::Value root;
	root["head"] = REQUESTADDR;
	c->send(m_hdl, root.toStyledString(), websocketpp::frame::opcode::value::text);
}

void connection_metadata::on_fail(client * c, websocketpp::connection_hdl hdl) {
	m_status = FAILED;
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	m_server = con->get_response_header("Server");
	m_error_reason = con->get_ec().message();
	if (--count_con_times > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); //100ms后再次发起连接
		c->connect(con);
		return;
	}

	if (seed_index_ >= 0) { //种子连接失败请求新的连接
		if (seed_index_ + 1 >= SEED_NUM) {
			tangle::log::error(__FUNCTION__) << "no seed";
			return;
		}
		m_status = CONNECTING;
		seed_index_ += 1;
		m_uri = SEED_URI_ARR[seed_index_];
		con->set_uri(make_shared<uri>(m_uri));
		c->connect(con);
	}
}

void connection_metadata::on_close(client * c, websocketpp::connection_hdl hdl) {
	m_status = CLOSED;
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	std::stringstream s;
	s << "close code: " << con->get_remote_close_code() << " ("
		<< websocketpp::close::status::get_string(con->get_remote_close_code())
		<< "), close reason: " << con->get_remote_close_reason();
	m_error_reason = s.str();
}

void connection_metadata::timer(client * c, lib::error_code const& e) {
	//client::connection_ptr con = c->get_con_from_hdl(m_hdl);
	timeout_ms -= PING_PERIOD_MS;
	c->ping(m_hdl, "period ping\n");
	cout << "ping" << endl;
}

void connection_metadata::on_pong(websocketpp::connection_hdl hdl, string meg) {
	lock_guard<mutex> guard(m_timer_lock);
	timeout_ms = TIMEOUT_MS;
}

void connection_metadata::on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {

	Json::Reader reader;
	Json::Value recv_js;
	if (reader.parse(msg->get_payload(), recv_js)) {
		std::cout << "recv: " << recv_js.toStyledString() << std::endl;
		request_type reqCode = request_type(recv_js["head"].asInt());
		switch (reqCode)
		{
		case network::REQUESTADDR: {

			break;
		}
		case network::RESPONSADDR: {

			Json::Value addr_js = recv_js["body"]["addr"];
			for (auto i = 0; i < addr_js.size(); i++) {
				std::string addr = addr_js[i].asString();
				tangle::log::info(__FUNCTION__) << "RESPONSADDR:" << addr;
				//websocket_endpoint::push_addr(addr);
			}
			break;
		}
		case network::REQUESTHOLES: {
			std::string dest_addr = recv_js["body"]["addr"][0].asString();
			int i = 0;
			tangle::log::info(__FUNCTION__) << "REQUESTHOLES:" << dest_addr;
			break;
		}
		default:
			break;
		}

	}
	else if (msg->get_opcode() == websocketpp::frame::opcode::text) {
		m_messages.push_back("<< " + msg->get_payload());
	}
	else {
		m_messages.push_back("<< " + websocketpp::utility::to_hex(msg->get_payload()));
	}
}

std::ostream & operator<< (std::ostream & out, connection_metadata const & data) {
	out << "> URI: " << data.m_uri << "\n"
		<< "> Status: " << data.m_status << "\n"
		<< "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
		<< "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason) << "\n";
	out << "> Messages Processed: (" << data.m_messages.size() << ") \n";

	std::vector<std::string>::const_iterator it;
	for (it = data.m_messages.begin(); it != data.m_messages.end(); ++it) {
		out << *it << "\n";
	}

	return out;
}

}