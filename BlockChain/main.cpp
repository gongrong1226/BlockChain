#include <serial.h>
#include <tinytangle/keypair.h>
#include <tinytangle/transaction.h>
#include <tinytangle/unit.h>
#include <tinytangle/database.h>
#include <tinytangle/dag.h>
#include <network/p2pnetwork.h>
#include <network/threadpool.hpp>
using namespace tinychain;
using namespace tangle;
using namespace embeded_data;
using namespace network;
// global logger

//测试通过/*
static void testKeyDatabase(void) {
	database db;
	db.init();
	Dag dag(2018);
	dag.init();
	KeyPair A;
	dag.getNewKeyPair(A);

	KeyPair B;
	dag.getNewKeyPair(B);

	KeyPairDatabase keyDB;
	Transaction tx(A.address(), 233, B.address());
	keyDB.updAccount(tx);
	Transaction tx1(B.address(), 133, A.address());
	keyDB.updAccount(tx1);
	return;
}

//测试通过
static void testDag(void){
	//初始化本地数据库
	database db;
	db.init();

	threadpool::fixed_thread_pool m_thread_pool(2, 2);
	//初始化DAG，创建或同步节交易单元（同步是在NETWORK进行）
	Dag dag(2019);
	dag.init();

	//获取密钥
	KeyPair keyA;
	dag.getNewKeyPair(keyA);
	KeyPair keyB;
	dag.getNewKeyPair(keyB);

	//生成Tx
	payer_address A = keyA.address();
	payee_address B = keyB.address();
	Transaction payToB(A, 500, B);
	Transaction payToA(B, 400, A);
	auto keyAPrv = keyA.getPrvKey();
	Unit unit1;
	//根据Tx打包成交易单元
	std::function<void()> fun = bind(&Dag::creatUnit, &dag, unit1, payToB, keyAPrv);
	fun();
	//std::thread t(fun);
	//t.detach();
	//m_thread_pool.execute(fun, "timer1", 2000);
	m_thread_pool.execute(bind(&Dag::creatUnit, &dag, unit1, payToB, keyAPrv), "timer1", 2000);
	m_thread_pool.execute([](){std::cout << "timer2" << std::endl; }, "timer2", 3000);
	/*m_thread_pool.execute(bind([](Dag* dag, Transaction* payToB, private_key_t* keyAPrv) {
		Unit unit1;
		dag->creatUnit(unit1, *payToB, *keyAPrv);
		//交易单元转换json在网络中传输
		Json::Value root = unit1.to_json();

		///////////////网络另一端//////////////
		//收到Json报文，直接push到DAG
		//交由DAG进行验证， 网络只管收发
		Json::Value recv = root;
		if (dag->pushUnit(recv)) {
			//如过验证通过，继续广播该交易
			//broadcast(recv);
			//log::info("testDag") << "Broadcast unit";
			std::cout << "Broadcast unit" << std::endl;
		}
		cout << "test timer1:" << threadpool::fixed_thread_pool::get_current_ms() << endl;
	}, &dag, &payToB, &keyAPrv), "timer1", 2000);*/
	/*
	m_thread_pool.execute(bind([&]() {
		Unit unit1;
		dag.creatUnit(unit1, payToA, keyB.getPrvKey());
		//交易单元转换json在网络中传输
		Json::Value root = unit1.to_json();

		Json::Value recv = root;
		if (dag.pushUnit(recv)) {
			std::cout << "Broadcast unit" << std::endl;
		}
		cout << "test timer2:" << threadpool::fixed_thread_pool::get_current_ms() << endl;
	}), "timer2", 3000);*/
}
std::string test::globalStr = "1";

int main(int argc, char* argv[])
{
	P2PNetwork network_;
	network_.init(9002);
	//testDag();
	while (true) {
		string cmd;
		//cin >> cmd;
		if (cmd == "quit")
			break;
	}

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

