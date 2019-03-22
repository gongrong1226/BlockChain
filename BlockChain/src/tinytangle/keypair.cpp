#include <tinytangle/keypair.h>

namespace tangle {

sha256_t getHash256(Json::Value jv){
	Json::StreamWriterBuilder builder;
	std::ostringstream oss;
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(jv, &oss);
	//std::cout << oss.str() << std::endl;
	return sha256(oss.str());
}

bool jsonToString(const Json::Value &json, std::string& str) {
	Json::StreamWriterBuilder builder;
	std::ostringstream oss;
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(json, &oss);
	str = oss.str();
	return true;
}

//ʹ��md5�õ�hash
md5_t to_md5(const std::string& message) {
	uint8_t digest[CryptoPP::Weak::MD5::DIGESTSIZE];

	CryptoPP::Weak::MD5 hash;
	//�õ���ϢժҪ
	//https://www.cryptopp.com/wiki/Hash_Functions#The_MD5_algorithm
	hash.CalculateDigest(digest, (const uint8_t*)message.c_str(), message.length());

	CryptoPP::HexEncoder encoder;
	std::string output;

	encoder.Attach(new CryptoPP::StringSink(output));
	encoder.Put(digest, sizeof(digest));
	encoder.MessageEnd();

	return output;
}

// �ӹ�Կ�Ƶ���ַ
address_t key_to_address(const public_key_t& public_key) {

	//https://blog.csdn.net/yo_joky/article/details/47041385
	ByteQueue queue;
	std::ostringstream  osstring;
	public_key.DEREncodePublicKey(queue);
	FileSink file(osstring);
	queue.CopyTo(file);
	file.MessageEnd();
	std::string pubkey_string = osstring.str();
	//int lengt1h = qs.size(); //138 Bytes
	log::debug(__FUNCTION__) << pubkey_string;

	/*
	//https://www.cryptopp.com/wiki/BERDecode
	std::string pubkey_string;
	//CryptoPP::StringSource pubkey_ss(pubkey_string, true, new CryptoPP::Base64Encoder);
	CryptoPP::HexEncoder encoder;
	encoder.Attach(new CryptoPP::StringSink(pubkey_string));
	public_key.Save(encoder);
	int length = pubkey_string.size();//320 Bytes
	log::debug(__FUNCTION__) << pubkey_string ;
	//�ݲ���MD5��ֱ�ӷ���pubkey

	CryptoPP::HexDecoder decoder;
	//decoder.Put((byte*)pubkey_string.c_str(), pubkey_string.size());
	decoder.MessageEnd();
	public_key_t pktemp;
	pktemp.Load(decoder);
	*/

	/*
	//���Ȼ�ȡ��׼��ʽ�Ĺ�Կ
	std::string x509_pubk; //binary pubk as x.509
	CryptoPP::StringSink ss(x509_pubk);
	public_key.Save(ss);
	int length2 = x509_pubk.size(); //160 Bytes
	log::debug(__FUNCTION__) << x509_pubk;
	// ����һ��MD5��Ϊ��ַ
	//return to_md5(x509_pubk);
	*/

	return pubkey_string;
}

// ��˽Կ�õ���Կ��ַ���ٽ���Կת��md5(�ݶ�Ϊֱ��pubKey��
address_t key_to_address(const private_key_t& private_key) {
	public_key_t public_key(private_key);
	return key_to_address(public_key);
}

KeyPair::KeyPair(){
	private_key_.GenerateRandomWithKeySize(rng, 1024);
}

KeyPair::KeyPair(const KeyPair& rk) {
	private_key_ = rk.getPrvKey();
}

KeyPair::KeyPair(std::string& encoded_prik) {

	log::debug("KeyPair-in") << encoded_prik;
	// decode base64 into private key
	CryptoPP::StringSource prik_ss(encoded_prik, true, new CryptoPP::Base64Decoder());
	private_key_.BERDecode(prik_ss);
	log::debug("KeyPair-out") << to_json();
}

KeyPair& KeyPair::operator=(const KeyPair& rk){
	private_key_ = rk.getPrvKey();
	return *this;
}

void KeyPair::print(){
	log::info("KeyPair") << to_json();
}

pubkey_t KeyPair::address() const {
	return key_to_address(private_key_);
}

std::pair<std::string, std::string> KeyPair::encode_pair() const {
	// get public key
	public_key_t pubKey(private_key_);
	// encode with base64
	std::string encoded_prik, encoded_pubk;
	CryptoPP::Base64Encoder prik_slink(new CryptoPP::StringSink(encoded_prik), false);//false for no '\n'
	CryptoPP::Base64Encoder pubk_slink(new CryptoPP::StringSink(encoded_pubk), false);
	private_key_.DEREncode(prik_slink);
	pubKey.DEREncode(pubk_slink);
	prik_slink.MessageEnd();//base64 ���벹��=
	pubk_slink.MessageEnd();
	//log::debug("key_pair-0")<<encoded_prik;
	return std::make_pair(encoded_prik, encoded_pubk);
}

private_key_t KeyPair::getPrvKey() const { 
	return private_key_;
}

Json::Value KeyPair::to_json() const {
	Json::Value root;
	auto&& keypair = encode_pair();
	root["address"] = address();//˽Կ->��Կ->md5(�ݶ�string)
	root["public_key"] = keypair.second;
	root["private_key"] = keypair.first;
	return root;
}

bool KeyPair::AddressToKey(std::string pubkey_string, public_key_t& public_key) {
	ByteQueue decodeQueue;
	std::istringstream  instring(pubkey_string);
	log::debug(__FUNCTION__) << instring.str();
	FileSource defile(instring, true /*pumpAll*/);
	defile.TransferTo(decodeQueue);
	decodeQueue.MessageEnd();
	try
	{
		public_key.BERDecodePublicKey(decodeQueue, false /*optParams*/, decodeQueue.MaxRetrievable());
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}
	return true;
}

}