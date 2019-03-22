#include <tinytangle/unit.h>

namespace tangle {

Unit::Unit(){
}

Unit::Unit(const Unit& unit_) {
	header_ = unit_.geHheader();
	tx_ = unit_.getTransaction();
}

Unit::Unit(const Json::Value& json) {
	if (json["header"]["nonce"].type() != Json::nullValue
			&&json["header"]["selfWeight"].type() != Json::nullValue
			&&json["header"]["timestamp"].type() != Json::nullValue
			&&json["header"]["difficulty"].type() != Json::nullValue
			&&json["header"]["signature"].type() != Json::nullValue
			&&json["header"]["hash"].type() != Json::nullValue
			&&json["header"]["tipsHash"][0].type() != Json::nullValue
			&&json["header"]["tipsHash"][1].type() != Json::nullValue)
	{
		header_.nonce = json["header"]["nonce"].asUInt();
		header_.selfWeight = json["header"]["selfWeight"].asUInt();
		header_.timestamp = json["header"]["timestamp"].asUInt();
		header_.difficulty = json["header"]["difficulty"].asUInt();
		header_ .signature= json["header"]["signature"].asString();
		header_.hash = json["header"]["hash"].asString();
		header_.tipsHash[0] = json["header"]["tipsHash"][0].asString();
		header_.tipsHash[1] = json["header"]["tipsHash"][1].asString();

		Transaction temp(json["tx"]);
		tx_ = temp;
	}
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

void Unit::signature(private_key_t privateKey) {
	using CryptoPP::StringSource;
	using CryptoPP::SignerFilter;
	using CryptoPP::StringSink;
	using CryptoPP::PSSR;
	using CryptoPP::PSS;
	using CryptoPP::InvertibleRSAFunction;
	using CryptoPP::RSASS;
	using CryptoPP::RSA;
	using CryptoPP::SHA256;
	CryptoPP::AutoSeededRandomPool rng;
	InvertibleRSAFunction parameters;
	parameters.GenerateRandomWithKeySize(rng, 1024);

	//get siganature 
	//www.cryptopp.com/wiki/RSA_Signature_Schemes#RSA_Signature_Scheme_with_Appendix_.28Filters.29
	RSASS<PSS, SHA256>::Signer signer(privateKey);
	std::string &&md = to_string();
	md5_t str = to_md5(md);
	std::string signature;
	StringSource ss1(md, true,
		new SignerFilter(rng, signer,
			new CryptoPP::StringSink(signature)
		) // SignerFilter
	); // StringSource

	//set siganature
	header_.signature = signature;
}

bool Unit::verify() {
	Json::Value &&root = to_json();
	return verify(root);
}

bool Unit::verify(Json::Value root){
	using CryptoPP::StringSource;
	using CryptoPP::StringSink;
	using CryptoPP::PSS;
	using CryptoPP::RSASS;
	using CryptoPP::SHA256;

	//get sinature and reset the sinature
	std::string getSignature = root["header"]["signature"].asString();
	std::string tempstr = "";
	root["header"]["signature"] = tempstr;
	std::string jsonStr = root.toStyledString();

	//get public key of the payer
	std::string pubKeyStr = root["tx"]["tx_item"]["payer"].asString();
	
	public_key_t publicKey;
	if (!KeyPair::AddressToKey(pubKeyStr, publicKey)) {
		return false;
	}

	//verify
	RSASS<PSS, SHA256>::Verifier verifier(publicKey);
	std::string recovered;
	try
	{
		StringSource ss2(jsonStr + getSignature, true,
			new CryptoPP::SignatureVerificationFilter(
				verifier,
				new StringSink(recovered),
				CryptoPP::SignatureVerificationFilter::THROW_EXCEPTION |
				CryptoPP::SignatureVerificationFilter::PUT_MESSAGE
			) // SignatureVerificationFilter
		); // StringSource
	} catch (CryptoPP::Exception&e){
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
	return true;
}

}