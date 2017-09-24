/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
 *
 * This file is part of mvs-node.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/chain/attachment/account/account.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

#ifdef MVS_DEBUG
#include <json/minijson_writer.hpp>
#endif

#include <metaverse/bitcoin/math/crypto.hpp>
#include <metaverse/bitcoin.hpp>
using namespace libbitcoin::wallet;

namespace libbitcoin {
namespace chain {

account_multisig::account_multisig():hd_index_(0), m_(0),
	n_(0), pubkey_("")
{
	cosigner_pubkeys_.clear();
}
account_multisig::account_multisig(uint32_t hd_index, uint8_t m, uint8_t n, 
	std::vector<std::string>&& cosigner_pubkeys, std::string& pubkey):hd_index_(hd_index), m_(m),
	n_(n), cosigner_pubkeys_(cosigner_pubkeys), pubkey_(pubkey)
{
}
void account_multisig::set_hd_index(uint32_t hd_index){
	hd_index_ = hd_index;
}
uint32_t account_multisig::get_hd_index() const{
	return hd_index_;
}
void account_multisig::set_index(uint32_t index){
	index_ = index;
}
uint32_t account_multisig::get_index() const{
	return index_;
}
void account_multisig::set_m(uint8_t m){
	m_ = m;
}
uint8_t account_multisig::get_m() const{
	return m_;
}
void account_multisig::set_n(uint8_t n){
	n_ = n;
}
uint8_t account_multisig::get_n() const{
	return n_;
}
std::vector<std::string>& account_multisig::get_cosigner_pubkeys() {
	return cosigner_pubkeys_;
}
void account_multisig::set_cosigner_pubkeys(std::vector<std::string>&& cosigner_pubkeys){
	cosigner_pubkeys_ = cosigner_pubkeys;
}
std::string account_multisig::get_pubkey() const{
	return pubkey_;
}
void account_multisig::set_pubkey(std::string& pubkey){
	pubkey_ = pubkey;
}
std::string account_multisig::get_description() const {
	return description_;
}
void account_multisig::set_description(std::string& description) {
	description_ = description;
}
std::string account_multisig::get_address() const {
	return address_;
}
void account_multisig::set_address(std::string& address) {
	address_ = address;
}
bool account_multisig::from_data(reader& source)
{
    hd_index_ = source.read_4_bytes_little_endian();
    index_ = source.read_4_bytes_little_endian();
	m_ = source.read_byte();
	n_ = source.read_byte();
	pubkey_ = source.read_string();
	// read consigner pubkeys
	uint8_t size = source.read_byte();
	log::trace("from_data")<< size;
	while(size--)
		cosigner_pubkeys_.push_back(source.read_string());
	
	description_ = source.read_string();
	address_ = source.read_string();
	
    return true;	
}

void account_multisig::to_data(writer& sink) const
{
	sink.write_4_bytes_little_endian(hd_index_);
	sink.write_4_bytes_little_endian(index_);
	sink.write_byte(m_);
	sink.write_byte(n_);
	sink.write_string(pubkey_);
	sink.write_byte(cosigner_pubkeys_.size());
	
	log::trace("to_data")<< cosigner_pubkeys_.size();
	
	//uint8_t size = 15; 
	if(cosigner_pubkeys_.size()){
		for(auto& each : cosigner_pubkeys_) {
			sink.write_string(each);
			//size--;
		}
		
	} 
	//while(size--)
		//sink.write_string(std::string("02b66fcb1064d827094685264aaa90d0126861688932eafbd1d1a4ba149de3308b"));
	sink.write_string(description_);
	sink.write_string(address_);
}

uint64_t account_multisig::serialized_size() const
{
	uint64_t size = 4 + 4 + 1 + 1 + (pubkey_.size() + 9) + 1; // hd_index,index,m,n,pubkey,pubkey number
	
    for(auto& each : cosigner_pubkeys_)
    	size += (each.size() + 9);
	size += (description_.size() + 9); 
	size += (address_.size() + 9); 
	return size;
}
bool account_multisig::operator==(account_multisig& other) 
{
	auto field_flag = (hd_index_ == other.get_hd_index()) 	&& (m_ == other.get_m())
		&& (n_ == other.get_n()) && (pubkey_ == other.get_pubkey());

	//auto self_pubkeys = get_cosigner_pubkeys();
	std::sort(cosigner_pubkeys_.begin(), cosigner_pubkeys_.end());
	std::sort(other.get_cosigner_pubkeys().begin(), other.get_cosigner_pubkeys().end());
	auto vec_flag = (cosigner_pubkeys_ == other.get_cosigner_pubkeys());
	
	return field_flag && vec_flag;
}
void account_multisig::reset(){
	hd_index_ = 0;
	index_ = 0;
	m_ = 0;
	n_ = 0;
	pubkey_ = ""; 
	cosigner_pubkeys_.clear();
	description_ = "";
	address_ = "";
}
#ifdef MVS_DEBUG
std::string account_multisig::to_string() 
{
    std::ostringstream ss;

    ss << "\t hd_index = " << hd_index_ << "\n"
		<< "\t index = " << index_ << "\n"
		<< "\t m = " << m_ << "\n"
		<< "\t n = " << n_ << "\n"
		<< "\t pubkey = " << pubkey_ << "\n"
		<< "\t description = " << description_ << "\n";
	for(auto& each : cosigner_pubkeys_)
		ss << "\t cosigner-pubkey = " << each << std::endl;
    return ss.str();
}
#endif
std::string account_multisig::get_multisig_script(){
	std::sort(cosigner_pubkeys_.begin(), cosigner_pubkeys_.end());
	std::ostringstream ss;
	ss << std::to_string(m_);
	for(auto& each : cosigner_pubkeys_)
		ss << " [ " << each << " ] ";
	ss << std::to_string(n_) << " checkmultisig";
	return ss.str();
}

account::account()
{
	reset();
}
account::account(std::string name, std::string mnemonic, hash_digest passwd, 
		uint32_t hd_index, uint8_t priority, uint8_t status, uint8_t type):
		name(name), mnemonic(mnemonic), passwd(passwd), hd_index(hd_index), priority(priority), status(status), type(type)
{
}

account account::factory_from_data(const data_chunk& data)
{
    account instance;
    instance.from_data(data);
    return instance;
}

account account::factory_from_data(std::istream& stream)
{
    account instance;
    instance.from_data(stream);
    return instance;
}

account account::factory_from_data(reader& source)
{
    account instance;
    instance.from_data(source);
    return instance;
}

bool account::is_valid() const
{
    return true;
}

void account::reset()
{	
    this->name = "";
    this->mnemonic = "";
    //this->passwd = "";
    this->hd_index = 0;
    this->priority = account_priority::common_user; // 0 -- admin user  1 -- common user
    this->status = account_status::normal;
}

bool account::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool account::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool account::from_data(reader& source)
{
    reset();
    name = source.read_string();
    //mnemonic = source.read_string();
    
	// read encrypted mnemonic
	auto size = source.read_variable_uint_little_endian();
	data_chunk string_bytes = source.read_data(size);
	std::string result(string_bytes.begin(), string_bytes.end());
	mnemonic = result;
	
    passwd = source.read_hash();
    hd_index= source.read_4_bytes_little_endian();
    priority= source.read_byte();
	//status = source.read_2_bytes_little_endian();
	type = source.read_byte();
	status = source.read_byte();
	if(type == account_type::multisignature) {
		//multisig.from_data(source);
		account_multisig multisig;
		uint32_t size = source.read_4_bytes_little_endian();
		while(size--) {
			multisig.reset();
			multisig.from_data(source);
			multisig_vec.push_back(multisig);
		}
	}
    return true;	
}

data_chunk account::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size()); // serialized_size is not used
    return data;
}

