#include <tinychain/tinychain.hpp>
#include <tinychain/blockchain.hpp>

namespace tinychain
{
void blockchain::test(){};

block blockchain::get_last_block() {
	return chain_database_.get_last_block();
}

//获取区块，待完善
bool blockchain::get_block(sha256_t block_hash, block& b) {
	if (!chain_database_.get_block(block_hash, b)) {
		return false;
	}
	return true;
}


uint64_t blockchain::get_height() {
	return chain_database_.height();
}

//待完善
bool blockchain::get_balance(address_t address, uint64_t balance) {
	return true;
}

//待完善
bool blockchain::get_tx(sha256_t tx_hash, tx& t) {
	if (!chain_database_.get_tx(tx_hash, t)) {
		return false;
	}
	return true;
}

}
