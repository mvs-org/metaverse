/**
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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

#include <metaverse/bitcoin/chain/attachment/asset/attenuation_model.hpp>

namespace libbitcoin {
namespace chain {

bool attenuation_model::check_model_index(uint32_t index)
{
    return index < ATTENUATION_MODEL_FIRST_UNUSED;
}

// ASSET_TODO
bool attenuation_model::check_model_param(uint32_t index, const data_chunk& param)
{
    return true;
}

} // namspace chain
} // namspace libbitcoin
