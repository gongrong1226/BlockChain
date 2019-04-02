#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <json/json.h>
#include <iostream>
#include <set>
#include <timer/timer.h>
/*#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>*/
#include <websocketpp/common/thread.hpp>
#include <network/enumdef.h>

namespace network {

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

/* on_open insert connection_hdl into channel
 * on_close remove connection_hdl from channel
 * on_message queue send to all channels
 */


struct action {
	action(action_type t, connection_hdl h) : type(t), hdl(h) {}
	action(action_type t, connection_hdl h, server::message_ptr m)
		: type(t), hdl(h), msg(m) {}
	action(action_type t, connection_hdl h, Json::Value root) : type(t), hdl(h), recv_json(root) {}

	action_type type;
	websocketpp::connection_hdl hdl;
	server::message_ptr msg;
	Json::Value recv_json;

};


class p2p_server {
public:

	p2p_server();
	~p2p_server();

	void run(uint16_t port);

	bool recv_ping(connection_hdl hdl, std::string payload);

	void recv_pong(connection_hdl hdl, std::string payload);

	void pong_timeout(connection_hdl hdl, std::string payload);

	void on_open(connection_hdl hdl);

	void on_close(connection_hdl hdl);

	void on_message(connection_hdl hdl, server::message_ptr msg);

	void process_messages();
	void process_json();

	void brocastPing();
	void clrTimeoutCnnt();

public:
	const int TIME_OUT_SECONDS = 5; //second * 睡眠时间才是
	const unsigned PING_INTERVAL_MS = 4000;

private:
	typedef std::set<connection_hdl, std::owner_less<connection_hdl> > con_list;
	typedef std::string endpoint_addr_t;
	typedef std::map<endpoint_addr_t, connection_hdl> map_conn_addr_t;
	typedef std::map<endpoint_addr_t, int> map_connet_timer_t;
	typedef std::pair<endpoint_addr_t, Json::Value> request;

	server m_server;
	con_list m_connections;
	std::queue<action> m_actions;
	std::queue<request> m_requests;

	mutex m_action_lock;
	mutex m_connection_lock;
	mutex m_request_lock;
	mutex m_timer_lock;
	condition_variable m_action_cond; //用来等待一个锁，同步
	condition_variable m_request_cond;
	map_connet_timer_t map_connet_timer;
	map_conn_addr_t map_conn_addr;

	timer::TimerManager m_timer_manager;
	timer::Timer timer_ping;
	timer::Timer timer_clear_addr;
	//timer::Timer timer_ping(m_timer_manager)
	//timer::Timer timer_clear_addr(m_timer_manager);
};

}