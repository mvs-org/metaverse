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
#ifndef MVS_BASE_PRIMARY_HPP
#define MVS_BASE_PRIMARY_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>

namespace libbitcoin {

template<class T>
class  base_primary
{
public:
    T& Instance();
    const T& Instance() const;
    //
    template <typename... Args>
    static T factory_from_data(const data_chunk &data, Args... args);

    template <typename... Args>
    static T factory_from_data(std::istream& stream, Args... args);

    template<typename... Args>
    static T factory_from_data(reader& source, Args... args);

    template<typename... Args>
    bool from_data(const data_chunk& data, Args... args);

    template<typename... Args>
    bool from_data(std::istream& stream, Args... args);

    template<typename... Args>
    bool from_data(reader& source, Args... args);

    data_chunk to_data() const;

    template<typename T1_, typename T2_ = typename std::decay<T1_>::type,
    typename std::enable_if< (!std::is_base_of<std::ostream, T2_>::value 
    && !std::is_base_of<writer, T2_>::value )>::type* = nullptr, typename... Args>
    data_chunk to_data(T1_&& t , Args... args) const;

    template<typename... Args >
    void to_data(std::ostream& stream, Args... args) const;

    template<typename... Args>
    void to_data(writer& sink, Args... args) const;

private:
    base_primary(){}
    friend T;
};



} // namespace libbitcoin

#include <metaverse/bitcoin/impl/base_primary.ipp>

#endif

