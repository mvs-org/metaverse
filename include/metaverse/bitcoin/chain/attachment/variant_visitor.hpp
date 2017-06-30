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
#ifndef MVS_CHAIN_VARIANT_VISITOR_HPP
#define MVS_CHAIN_VARIANT_VISITOR_HPP

#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <boost/variant.hpp>

namespace libbitcoin {
namespace chain {

class from_data_visitor : public boost::static_visitor<bool>
{
public:
	from_data_visitor(reader& src): source(src)
	{

	}
	
	template <class T>
	bool operator()(T &t)
	{
	  return t.from_data(source);
	}
	
	reader& source;
};

class to_data_visitor : public boost::static_visitor<void>
{
public:
	to_data_visitor(writer& dst): sink(dst)
	{

	}
	
	template <class T>
	void operator()(T &t) const
	{
	  return t.to_data(sink);
	}
	
	writer& sink;
};

class serialized_size_visitor : public boost::static_visitor<uint64_t>
{
public:
	serialized_size_visitor()
	{

	}
	
	template <class T>
	uint64_t operator()(T &t) const
	{
	  return t.serialized_size();
	}
};

class to_string_visitor : public boost::static_visitor<std::string>
{
public:
	to_string_visitor()
	{

	}
	
	template <class T>
	std::string operator()(T &t) const
	{
	  return t.to_string();
	}
};

class reset_visitor : public boost::static_visitor<void>
{
public:
	reset_visitor()
	{

	}
	
	template <class T>
	void operator()(T &t)
	{
	  return t.reset();
	}
};

class is_valid_visitor : public boost::static_visitor<bool>
{
public:
	is_valid_visitor()
	{

	}
	
	template <class T>
	bool operator()(T &t) const
	{
	  return t.is_valid();
	}
};

} // namespace chain
} // namespace libbitcoin

#endif

