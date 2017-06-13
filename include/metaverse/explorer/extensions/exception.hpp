/*
 * exception.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: jiang
 */

#ifndef INCLUDE_METAVERSE_EXPLORER_UTILITIES_EXCEPTION_HPP_
#define INCLUDE_METAVERSE_EXPLORER_UTILITIES_EXCEPTION_HPP_

#include <boost/format.hpp>

#define DEFINE_EXPLORER_EXCEPTION(class_name, exception_code) \
	class class_name:public explorer_exception{\
public:\
	class_name(const std::string& message):\
	explorer_exception(exception_code, message)\
{}\
}

namespace libbitcoin {
namespace explorer {


class explorer_exception:public std::exception
{
public:
	explorer_exception(uint32_t code, const std::string& message);
	virtual ~explorer_exception() = default;
	uint32_t code() const { return code_; }
	const std::string& message() const { return message_; }
	#ifdef _WIN32
	virtual const char* what() const override { return message_.data(); }
	#else
	virtual const char* what() const _GLIBCXX_USE_NOEXCEPT override { return message_.data(); }
	#endif
private:
	uint32_t code_;
	std::string message_;
};

std::ostream& operator<<(std::ostream& out, const explorer_exception& ex)
{
	boost::format fmt{"{\"code\":%d, \"message\":\"%s\", \"result\":null}"};
	out << (fmt % ex.code() % ex.message());
	return out;
}

DEFINE_EXPLORER_EXCEPTION(invalid_account_exception, 1);
DEFINE_EXPLORER_EXCEPTION(account_not_exist_exception, 2);
DEFINE_EXPLORER_EXCEPTION(address_mismatch_account_exception, 3);
DEFINE_EXPLORER_EXCEPTION(invalid_argument_exception, 4);
DEFINE_EXPLORER_EXCEPTION(tx_not_found_exception, 5);
DEFINE_EXPLORER_EXCEPTION(account_existed_exception, 6);
DEFINE_EXPLORER_EXCEPTION(admin_required_exception, 7);
DEFINE_EXPLORER_EXCEPTION(last_word_mismatch_exception, 8);
DEFINE_EXPLORER_EXCEPTION(invalid_address_exception, 9);
DEFINE_EXPLORER_EXCEPTION(query_block_height_failed_exception, 10);
DEFINE_EXPLORER_EXCEPTION(invalid_block_height_exception, 11);
DEFINE_EXPLORER_EXCEPTION(no_asset_found_exception, 12);


} //namespace explorer
} //namespace libbitcoin
#endif /* INCLUDE_METAVERSE_EXPLORER_UTILITIES_EXCEPTION_HPP_ */
