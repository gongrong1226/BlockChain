#include <tinychain/tinychain.hpp>
#include <tinychain/commands.hpp>

namespace tinychain
{
bool commands::exec(Json::Value& out){
    try {
        if (*(vargv_.begin()) == "getnewkey") {
            auto&& key = node_.get_chain().get_new_key_pair();
            out = key.to_json();
        } else if  (*(vargv_.begin()) == "listkeys") {
            out = node_.get_chain().list_keys();
        } else if  (*(vargv_.begin()) == "send") {
            if (vargv_.size() >= 3) {
                uint64_t amount = std::stoul(vargv_[2]);
                auto ret = node_.get_chain().send(vargv_[1], amount);
                out = ret.toStyledString();
            } else {
                out = "Invalid or Empty paramas";
            }

        } else if  (*(vargv_.begin()) == "getbalance") {
            out = "getbalance-TODO";

        } else if  (*(vargv_.begin()) == "getlastblock") {
            auto&& b = node_.get_chain().get_last_block();
            out = b.to_json();

        } else if  (*(vargv_.begin()) == "getheight") {
            out = node_.get_chain().get_height();

        } else if  (*(vargv_.begin()) == "startmining") {
            std::string addr;
            if (vargv_.size() >= 2) {
                addr = vargv_[1];
                node_.miner_run(addr);
                out["result"] = "start mining on address: " + addr;
            } else {
                node_.miner_run(addr);
                out["result"] = "start mining on a random address of your wallet";
            }
        } else {
            out = "<getnewkey>  <listkeys>  <getlastblock> <getheight> <getbalance>  <send>  <startmining>";
            return false;
        }
    } catch (std::exception& ex) {
        log::error("commands")<<"error:"<< ex.what();
        return false;
    }

    return true;
}

const commands::vargv_t command_list = { "getnewkey","getlastblock","getheight","send","getbalance", "startmining" };

}	//tinychain