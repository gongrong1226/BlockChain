#include <tinychain/tinychain.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace tinychain
{
// �ӹ�Կ�Ƶ���ַ
address_t key_to_address(const public_key_t& public_key){
	// ���Ȼ�ȡ��׼��ʽ�Ĺ�Կ
	std::string x509_pubk; //binary pubk as x.509
	CryptoPP::StringSink ss(x509_pubk);
	public_key.Save(ss);
	//std::cout << "key_to_address:" << x509_pubk << std::endl;
	// ����һ��MD5��Ϊ��ַ
	return to_md5(x509_pubk);
}

// ��˽Կ�õ���Կ��ַ���ٽ���Կת��md5
address_t key_to_address(const private_key_t& private_key) {
	public_key_t public_key(private_key);
	return key_to_address(public_key);
}

// ��ȡʱ����Ĺ��ߺ���
uint64_t get_now_timestamp() {
	boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
	boost::posix_time::time_duration time_from_epoch = boost::posix_time::second_clock::universal_time() - epoch;
	return time_from_epoch.total_seconds();
}

// ��ȡ������Ĺ��ߺ���
uint64_t pseudo_random() {
	std::random_device device;
	std::uniform_int_distribution<uint64_t> distribution;
	return distribution(device);
}

//ʹ��sha256��jason�õ�hash
sha256_t to_sha256(Json::Value jv) {
	Json::StreamWriterBuilder builder;
	std::ostringstream oss;
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(jv, &oss);
	//std::cout << oss.str() << std::endl;
	return sha256(oss.str());
}

//ʹ��md5�õ�hash
md5_t to_md5(const std::string& message) {
	uint8_t digest[CryptoPP::Weak::MD5::DIGESTSIZE];

	CryptoPP::Weak::MD5 hash;
	hash.CalculateDigest(digest, (const uint8_t*)message.c_str(), message.length());

	CryptoPP::HexEncoder encoder;
	std::string output;

	encoder.Attach(new CryptoPP::StringSink(output));
	encoder.Put(digest, sizeof(digest));
	encoder.MessageEnd();

	return output;
}

//#################### ������ĳ�Ա����ʵ�� ##################
tx::tx(address_t& address) {
	// coinbase tx: TODO
	auto&& input_item = std::make_tuple("00000000000000000000000000000000", 0, "0ffffffffffffff");
	inputs_.push_back(input_item);

	// build tx
	auto&& ouput_item = std::make_tuple(address, 1000, "1ffffffffffffffff");
	outputs_.push_back(ouput_item);

	// hash
	to_json();
}

tx::tx(address_t& address, uint64_t amount) {
	//get_balance_from blokchain, P2PKH(Pay To Public Key Hash)
	//TODO
	auto&& input_item = std::make_tuple(to_md5(address), 0, "0ffffffffffffff");
	inputs_.push_back(input_item);

	// build tx
	auto&& ouput_item = std::make_tuple(address, amount, "1ffffffffffffffff");
	outputs_.push_back(ouput_item);

	// hash
	to_json();
}

Json::Value tx::item_to_json(input_item_t in) {
	Json::Value root;
	root["hash"] = std::get<0>(in);
	root["index"] = std::get<1>(in);
	root["script_sign"] = std::get<2>(in);
	return root;
}

Json::Value tx::item_to_json(output_item_t out) {
	Json::Value root;
	root["address"] = std::get<0>(out);
	root["value"] = std::get<1>(out);
	root["script_sign"] = std::get<2>(out);
	return root;
}

Json::Value tx::to_json() {
	Json::Value root;

	Json::Value inputs;
	for (auto& each : inputs_) {
		inputs.append(item_to_json(each));
	}
	root["inputs"] = inputs;

	Json::Value outputs;
	for (auto& each : outputs_) {
		outputs.append(item_to_json(each));
	}
	root["outputs"] = outputs;
	hash_ = to_sha256(root);
	root["hash"] = hash_;

	return root;
}


}