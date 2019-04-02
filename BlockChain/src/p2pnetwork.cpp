#include <network/p2pnetwork.h>
namespace network {


void P2PNetwork::init(uint16_t port){
	try{
		p2pClient.init();
		//p2pServer.run(port);
	}
	catch (const std::exception& e){
		std::cout << e.what() << std::endl;
	}
}

}