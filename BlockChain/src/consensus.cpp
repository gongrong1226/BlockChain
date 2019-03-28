#include <algorithm>
#include <tinychain/tinychain.hpp>
#include <tinychain/consensus.hpp>
#include <tinychain/blockchain.hpp>
#ifdef NETWORK
#include <tinychain/network.hpp>
#endif // NETWORK

namespace tinychain
{
	void miner::start(address_t& addr) {
		for (;;) {
			block new_block;

			// δ�ҵ��������
			if (!pow_once(new_block, addr)) {
				continue;
			}

			// ��Ҫ��pool���Ƴ��Ѿ�������Ľ���
			//��Ϊpool.push_back(tx);����+1
			chain_.pool_reset(new_block.header_.tx_count);

			// ��������㲥
			//ws_send(new_block.to_json().toStyledString());

			// ���ش洢
			chain_.push_block(new_block);
		}
	}

	tx miner::create_coinbase_tx(address_t& addr) {
		// TODO

		return tx{ addr };
	}

	bool miner::pow_once(block& new_block, address_t& addr) {


		auto&& pool = chain_.get_pool();
		if(pool.size() == 0) {
			//return false;

		}

		auto&& prev_block = chain_.get_last_block();

		// ����¿�
		new_block.header_.height = prev_block.header_.height + 1;
		new_block.header_.prev_hash = prev_block.header_.hash;
		new_block.header_.timestamp = get_now_timestamp();
		new_block.header_.tx_count = pool.size();

		// �Ѷȵ���: 
		// ����ÿ���ٶȣ���������ٶȣ���Լ10��
		uint64_t time_peroid = new_block.header_.timestamp - prev_block.header_.timestamp;
		log::info("consensus") << "block time :" << time_peroid << " s";

		if (time_peroid <= 10u) {
			new_block.header_.difficulty = prev_block.header_.difficulty + 9000;
		}
		else {
			if (prev_block.header_.difficulty <= 3000) {
				new_block.header_.difficulty = prev_block.header_.difficulty + 9000;
			}
			else {
				new_block.header_.difficulty = prev_block.header_.difficulty - 3000;
			}
		}

		log::debug("consensus") << prev_block.header_.difficulty;

		// �����ڿ�Ŀ��ֵ,���ֵ�����ѶȾ�Ŀ��ֵ
		uint64_t target = 0xffffffffffffffff / prev_block.header_.difficulty;

		// ����coinbase����, CHENHAO => �������������coinbase������ʵ����֤���׺����顣
		auto&& tx = create_coinbase_tx(addr);
		pool.push_back(tx);

		// װ�ؽ���
		new_block.setup(pool);

		// ����Ŀ��ֵ
		for (uint64_t n = 0; ; ++n) {
			//���Ժ�ѡĿ��ֵ
			new_block.header_.nonce = n;
			auto&& jv_block = new_block.to_json();
			auto&& can = to_sha256(jv_block);//���ﻹҪ�ٿ�һ��jv_blockת����String�ж೤
			uint64_t ncan = std::stoull(can.substr(0, 16), 0, 16); //�ض�ǰ16λ��ת��uint64 ����бȽ�

			// �ҵ���
			if (ncan < target) {
				new_block.header_.hash = can;
				//log::info("consensus") << "target:" << ncan;
				//log::info("consensus") << "new block :" << can;
				log::info("consensus") << "new block :" << new_block.to_json().toStyledString();
				return true;
			}
		}
		return false;
	}

	//������
	bool validate_tx(blockchain& chain, const tx& new_tx) {
		// TODO
		// input exsited?
		auto&& inputs = new_tx.get_inputs();
		for (auto& each : inputs) {

			tx pt;
			if (!chain.get_tx(std::get<0>(each), pt)) {
				return false;
			}

			log::info("consensus") << pt.to_json().toStyledString();
			//auto&& tl = pt.outputs();
		}

		return true;
	}


	//������
	bool validate_block(const tx& new_block) {
		// TODO

		return true;
	}

}
