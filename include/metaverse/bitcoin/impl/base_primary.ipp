/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
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

#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

#ifndef MVS_BASE_PRIMARY_IPP
#define MVS_BASE_PRIMARY_IPP

namespace libbitcoin {

template<class T>
T& base_primary<T>::Instance(){
    return static_cast<T&>(*this); 
}

template<class T>
const T& base_primary<T>::Instance() const {
    return static_cast<const T&>(*this);
}

template<class T>
template<typename... Args>
T base_primary<T>::factory_from_data(const data_chunk &data, Args... args){
    T instance;
    instance.from_data(data, args...);
    return instance;
}

template<class T>
template<typename... Args>
T base_primary<T>::factory_from_data(std::istream& stream, Args... args){
    T instance;
    instance.from_data(stream, args...);
    return instance;
}

template<class T>
template<typename... Args>
T base_primary<T>::factory_from_data(reader& source, Args... args){
    T instance;
    instance.from_data(source, args...);
    return instance;
}

template<class T>
template<typename... Args>
bool base_primary<T>::from_data(const data_chunk& data, Args... args){
    data_source istream(data);
    return from_data(istream, args...);
}

template<class T>
template<typename... Args>
bool base_primary<T>::from_data(std::istream& stream, Args... args){
    istream_reader source(stream);
    return from_data(source, args...);
}

template<class T>
template<typename... Args>
bool base_primary<T>::from_data(reader& source, Args... args){
    return Instance().from_data_t(source, args...);
}

template<class T>
data_chunk base_primary<T>::to_data() const{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    return data;
}

template<class T>
template<typename T1_, typename T2_,
typename std::enable_if< (!std::is_base_of<std::ostream, T2_>::value 
&& !std::is_base_of<writer, T2_>::value )>::type*, typename... Args>
data_chunk base_primary<T>::to_data(T1_&& t, Args... args) const{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream, std::forward<T1_&&>(t), std::forward<Args>(args)...);
    ostream.flush();
    return data;
}


template<class T>
template<typename... Args>
void base_primary<T>::to_data(std::ostream& stream, Args... args) const{
    ostream_writer sink(stream);
    to_data(sink, std::forward<Args>(args)...);
}

template<class T>
template< typename... Args>
void base_primary<T>::to_data(writer& sink, Args... args) const{
    writer & wr = sink;
    Instance().to_data_t(wr, std::forward<Args>(args)...);
}

} // namespace libbitcoin

#endif

