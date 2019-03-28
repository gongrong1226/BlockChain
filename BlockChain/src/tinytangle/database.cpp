#include <tinytangle/database.h>

namespace tangle {


// ----------------------- class databse -------------------
void database::print(){
    sqlite3pp::database db_conn{db_name_};
    log::debug("database")<<"====== Print begin =====";
    try{
        sqlite3pp::query qry(db_conn, "SELECT * FROM key_pairs");
        log::debug("database")<< "column count:"<< qry.column_count();

        for (int i = 0; i < qry.column_count(); ++i) {
	      log::debug("database") << "columm name:" << qry.column_name(i);
        }

        for (sqlite3pp::query::iterator i = qry.begin(); i != qry.end(); ++i) {
	        for (int j = 0; j < qry.column_count(); ++j) {
                log::debug("database")<< (*i).get<char const*>(j);
	        }
        }
    } catch (std::exception& ex) {
        db_conn.disconnect();
        log::error("database")<<"disconnect db with error:"<< ex.what();
    }
    db_conn.disconnect();
    log::debug("database")<<"====== Print end =====";
}

void database::init() {

	namespace bf = boost::filesystem;

	auto db_path = bf::current_path() / db_name_;

	if (bf::exists(db_path)) {
		log::info("database") << "Using " << db_name_;
		return;
	}

	log::info("database") << "Initializing " << db_name_;

	sqlite3pp::database db_conn{ db_name_ };

	try {
		sqlite3pp::command cmd(db_conn, "create table if not EXISTS Unit ( \
          hash char(64) not null primary KEY, \
          tiphash1 char(64) not null, \
          tiphash2 char(64) not null, \
          signature char(256), \
		  time_stamp INTEGER ,	\
          nonce INTEGER , \
          difficulty INTEGER , \
          self_weight INTEGER , \
          version char(8) , \
          payer char(64), \
          payee char(64), \
          value BIGINT);");
		//name type 属性
		cmd.execute();

		sqlite3pp::command cmd2(db_conn, "create table if not EXISTS key_pairs( \
          address char(256) primary key, \
          private_key BLOB, \
          account bigint NOT NULL DEFAULT 0);");
		cmd2.execute();

	}
	catch (std::exception& ex) {
		db_conn.disconnect();
		log::error("database") << "disconnect db with error:" << ex.what();
	}

	db_conn.disconnect();

	//log::info("database") << "Creating genesis block";
	log::info("database") << "Finish initializing";
}
bool DagDatabase::push(const Unit& newUnit) {
	sqlite3pp::transaction xct(db_conn_);
	sqlite3pp::command cmd(db_conn_, "INSERT INTO Unit (\
						hash, tiphash1, tiphash2, signature,time_stamp, nonce, difficulty, self_weight, version,\
						payer, payee,value\
						) VALUES (?, ?, ?, ?, ?, ?, ? ,? ,? ,? ,? ,?)");
	//md5不可恢复，如果要可视化应该用另外的编码
	/*payer_address payer = newUnit.getTransaction().getPayer(); //获取地址，因为DERencode的原因，是乱码
	payee_address payee = newUnit.getTransaction().getPayee();	
	addr2String(payer, payer); //将乱码转换可视化字符， code->pubkey->md5
	addr2String(payee, payee);*/ 	
	cmd.binder() << newUnit.header_.hash									//height
		<< newUnit.header_.tipsHash[0]											//tiphash1
		<< newUnit.header_.tipsHash[1] 										//tiphash2
		<< newUnit.header_.signature
		<< static_cast<int>(newUnit.header_.timestamp)										//time_stamp
		<< static_cast<int>(newUnit.header_.nonce)											//nonce
		<< static_cast<int>(newUnit.header_.difficulty)										//difficulty
		<< static_cast<int>(newUnit.header_.selfWeight)										//self weight*/
		<< newUnit.header_.version											//version
		<< newUnit.getTransaction().getPayer()														//payer;
		<< newUnit.getTransaction().getPayee()														//payee
		<< static_cast<long long>(newUnit.getTransaction().getValue());

	log::debug("DagDatabase") << "ADD UNIT " << newUnit.header_.hash;
	cmd.execute();
	xct.commit();
}

void DagDatabase::createGenesisUnit(void) {
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

	push(genesisUnit);
}

bool DagDatabase::getLastUnit(int64_t& timestamp, sha256_t& hash) {
	//sqlite3pp::query qry(db_conn_, "SELECT hash, time_stamp FROM Unit where time_stamp = (SELECT count(*) - 1 FROM Unit)");
 	sqlite3pp::query qry(db_conn_, "SELECT time_stamp,hash FROM Unit order by time_stamp desc limit 0,1");//0为偏移，1为条数
	for (auto i = qry.begin(); i != qry.end(); ++i) {
		timestamp = (*i).get<long long int>(0);
		hash = (*i).get<const char*>(1);
	}
	log::info("database") << "last_block_hash:" << hash;	return true;}uint64_t DagDatabase::getCount() {
	sqlite3pp::query qry(db_conn_, "SELECT count(*) FROM Unit");	auto i = qry.begin();	uint64_t count = (*i).get<long long int>(0);	return count;}bool DagDatabase::getUnit(sha256_t hash, Unit& dest) {
}
KeyPair KeyPairDatabase::getNewKeyPair() {
	KeyPair key; //随机私钥
	auto&& keypair = key.encode_pair(); //encode 64
	std::string prvKey = keypair.first;
	std::string pubKey = keypair.second;

	log::info(__FUNCTION__) << "get new key address: " << pubKey;
	sqlite3pp::transaction xct(db_conn_);
	sqlite3pp::command cmd(db_conn_, "INSERT INTO key_pairs (address, private_key) VALUES (?, ?)");
	//cmd.binder() << key.address() << keypair.first;
	cmd.binder() << pubKey << prvKey;
	cmd.execute();
	xct.commit();

	return key;
}
//正确情况应该是在DagDatabase更新的时候就触发交易更新bool KeyPairDatabase::updAccount(const Transaction& tx) {
	//DEREncodePublicKey -> pubkey
	payer_address payerStr = tx.getPayer();
	payee_address payeeStr = tx.getPayee();
	value v = tx.getValue();
	public_key_t payerKey, payeeKey;
	KeyPair::AddressToKey(payerStr, payerKey);
	KeyPair::AddressToKey(payeeStr, payeeKey);

