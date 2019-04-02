#pragma once
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <sstream>

#include <tinytangle/logging.hpp>
#include <network/enumdef.h>
#include <json/json.h>
#include <timer/timer.h>

namespace network {

using namespace tangle;
using namespace std;
using namespace websocketpp;
using namespace timer;

typedef websocketpp::client<websocketpp::config::asio_client> client;

class connection_metadata {

public:
    typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

    connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri, int seed_index = -1)
      : m_id(id)
	  , timeout_ms(TIMEOUT_MS)
	  , seed_index_(seed_index)
      , m_hdl(hdl)
      , m_status(CONNECTING)
      , m_uri(uri)
      , m_server("N/A")
	  , count_con_times(COUNT_CON_TIMES)
    {}

	void init_socket(uint16_t port, connection_hdl hdl, lib::asio::ip::tcp::socket& socket);

	void on_open(client * c, websocketpp::connection_hdl hdl);

	void on_fail(client * c, websocketpp::connection_hdl hdl);

	void on_close(client * c, websocketpp::connection_hdl hdl);

	void timer(client * c, lib::error_code const &);

	void on_pong(websocketpp::connection_hdl hdl, string meg);

	void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);


    websocketpp::connection_hdl get_hdl() const {
        return m_hdl;
    }
    
	bool sub_timeout(int ms) { //判断连接是否超时
		lock_guard<mutex> guard(m_timer_lock);
		timeout_ms -= ms;
		return timeout_ms <= 0;
	}

    int get_id() const {
        return m_id;
    }
    
    int get_status() const {
        return m_status;
    }

	void set_timer_ptr(client::timer_ptr timer_ptr) {
		m_timer_ptr = timer_ptr;
	}

	client::timer_ptr get_timer_ptr() const{
		return m_timer_ptr;
	}

    void record_sent_message(std::string message) {
        m_messages.push_back(">> " + message);
    }

    friend std::ostream & operator<< (std::ostream & out, connection_metadata const & data);

	void set_push_addr_handler(std::function<void(std::string)>& hdl) {
		if (seed_index_ < 0) {
			m_push_addr_hdl = std::bind([]() {
				std::cerr << "this connetion is not the seed" << std::endl;
			});
			return;
		}
		m_push_addr_hdl = hdl;
	}

	void req_all_addr(client * c);

private:

private:
    int m_id;
    websocketpp::connection_hdl m_hdl;//to access to information about the connection and allows changing connection settings.
	client::timer_ptr m_timer_ptr;
    int m_status;
	int timeout_ms;
	int seed_index_;	//种子节点index
	int count_con_times;
	mutex m_timer_lock;
    std::string m_uri;
    std::string m_server;
    std::string m_error_reason;
    std::vector<std::string> m_messages;
	std::function<void(std::string)> m_push_addr_hdl;
};

}