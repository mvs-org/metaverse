/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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
#ifndef BX_PROPERTY_TREE_IPP
#define BX_PROPERTY_TREE_IPP

#include <string>
#include <metaverse/explorer/define.hpp>

/* NOTE: don't declare 'using namespace foo' in headers. */

namespace libbitcoin {
namespace explorer {
namespace config {

template <typename Values>
Json::Value json_helper::prop_tree_list(const std::string& name, const Values& values,
    bool json)
{
    const auto denormalized_name = json ? "" : name;

    Json::Value list;
    for (const auto& value: values)
        list.append(Json::Value()[denormalized_name] = prop_list(value));

    if(list.isNull())
        list.resize(0);
    return list;
}

template <typename Values>
Json::Value json_helper::prop_tree_list_of_lists(const std::string& name,
    const Values& values, bool json)
{
    const auto denormalized_name = json ? "" : name;

    Json::Value list;
    for (const auto& value: values)
        list.append(Json::Value()[denormalized_name] = prop_list(value, json));


    if(list.isNull())
        list.resize(0);

    return list;
}

template <typename Values>
Json::Value json_helper::prop_value_list(const std::string& name, const Values& values,
    bool json)
{
    const auto denormalized_name = json ? "" : name;

    Json::Value list;
    for (const auto& value: values)
    {
        list.append(Json::Value()[denormalized_name] += value);
    }

    if(list.isNull())
        list.resize(0);

    return list;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin

#endif
