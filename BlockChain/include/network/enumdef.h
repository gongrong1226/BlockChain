#pragma once
namespace network {
const int CLIENT_PORT = 9002;
const int SERVER_PORT = 9002;
const int TIMEOUT_MS = 5000; //一个连接超时时间
const int PING_PERIOD_MS = 4000; //4000ms Ping 一次服务器

const int COUNT_CON_TIMES = 10; //打洞尝试次数

const int SEED_NUM = 2;
const std::string SEED_URI_ARR[] = { "ws://127.0.0.1:9002",
								"ws://127.0.0.1:9002" };

enum action_type {
	SUBSCRIBE,
	UNSUBSCRIBE,
	MESSAGESTR,
	MESSAGEJSON,
	RECV_PING,
	RECV_PONG,
	TIMEOUT
};

enum request_type {
	REQUESTADDR,
	RESPONSADDR,
	REQUESTHOLES
};

enum connect_type {
	CONNECTING,
	OPEN,
	FAILED,
	CLOSED
};
}