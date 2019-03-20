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

	//��ȡ��ǰ�ڵ�߶�
	uint64_ get_height();
	//��ȡ��ǰ�ڵ����¿�
	block get_last_block();
	//��ѯָ������
	bool get_block(sha256_t block_hash, block& out);
	//��ѯָ������
	bool get_tx(sha256_t tx_hash, tx& out);
	//��ѯĿ���ַ���
	bool get_balance(address_t address, uint64_t balance);
	// ��ȡ��ǰ����Id
	auto get_id() { return id_; }
	//��ȡ���׵�ַ����
	memory_pool_t get_pool() { return tx_pool_; }
	//�������Ժ�������ս��׳�
	void pool_reset(size_t times) {//������
		//TO FIX, dirty impl
		while (times--)
			tx_pool_.erase(tx_pool_.begin());
	}
	//�滻block list
	//void merge_replace(block_list_t& block_list);

	//������tx��ӵ�δ����Ľ��׻�������
	void collect(tx& tx) {
		tx_pool_.push_back(tx);
		log::info("blockchain-pool") << "new tx:" << tx.to_json().toStyledString();
	}

	//ע������Կ
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

		//����tx_pool
		collect(target_tx);
		root = target_tx.to_json();
		//�㲥
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