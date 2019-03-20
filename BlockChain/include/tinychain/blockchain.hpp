#pragma once
#include <queue>
#include <tinychain/tinychain.hpp>
#include <tinychain/database.hpp>
#ifdef NETWORK
#include <tinychain/network.hpp>
#endif // NETWORK

namespace tinychain
{
class blockchain
{
public:
	typedef block::tx_list_t memory_pool_t;
	blockchain(uint16_t id = 1998) :id_(id) {
		id_ = id;
		chain_database_.create_genesis_block();
	}
	blockchain(const blockchain&) = default;
	blockchain(blockchain&&) = default;
	blockchain& operator=(blockchain&&) = default;
	blockchain& operator=(const blockchain&) = default;

	void print() {
		log::info("blockchain") << "--------begin--------";
		chain_database_.print();
		log::info("blockchain") << "---------end---------";
	}

	void test();

	void push_block(const block& new_block) {
		chain_database_.push(new_block);
	}

	//获取当前节点高度
	uint64_ get_height();
	//获取当前节点最新块
	block get_last_block();
	//查询指定区块
	bool get_block(sha256_t block_hash, block& out);
	//查询指定交易
	bool get_tx(sha256_t tx_hash, tx& out);
	//查询目标地址余额
	bool get_balance(address_t address, uint64_t balance);
	// 获取当前区块Id
	auto get_id() { return id_; }
	//获取交易地址数据
	memory_pool_t get_pool() { return tx_pool_; }
	//区块打包以后，用于清空交易池
	void pool_reset(size_t times) {//待完善
		//TO FIX, dirty impl
		while (times--)
			tx_pool_.erase(tx_pool_.begin());
	}
	//替换block list
	//void merge_replace(block_list_t& block_list);

	//将交易tx添加到未打包的交易缓冲里面
	void collect(tx& tx) {
		tx_pool_.push_back(tx);
		log::info("blockchain-pool") << "new tx:" << tx.to_json().toStyledString();
	}

	//注册新密钥
	key_pair get_new_key_pair() {
		return key_pair_database_.get_new_key_pair();
	}

	Json::Value list_keys() {
		Json::Value root;
		key_pair_database_.list_keys(root);
		return root;
	}

	Json::Value send(address_t addr, uint64_t amount) {
		Json::Value root;
		tx target_tx{ addr, amount };

		//本地tx_pool
		collect(target_tx);
		root = target_tx.to_json();
		//广播
		//ws_send(target_tx.to_json().toStyledString());

		return root;
	}

private:
	uint16_t id_;
	chain_database chain_database_;
	block genesis_block_;
	key_pair_database key_pair_database_;
	memory_pool_t tx_pool_;
};


}