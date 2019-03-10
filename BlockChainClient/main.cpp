
#include "MongooseCli.hpp"
#include <json/json.h>

 /**
  * Invoke this program with the raw arguments provided on the command line.
  * All console input and output streams for the application originate here.
  * @param argc  The number of elements in the argv array.
  * @param argv  The array of arguments, including the process.
  * @return      The numeric result to return via console exit.
  */
using namespace mgbubble::cli;

void my_impl(const http_message* hm)
{
	auto&& reply = std::string(hm->body.p, hm->body.len);
	std::cout << reply << std::endl;
}

int main(int argc, char* argv[])
{
	std::string ip{ argv[1] };
	std::string url = ip + ":8000/rpc";
	//std::cout << url << ", argc= " << argc << std::endl;
	//Json::Reader r;
	// HTTP request call commands
	HttpReq req(url, 3000, reply_handler(my_impl));

	Json::Value jsonvar;
	Json::Value jsonopt;
	jsonvar["jsonrpc"] = "2.0";
	jsonvar["id"] = 1;
	jsonvar["method"] = (argc > 2) ? argv[2] : "help";
	jsonvar["params"] = Json::arrayValue;
	if (argc > 3)
	{
		for (int i = 3; i < argc; i++)
		{
			//std::cout << argv[i] << std::endl;
			jsonvar["params"].append(argv[i]);
		}
	}

	req.post(jsonvar.toStyledString());
	return 0;
}
