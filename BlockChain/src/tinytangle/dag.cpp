#include <tinytangle/dag.h>

namespace tangle {


Dag::Dag(int id):id_(id){
	//dag_db_.create_genesis_block;
	difficulty_ = 3000;
	version_ = "v1.0";
}

static Unit creatGenesisUnit() {
	Unit genesisUnit;
	const address_t genesisAddr = "0000000000000000000000000000000000000000000000000000000000000000";
	Transaction tx(genesisAddr, 0, genesisAddr);

	genesisUnit.header_.tipsHash[0] = "0000000000000000000000000000000000000000000000000000000000000000";
	genesisUnit.header_.tipsHash[1] = "0000000000000000000000000000000000000000000000000000000000000000";
	genesisUnit.header_.signature = "0000000000000000000000000000000000000000000000000000000000000000";
	genesisUnit.header_.timestamp = get_now_timestamp();
	genesisUnit.header_.nonce = 0;
	genesisUnit.header_.difficulty = 1;
	genesisUnit.header_.selfWeight = 1;
	genesisUnit.header_.hash = getHash256(genesisUnit.to_json());
	genesisUnit.setup(tx);
	return genesisUnit;
}

bool Dag::init(void) {
	if (dag_db_.getCount() == 0) {
		//dag_db_.createGenesisUnit();
		Unit genesisUnit = creatGenesisUnit();
		tipsPool_.push_back(genesisUnit);
	}
	return true;
}
void Dag::getNewKeyPair(KeyPair& newKey) {
	newKey = key_db_.getNewKeyPair();
	//还应该对公钥地址进行广播
	//broadcast(pubKey);
}

static uint32_t calcSelfWeight(uint32_t difficulty) {
	return difficulty / 3000;
}

bool Dag::creatUnit(Unit& newUnit, const Transaction tx, const private_key_t privateKey) {
	newUnit.setup(tx);
	newUnit.header_.difficulty = getDifficulty();
	newUnit.header_.selfWeight = calcSelfWeight(newUnit.header_.difficulty);
	newUnit.header_.version = getVersion();
	if (!selectTips(newUnit.header_.tipsHash)) {
		//尖端缓存区不存在合法交易
		return false;
	}
	//nonce & timestamp
	Consensus::PoW(newUnit);

	//signature;
	std::string str = newUnit.to_string();
	std::string signature = KeyPair::sinature(str, privateKey);
	newUnit.header_.signature = signature;

	return true;
}


//如果传过来的报文（unit)只含有尖端选择、签名和PoW，也会通过验证并放入尖端
//这样会不断增加所选尖端的权重，所以selectTip很重要
bool Dag::pushUnit(const Json::Value& root) {
	Json::Value js = root;

	//验证签名
	if (!KeyPair::verifySignature(js)) {
		return false;
	}

	//验证PoW
	if (!Consensus::verifyPoW(js)) {
		return false;
	}

	Unit newUnit(root);
	//将对应尖端放入数据库
	sha256_t tip1 = newUnit.getHheader().tipsHash[0];
	sha256_t tip2 = newUnit.getHheader().tipsHash[1];
	for (auto i = tipsPool_.begin(); i < tipsPool_.end(); i++) {
		if ((*i).getHash() == tip1 || (*i).getHash() == tip2) {
			dag_db_.push((*i));
			if (tip1 == tip2)
				break;
		}
	}
	tipsPool_.push_back(newUnit);
	return true;
}
//TO DObool Dag::verifyTip(const Unit& tip) {
	if (tip.getHheader().tipsHash[0] == "0000000000000000000000000000000000000000000000000000000000000000") {
		return true;
	}

	return true;
}

//TO DO 后期改为MCMC
bool Dag::getRandTip(uint32_t& index) {
	uint32_t size = tipsPool_.size();
	if (size < 1)
		return false; //没有尖端交易了
	std::vector<int> rand;
	for (uint32_t i = 0; i < size; i++)
		rand.push_back(i);
	random_shuffle(rand.begin(), rand.end());
	std::string  k;
	index = rand[0];
	return true;
}

bool Dag::selectTips(sha256_t(&tipsHash)[2]) { //多进程时资源竞争问题？
	uint32_t rand = 0;
	int tipCount = 0;
	tipsHash[0] = ""; //initialize
	tipsHash[1] = "";
	while (tipCount < 2 && getRandTip(rand) ) {
		if (verifyTip(tipsPool_[rand])) {
			//尖端合法
			tipsHash[tipCount] = tipsPool_[rand].getHash();
			dag_db_.push(tipsPool_[rand]);
			++tipCount;
		}
		else {
		}
		//无论是否合法都应该擦出
		auto begin = tipsPool_.begin();
		tipsPool_.erase(begin + rand);
	}
	if (tipsHash[0] == "")	//尖端缓冲区不存在合法交易
		return false;
	if (tipsHash[1] == "")	//只存在一个尖端
		tipsHash[1] = tipsHash[0];
	return true;
}

//获取最新区块hash和timastamp
bool Dag::getLastUnit(int64_t& timestamp, sha256_t& hash) {
	return dag_db_.getLastUnit(timestamp, hash);
}

uint64_t Dag::getCount() {
	return dag_db_.getCount();
}

bool Dag::getUnit(sha256_t hash, Unit& dest) {
	return dag_db_.getUnit(hash, dest);
}

bool Dag::getBalance(const address_t& encodePubKey, value& balance) {
	return key_db_.getBalance(encodePubKey, balance);
}

void Dag::setDifficulty(uint32_t difficulty) {
	difficulty_ = difficulty;
}

uint32_t Dag::getDifficulty(void) {
	return difficulty_;
}

void Dag::setVersion(const std::string& version) {
	version_ = version;
}

std::string Dag::getVersion(void) {
	return version_;
}


}