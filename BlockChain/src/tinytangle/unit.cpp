#include <tinytangle/unit.h>

namespace tangle {

Unit::Unit(){
}

Unit::Unit(const Unit& unit_) {
	header_ = unit_.geHheader();
	tx_ = unit_.getTransaction();
}

Unit::Unit(const Json::Value& json) {

	header_.nonce = json["header"]["nonce"].asUInt();
	header_.selfWeight = json["header"]["selfWeight"].asUInt();
	header_.timestamp = json["header"]["timestamp"].asUInt();
	header_.difficulty = json["header"]["difficulty"].asUInt();
	const char* prch= json["header"]["signature"].asCString();
	setSignature(prch, 128);
	header_.hash = json["header"]["hash"].asString();
	header_.tipsHash[0] = json["header"]["tipsHash"][0].asString();
	header_.tipsHash[1] = json["header"]["tipsHash"][1].asString();

	Transaction temp(json["tx"]);
	tx_ = temp;
}

Unit& Unit::operator=(const Unit& rb) {
	header_ = rb.geHheader();
	tx_ = rb.getTransaction();
	return *this;
}

Transaction Unit::getTransaction() const{
	return tx_; 
}

unitHeader Unit::geHheader() const{
	return header_;
}

sha256_t Unit::getHash() const{ 
	return header_.hash; 
}

void Unit::setup(Transaction& tx) {
	tx_ = tx; 
}

Json::Value Unit::to_json() {
	Json::Value root;
	Json::Value uheader;

	uheader["nonce"] = header_.nonce;
	uheader["selfWeight"] = header_.selfWeight;
	uheader["timestamp"] = header_.timestamp;
	uheader["difficulty"] = header_.difficulty;
	uheader["hash"] = header_.hash;
	uheader["signature"] = header_.signature;
	uheader["tipsHash"].append(header_.tipsHash[0]);
	uheader["tipsHash"].append(header_.tipsHash[1]);

	root["header"] = uheader;
	root["tx"] = tx_.to_json();
	return root;
}

std::string Unit::to_string() {
	auto&& j = to_json();
	return j.toStyledString();
}

std::string Unit::to_string(const Json::Value json) {
	return json.toStyledString();
}

bool verify(public_key_t& publicKey, md5_t& md5, const char* getSignature) {
	using CryptoPP::SecByteBlock;
	using CryptoPP::PSS;
	using CryptoPP::InvertibleRSAFunction;
	using CryptoPP::RSASS;
	using CryptoPP::RSA;
	using CryptoPP::SHA256;
	RSASS<PSS, SHA256>::Verifier verifier(publicKey);
	bool result = verifier.VerifyMessage((const byte*)md5.c_str(), md5.size(), (const byte*)getSignature, 300);
	return result;
}

bool verify1(public_key_t& publicKey, md5_t& md5, const char* getSignature) {
	using CryptoPP::SecByteBlock;
	using CryptoPP::PSS;
	using CryptoPP::InvertibleRSAFunction;
	using CryptoPP::RSASS;
	using CryptoPP::RSA;
	using CryptoPP::SHA256;
	RSASS<PSS, SHA256>::Verifier verifier(publicKey);
	bool result = verifier.VerifyMessage((const byte*)md5.c_str(), md5.size(), (const byte*)getSignature, 128);
	return result;
}

void Unit::signature(private_key_t privateKey) {
	using CryptoPP::SecByteBlock;
	using CryptoPP::PSSR;
	using CryptoPP::PSS;
	using CryptoPP::InvertibleRSAFunction;
	using CryptoPP::RSASS;
	using CryptoPP::RSA;
	using CryptoPP::SHA256;
	CryptoPP::AutoSeededRandomPool rng;
	InvertibleRSAFunction parameters;
	parameters.GenerateRandomWithKeySize(rng, 1024);
	//siganature
	RSASS<PSSR, SHA256>::Signer signer(privateKey);
	std::string &&md = to_string();
	md5_t str = to_md5(md);
	size_t messageLen = str.size();
	const char *message = str.c_str();
	size_t length = signer.MaxSignatureLength();
	SecByteBlock signature(length);
	size_t signatureLen;
	try
	{
		signature.CleanNew(length);
		length = signer.SignMessage(rng, (const byte*)message, messageLen, signature);
		signature.resize(length);
	}
	catch (CryptoPP::Exception&e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	const void* tempp = (const byte*)signature;
	const char* signByte = static_cast<const char*>(tempp);
	setSignature(signByte, length);

	Json::Value root;
	root["signature"] = header_.signature;
	const char *ch = root["signature"].asCString();
	char getSignature[128];
	memcpy(getSignature, ch, 128);

	//verify
	std::string pubkey_str = this->getTransaction().getPayer();
	public_key_t publicKey;
	KeyPair::AddressToKey(pubkey_str, publicKey);

	RSASS<PSS, SHA256>::Verifier verifier(publicKey);
	bool result = verifier.VerifyMessage((const byte*)str.c_str(), str.size(), (const byte*)signByte, 128);
	bool result1 = verifier.VerifyMessage((const byte*)str.c_str(), str.size(), (const byte*)getSignature, 128);
	bool result2 = verifier.VerifyMessage((const byte*)str.c_str(), str.size(), (const byte*)getSignature, 128);
	bool s = tangle::verify(publicKey, str, signByte);
	bool s1 = tangle::verify1(publicKey, str, signByte);

	/*Json::Value unit_json = to_json();
	str = to_string();
	std::cout << str << std::endl;
	messageLen = str.size();
	Unit unit(unit_json);

	char* sich = unit.geHheader().signature;
	const byte* sign_byte = (const byte*)sich;
	int signLen = 128;
	SecByteBlock getSignature(sign_byte, signLen);
	memset(unit.header_.signature, 0, 128);
	std::string unit_str = unit.to_string();
	const byte* unit_byte = (const byte*)unit_str.c_str();
	int unitLen = unit_str.size();
	if (unit_str != str) {
		std::cerr << unit_str << std::endl;
	}*/
}

bool Unit::verify(Unit &unit, public_key_t publicKey){
}


bool Unit::setSignature(const char* signature, int length) {
	if (length > 128) {
		return false;
	}
	memcpy(header_.signature, signature, length);
	return true;
}

}