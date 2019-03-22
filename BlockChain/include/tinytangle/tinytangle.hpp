#pragma once

#include <tinytangle/logging.hpp>
#include <tinytangle/sha256.hpp>
#include <json/json.h>
#include <string>
#include <array>
#include <random>
#include <sstream>

// Encryption Algorithm from cryptopp
#include <cryptopp/rsa.h> 
#include <cryptopp/base64.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1 //ignore #warning from comipler for MD5
#include <cryptopp/md5.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <cryptopp/pssr.h>
#include <cryptopp/whrlpool.h>

namespace tangle {
typedef std::string md5_t;
typedef std::string address_t;
typedef std::string script_sign_t;
typedef std::string script_pubkey_t;
typedef std::string data_t;
typedef CryptoPP::RSA::PublicKey public_key_t;
typedef CryptoPP::RSA::PrivateKey private_key_t;


// ---------------------------- ulitity ----------------------------
// 获取时间戳的工具函数
uint64_t get_now_timestamp();
//使用sha256得到hash
sha256_t to_sha256(Json::Value jv);
//使用md5得到信息摘要
md5_t to_md5(const std::string& message);
//从公钥推导地址
address_t key_to_address(const public_key_t& public_key);
// 从私钥推导地址
address_t key_to_address(const private_key_t& private_key);


//采用RSA非对称加密算法，生成RSA密钥对，可升级为ECC椭圆曲线加密，待完善
class key_pair
{
public:
	// new key pair
	key_pair() {
		private_key_.GenerateRandomWithKeySize(rng, 1024);
	}
	// Copy
	key_pair(const key_pair& rk) {
		private_key_ = rk.getPrvKey();
	}
	// =
	key_pair& operator=(const key_pair& rk) {
		private_key_ = rk.getPrvKey();
		return *this;
	}

	// 从base64直接构造RSA私钥(密钥本身不可打印，base64可以）
	key_pair(std::string& encoded_prik) {

		log::debug("key_pair-in") << encoded_prik;
		// decode base64 into private key
		CryptoPP::StringSource prik_ss(encoded_prik, true, new CryptoPP::Base64Decoder());
		private_key_.BERDecode(prik_ss);
		log::debug("key_pair-out") << to_json();
	}

	// TODO
	key_pair(key_pair&&) = default;
	key_pair& operator=(key_pair&&) = default;

	void print() { log::info("key_pair") << to_json(); }
	void test();

	//从private_key得到public_key，再将公钥md5摘要
	address_t address() const {
		return key_to_address(private_key_);
	}

	private_key_t getPrvKey() const { return private_key_; }

	// 构造base64的可打印字符串密钥对
	auto encode_pair() const {
		// get public key
		public_key_t pubKey(private_key_);

		// encode with base64
		std::string encoded_prik, encoded_pubk;
		CryptoPP::Base64Encoder prik_slink(new CryptoPP::StringSink(encoded_prik), false);//false for no '\n'
		CryptoPP::Base64Encoder pubk_slink(new CryptoPP::StringSink(encoded_pubk), false);
		private_key_.DEREncode(prik_slink);
		pubKey.DEREncode(pubk_slink);
		prik_slink.MessageEnd();//base64 编码补足=
		pubk_slink.MessageEnd();
		//log::debug("key_pair-0")<<encoded_prik;

		return std::make_pair(encoded_prik, encoded_pubk);
	}

	// 格式化为JSON
	Json::Value to_json() const {
		Json::Value root;
		auto&& keypair = encode_pair();
		root["address"] = address();//私钥->公钥->md5
		root["public_key"] = keypair.second;
		root["private_key"] = keypair.first;
		return root;
	}

public:
	CryptoPP::AutoSeededRandomPool rng;

private:
	private_key_t private_key_;
};


class Transaction
{
public:
	//P2PKH(Pay To Public Key Hash)
	//typedef std::tuple<sha256_t, uint64_t, script_sign_t> input_item_t;
	//typedef std::tuple<address_t, uint64_t, script_pubkey_t> output_item_t;
	typedef std::tuple<address_t, uint64_t> output_item_t; //无脚本 负数为收入整数为输出

	//typedef std::vector<input_item_t> input_t;
	//typedef std::vector<output_item_t> output_t;

	Transaction() {}
	Transaction(address_t& address); //coinbase
	Transaction(address_t& address, uint64_t amount);

	Transaction(const Transaction& rt) {
		input_ = rt.get_inputs();
		output_ = rt.get_outputs();
	}
	Transaction& operator=(const Transaction& rt) {
		input_ = rt.get_inputs();
		output_ = rt.get_outputs();
		return *this;
	}
	Transaction(Transaction&&) = default;
	Transaction& operator=(Transaction&&) = default;

	//打印交易，待完善
	void print() { std::cout << "class Transaction" << std::endl; }
	void test();

	Json::Value item_to_json(input_item_t in);
	Json::Value item_to_json(output_item_t out);
	Json::Value to_json();

	//得到输入项
	input_item_t get_inputs() const { return input_; }
	//得到输出项
	input_item_t get_outputs() const { return output_; }
	sha256_t get_hash() const { return hash_; }

private:
	input_item_t input_;
	output_item_t output_;
	sha256_t hash_;

};


class Unit
{
public:

	struct unitHeader {
		version_t version = "v1.0";
		sha256_t hash;
		sha256_t tipsHash[2]; //存在只有一个tip的时候？！
		uint32_t timestamp{ 0 };
		uint32_t nonce{ 0 };
		uint32_t difficulty{ 0 };
		uint32_t selfWeight{ 0 };
	};

	Unit() {}
	//unit(uint64_t h) { header_.height = h; }
	Unit(const Unit& unit_) {
		header_ = unit_.header();
		tx_ = unit_.getTransaction();
	}
	Unit& operator=(const Unit& rb) {
		header_ = rb.header();
		tx_ = rb.getTransaction();
		return *this;
	}

	Unit(Unit&&) = default;
	Unit& operator=(Unit&&) = default;

	//待完善
	void print() { std::cout << "class unit" << std::endl; }
	void test();

	Transaction getTransaction() const { return tx_; }
	unitHeader header() const { return header_; }

	Json::Value to_json() {
		Json::Value root;
		Json::Value bheader;


		bheader["nonce"] = header_.nonce;
		bheader["selfWeight"] = header_.selfWeight;
		bheader["timestamp"] = header_.timestamp;
		bheader["difficulty"] = header_.difficulty;
		bheader["hash"] = header_.hash;
		bheader["tipsHash"].append(header_.tipsHash[0]);
		bheader["tipsHash"].append(header_.tipsHash[1]);

		root["header"] = bheader;
		Json::Value txJson;
		txJson.append(tx_.to_json());
		root["tx"] = txJson;
		return root;
	}
	Unit(const Json::Value& json) {


		header_.nonce		= json["header"]["nonce"].asUInt();
		header_.selfWeight  = json["header"]["selfWeight"].asUInt();
		header_.timestamp	= json["header"]["timestamp"].asUInt();
		header_.difficulty	= json["header"]["difficulty"].asUInt();
		header_.hash		= json["header"]["hash"].asString();
		header_.tipsHash[1] = json["header"]["tipsHash"][0].asString();
		header_.tipsHash[1] = json["header"]["tipsHash"][0].asString();

	}

	std::string to_string() {
		auto&& j = to_json();
		return j.toStyledString();
	}

	sha256_t hash() const { return header_.hash; }

	void setup(Transaction& tx) { tx_.swap(tx); }
	
	void signature(CryptoPP::RSA::PrivateKey privateKey);

	unitHeader header_;

private:
	Transaction tx_;

};




}