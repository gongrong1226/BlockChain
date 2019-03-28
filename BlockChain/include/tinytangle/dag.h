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

	//��ȡ��������hash��timastamp
	bool getLastUnit(int64_t& timestamp, sha256_t& hash);
	uint64_t getCount();

	//��ȡָ�����׵�Ԫ
	bool getUnit(sha256_t hash, Unit& dest);

	/**
	*@brief ��JsonתΪUnit������֤
	*@param root ������Unit Json����
	*@return	true ��Ԫ�Ϸ����ѷ���DAG��˻�������
	*		false ��Ԫ���Ϸ�
	*/
	bool pushUnit(const Json::Value& root);
	//���ѡ�񣬲�����ѡ��˷������ݿ�
	bool selectTips(sha256_t(&tips)[2]); //��Դ��������
	
	//��ѯĿ���ַ���
	bool getBalance(const address_t& encodePubKey, value& balance);

	// ��ȡ��ǰ����Id
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