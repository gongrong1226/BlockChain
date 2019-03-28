#pragma once
#include <tinytangle/keypair.h>
#include <tinytangle/transaction.h>
#include <tinytangle/unit.h>
#include <tinytangle/database.h>
#include <tinytangle/consensus.h>

namespace tangle {

class Dag{
	typedef std::vector<Unit> tipsPool_t;  
public:
	Dag(int id) ;
	Dag(const Dag&) = default;
	Dag(Dag&&) = default;
	Dag& operator=(Dag&&) = default;
	Dag& operator=(const Dag&) = default;

	bool init();
	void getNewKeyPair(KeyPair& newKey);
	bool creatUnit(Unit& newUnit, const Transaction tx, const private_key_t privateKey);

	//获取最新区块hash和timastamp
	bool getLastUnit(int64_t& timestamp, sha256_t& hash);
	uint64_t getCount();

	//获取指定交易单元
	bool getUnit(sha256_t hash, Unit& dest);

	/**
	*@brief 将Json转为Unit，并验证
	*@param root 完整的Unit Json报文
	*@return	true 单元合法并已放入DAG尖端缓存区中
	*		false 单元不合法
	*/
	bool pushUnit(const Json::Value& root);
	//尖端选择，并将所选尖端放入数据库
	bool selectTips(sha256_t(&tips)[2]); //资源竞争问题
	
	//查询目标地址余额
	bool getBalance(const address_t& encodePubKey, value& balance);

	// 获取当前区块Id
	int getId() { return id_; }

	void setDifficulty(uint32_t difficulty);
	uint32_t getDifficulty(void);
	void setVersion(const std::string& version);
	std::string getVersion(void);
private:	bool verifyTip(const Unit& tip);
	bool getRandTip(uint32_t& index);

private:
	int id_;
	DagDatabase dag_db_;
	KeyPairDatabase key_db_;
	uint64_t unit_quantity_ = 0;
	tipsPool_t tipsPool_;
	uint32_t difficulty_;
	std::string version_;

};




}