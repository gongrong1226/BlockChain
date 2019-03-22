#pragma once

#include <tinytangle/logging.hpp>
#include <tinytangle/sha256.hpp>
#include <tinytangle/transaction.h>
#include <tinytangle/keypair.h>
#include <json/json.h>
#include <string>
#include <array>
#include <random>
#include <sstream>

#include <cryptopp/secblock.h>

namespace tangle {
	
// ---------------------------- typedef ----------------------------
typedef std::string sha256_t;
typedef std::string version_t;

struct unitHeader {
	version_t version = "v1.0";
	sha256_t hash;
	sha256_t tipsHash[2]; //存在只有一个tip的时候？！
	std::string signature="";
	uint32_t timestamp{ 0 };
	uint32_t nonce{ 0 };
	uint32_t difficulty{ 0 };
	uint32_t selfWeight{ 0 };
};

class Unit
{
public:
	Unit();
	Unit(const Unit& unit_);
	Unit(const Json::Value& json);

	Unit(Unit&&) = default;

	Unit& operator=(const Unit& rb);
	Unit& operator=(Unit&&) = default;

	Transaction getTransaction() const;
	unitHeader geHheader() const;
	sha256_t getHash() const;

	Json::Value to_json();//广播还需加上signature和nonce
	std::string to_string();
	static std::string to_string(const Json::Value json);
	void setup(Transaction& tx);

	void signature(private_key_t privateKey);
	bool verify();

	//待完善
	void print() { std::cout << "class unit" << std::endl; }
	void test();

private:
	bool setSignature(const char* signature, int length);

	bool verify(Json::Value root);

public:
	unitHeader header_;

private:
	Transaction tx_;

};

}