	//pubkey-> Base64Encoder
	std::string encodePayer, encodePayee;
	CryptoPP::Base64Encoder payerSlink(new CryptoPP::StringSink(encodePayer), false);//false for no '\n'
	CryptoPP::Base64Encoder payeeSlink(new CryptoPP::StringSink(encodePayee), false);//false for no '\n'
	payerKey.DEREncode(payerSlink);
	payeeKey.DEREncode(payeeSlink);
	payerSlink.MessageEnd();//base64 编码补足
	payeeSlink.MessageEnd();//

	//检验账户和未确认的交易
	expenses(encodePayer, v);
	income(encodePayee, v);
	return true;
}
bool KeyPairDatabase::getBalance(const address_t& encodePubKey, value& balance) {
	try
	{
		std::string cmdstr = "SELECT account FROM key_pairs where address = '" + encodePubKey + "'";
		sqlite3pp::query qry(db_conn_, cmdstr.c_str());		auto i = qry.begin();		balance = (*i).get<long long>(0);
	} catch (const std::exception& e) {
		log::error(__FUNCTION__) << e.what();
		return false;
	}	return true;}static bool isGenesis(std::string str) {	int index = 0;	while (str[index] == '0') {		index++;	}	if (index > 32)		return true;}//测试通过bool KeyPairDatabase::expenses(const std::string encodePubKey, const value v) {
	if (isGenesis(encodePubKey))
		return true;
	try
	{
		value balance;
		if (!getBalance(encodePubKey, balance))
			return false;
		balance -= v;

		//www.runoob.com/sqlite/sqlite-update.html
		//cmdstr ="UPDATE key_pairs (account) VALUES (?) where address = '" + encodePubKey + "'";
		std::string cmdstr = "";
		cmdstr = "UPDATE key_pairs SET account = "+ std::to_string(balance) + " where address = '" + encodePubKey + "'";
		sqlite3pp::command cmd(db_conn_, cmdstr.c_str());
		cmd.execute();
	} catch (const std::exception& ex){
		db_conn_.disconnect();
		log::error(__FUNCTION__) << "disconnect db with error: " << ex.what();
		return false;
	}	return true;

}//测试通过bool KeyPairDatabase::income(const std::string encodePubKey, const value v) {
	if (isGenesis(encodePubKey))
		return true;
	try
	{
		value balance;
		if (!getBalance(encodePubKey, balance))
			return false;
		balance += v;

		//www.runoob.com/sqlite/sqlite-update.html
		//cmdstr = "UPDATE key_pairs (account VALUES (?) where address = '" + encodePubKey + "'";
		std::string cmdstr = "";
		cmdstr = "UPDATE key_pairs SET account = " + std::to_string(balance) + " where address = '" + encodePubKey + "'";
		sqlite3pp::command cmd(db_conn_, cmdstr.c_str());
		cmd.execute();

	}
	catch (const std::exception& ex) {
		db_conn_.disconnect();
		log::error(__FUNCTION__) << "disconnect db with error: " << ex.what();
		return false;
	}	return true;}
}