void account::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void account::to_data(writer& sink) const
{
    sink.write_string(name);
	sink.write_string(mnemonic);
	sink.write_hash(passwd);
	sink.write_4_bytes_little_endian(hd_index);
	sink.write_byte(priority);
	//sink.write_2_bytes_little_endian(status);
	sink.write_byte(type);
	sink.write_byte(status);
	if(type == account_type::multisignature) { 
		//multisig.to_data(sink);
		sink.write_4_bytes_little_endian(multisig_vec.size());
		if(multisig_vec.size()){
			for(auto& each : multisig_vec) {
				each.to_data(sink);
			}
			
		} 
	}
}

uint64_t account::serialized_size() const
{
	uint64_t size = name.size() + mnemonic.size() + passwd.size() + 4 + 1 + 2 + 2*9; // 2 string len 
	if(type == account_type::multisignature) {
		//size += multisig.serialized_size();
		size += 4; // vector size
		for(auto& each : multisig_vec)
			size += each.serialized_size();
	}
	return size;
}

account::operator bool() const
{
	return (name.empty() || mnemonic.empty());
}
bool account::operator==(const account& other) const
{
    return (name.compare(other.get_name()) == 0);
			//&& (mnemonic.compare(other.get_mnemonic()) == 0); // mnemonic not equal when account change passwd
}


