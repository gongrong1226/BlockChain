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
#include <network/con_metadata.h>
//SO_REUSEPORT

namespace network {
using namespace tangle;
using namespace std;
using namespace websocketpp;
using namespace timer;

//客户端连接总管理
class websocket_endpoint {
public:
	typedef std::string endpoint_addr_t;
	typedef std::map<int, connection_metadata::ptr> con_list;
	typedef std::map<endpoint_addr_t, connection_hdl> map_conn_addr_t;
	const int P2PCONNECT_MS = 3000;	//检查p2p连接

    websocket_endpoint();

	~websocket_endpoint();

	void init();//返回和种子服务器连接的端口？

	void ping();

	int connect(std::string const & uri, int seed_index = -1);

	void close(int id, websocketpp::close::status::value code, std::string reason);

	void send(int id, std::string message);

	connection_metadata::ptr get_metadata(int id) const;

	static void push_addr(endpoint_addr_t addr) {
		m_addr_list.insert(addr);
	}

	//+为增加-为减少
	static void add_sub_conn(int num) {
		p2p_conn_count += num;
	}

private:
	void req_holes(endpoint_addr_t addr);
	
private:
    client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
	int seed_id;
	bool is_full;
	static int p2p_conn_count;
	static std::set<endpoint_addr_t> m_addr_list;
	
	std::mutex m_connection_lock;
    con_list m_connection_list;
	map_conn_addr_t map_conn_addr;

    int m_next_id;
};
}