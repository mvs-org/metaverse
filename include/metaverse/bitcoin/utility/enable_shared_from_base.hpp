/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_ENABLE_SHARED_FROM_BASE_HPP
#define MVS_ENABLE_SHARED_FROM_BASE_HPP

#include <memory>

namespace libbitcoin {

/// Because enable_shared_from_this doesn't support inheritance.
template <class Base>
class enable_shared_from_base
    : public std::enable_shared_from_this<Base>
{
protected:
    template <class Derived>
    std::shared_ptr<Derived> shared_from_base()
    {
        return std::static_pointer_cast<Derived>(this->shared_from_this());
    }
};

} // namespace libbitcoin

#endif
