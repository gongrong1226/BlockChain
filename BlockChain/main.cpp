/**
 * Part of:
 * Comments:
 *
**/
//#include <tinychain/tinychain.hpp>
//#include <tinychain/node.hpp>
//#include <metaverse/mgbubble/mgbubble.hpp> 
#include <serial.h>
#include <tinytangle/keypair.h>
#include <tinytangle/transaction.h>
#include <tinytangle/unit.h>

using namespace tinychain;
using namespace tangle;
using namespace embeded_data;
// global logger

int main(int argc, char* argv[])
{
	KeyPair keyPair;
	//std::cout << keyPair.address() << std::endl;
	keyPair.encode_pair();
	pubkey_t pk = keyPair.address();
	Transaction tx(pk, 0xff, pk);
	Json::StreamWriterBuilder builder;
	std::ostringstream oss;
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(tx.to_json(), &oss);
	Json::Value va = tx.to_json();
	std::cout << oss.str() << std::endl;
	std::cout << va.toStyledString() << std::endl;

	Unit unit;
	unit.setup(tx);
	unit.header_.difficulty = 0;
	unit.header_.nonce = 0x4fa2;
	unit.header_.selfWeight = 1;
	unit.header_.timestamp = get_now_timestamp();
	unit.header_.tipsHash[0] = "tip1";
	unit.header_.tipsHash[1] = "tip2";
	Json::Value unitJson = unit.to_json();
	auto&& hash = getHash256(unitJson);
	unit.header_.hash = hash;
	unit.signature(keyPair.getPrvKey());

	Json::Value transUnit = unit.to_json();
	Unit p2punit(transUnit);
	


	return 0;


	// 初始化本地数据库
	/*serial uart("/dev/ttyUSB0", 115200);
	std::string wrt_string = "this is the test\n";
	std::string buff;
	uart.info();
	uart.start_read();

	while (true) { 
		std::cin >> wrt_string;
		uart.write_data(wrt_string);
		//usleep(500000);
		//uart.write_data("this is the test\n", sizeof("this is the test\n"));
		//if(wrt_string == "abc")

	}*/

	/*Logger logger;
	database d;
	d.init();
	d.print();
	node my_node; block last_block, wonder_block;
	mgbubble::RestServ Server{ "webroot", my_node };
	//都只是设计网络服务，没有涉及到node
	auto& conn = Server.bind("0.0.0.0:8000");
	//将内置HTTP事件处理程序附加到给定连接。用户定义的事件处理程序将接收以下额外事件：
	mg_set_protocol_http_websocket(&conn);
	mg_set_timer(&conn, mg_time() + mgbubble::RestServ::session_check_interval);

	// 启动本地服务
	log::info("main") << "httpserver started";
	Server.run();*/

	/*
	key_pair my_key = my_node.get_chain().get_new_key_pair();
	address_t my_addr = my_key.address();
	my_node.miner_run(my_addr);
	std::string cmd;
	while (cmd != "exit") {
		std::cin >> cmd;
		if (cmd == "tx") {
			tx new_tx(my_addr, 2000);
			my_node.get_chain().collect(new_tx);
		}
	}*/

	/*
	//测试用代码 后续会移除，所有的blockchain操作，都应该是对my_node里面的blockchain操作才对
	//不应该单独拿出来
	std::string input = "grape";
	auto&& output1 = sha256(input);
	log::info("main") << "sha256('" << input << "'):" << output1;

	std::string tx1_addr("tx1_address");
	std::string tx2_addr("tx2_address");
	std::string tx3_addr("tx3_address");
	std::string tx4_addr("tx4_address");

	tx tx1(tx1_addr, 100);
	tx tx2(tx2_addr, 200);
	tx tx3(tx3_addr, 300);
	tx tx4(tx4_addr, 400);

	//blockchain::memory_pool_t p; //一个交易的链表
	block::tx_list_t p;	//应该与上代码相同
	p.push_back(tx1);
	p.push_back(tx2);

	block block1(1);
	block1.setup(p);

	p.push_back(tx3);
	block block2(2);
	block2.setup(p);

	block block3(3);
	blockchain blockchain1;
	blockchain1.push_block(block1);
	blockchain1.push_block(block2);
	blockchain1.push_block(block3);
	blockchain1.print();*/

	return 0;
}