#ifdef MVS_DEBUG
std::string account::to_string() 
{
    std::ostringstream ss;

    ss << "\t name = " << name << "\n"
		<< "\t mnemonic = " << mnemonic << "\n"
		<< "\t password = " << passwd.data() << "\n"
		<< "\t hd_index = " << hd_index << "\n"
		<< "\t priority = " << priority << "\n"
		<< "\t type = " << type << "\n"
		<< "\t status = " << status << "\n";
		if(type == account_type::multisignature) { 
			 for(auto& each : multisig_vec)
			 	ss << "\t\t" << each.to_string();
		}
    return ss.str();
}
void account::to_json(std::ostream& output) 
{
	minijson::object_writer json_writer(output);
	json_writer.write("name", name);
	json_writer.write("mnemonic", mnemonic);
	//json_writer.write_array("passwd", std::begin(passwd.data()), std::passwd.end());
	json_writer.write("hd_index", hd_index);
	json_writer.write("priority", priority);
	json_writer.close();
}
#endif

const std::string& account::get_name() const
{ 
    return name;
}
void account::set_name(const std::string& name)
{ 
     this->name = name;
}
const std::string& account::get_mnemonic() const
{
	return mnemonic; // for account == operator
}
const std::string& account::get_mnemonic(std::string& passphrase, std::string& decry_output) const
{
	decrypt_string(mnemonic, passphrase, decry_output);
	return decry_output;
}

void account::set_mnemonic(const std::string& mnemonic, std::string& passphrase)
{ 
	if(!mnemonic.size())
		throw std::logic_error{"mnemonic size is 0"};
	if(!passphrase.size())
		throw std::logic_error{"invalid password!"};
	std::string encry_output("");

	encrypt_string(mnemonic, passphrase, encry_output);
	this->mnemonic = encry_output;
}
void account::set_mnemonic(const std::string& mnemonic)
{ 
	this->mnemonic = mnemonic;
}

const hash_digest& account::get_passwd() const
{ 
    return passwd;
}

uint32_t account::get_hd_index() const
{ 
    return hd_index;
}
void account::set_hd_index(uint32_t hd_index)
{ 
     this->hd_index = hd_index;
}
uint8_t account::get_type() const
{ 
    return type;
}
void account::set_type(uint8_t type)
{ 
     this->type = type;
}
uint8_t account::get_status() const
{ 
    return status;
}
void account::set_status(uint8_t status)
{ 
     this->status = status;
}

uint8_t account::get_priority() const
{ 
    return priority;
}
void account::set_priority(uint8_t priority)
{ 
     this->priority = priority;
}

void account::set_user_status(uint8_t status)
{ 
     this->status |= status;
}
void account::set_system_status(uint8_t status)
{ 
	this->status |= (status<<sizeof(uint8_t));
}
uint8_t account::get_user_status() const
{ 
     return static_cast<uint8_t>(status&0xff);
}
uint8_t account::get_system_status() const
{ 
	return static_cast<uint8_t>(status>>sizeof(uint8_t));
}

const std::vector<account_multisig>& account::get_multisig_vec() const {
	return multisig_vec;
}
void account::set_multisig_vec(std::vector<account_multisig>&& multisig_vec){
	this->multisig_vec = multisig_vec;
}
void account::set_multisig(account_multisig& multisig){
	if(!get_multisig(multisig))
		multisig_vec.push_back(multisig);
}
void account::remove_multisig(account_multisig& multisig, uint16_t index){
	uint16_t i = 1;
	for (auto it = multisig_vec.begin(); it != multisig_vec.end();) {
		if ((index && (i == index))
			|| (*it == multisig)) {
			multisig = *it;
			it = multisig_vec.erase(it);
			break;
		}
		++it;
		++i;
	}
}
bool account::get_multisig(account_multisig& multisig, uint16_t index){
	uint16_t i = 1;
	auto found = false;
	for (auto& each : multisig_vec)
	{
		if ((index && (i == index))
				|| (each == multisig) ){
			multisig = each;
			found = true;
			break;
		}
		++i;
	}
	return found;
}
bool account::get_multisig_by_address(account_multisig& multisig, std::string& addr){
	auto found = false;
	for (auto& each : multisig_vec) {
		if (addr == each.get_address()) {
			multisig = each;
			found = true;
			break;
		}
	}
	return found;
}
void account::modify_multisig(account_multisig& multisig){
	for (auto& each : multisig_vec)
	{
		if (each == multisig) {
			each = multisig;
			break;
		}
	}
}

} // namspace chain
} // namspace libbitcoin
