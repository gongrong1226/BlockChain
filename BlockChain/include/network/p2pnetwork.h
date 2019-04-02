#pragma once
#include <network/nodeclient.h>
#include <network/nodeserver.h>

namespace network{

using namespace timer;
class P2PNetwork{

public:

P2PNetwork(){
}

virtual ~P2PNetwork() {
}

void init(uint16_t port);

void sycUnit();

void brocastUnit();

void processUnit();
	
private:
	websocket_endpoint p2pClient;
	p2p_server p2pServer;
};

}