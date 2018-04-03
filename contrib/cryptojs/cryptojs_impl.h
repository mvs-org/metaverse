/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-server.
 *
 * metaverse-server is free software: you can redistribute it and/or
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
//
// Created by czp on 18-3-30.
//

#ifndef METAVERSE_CRYPTOJS_IMPL_H
#define METAVERSE_CRYPTOJS_IMPL_H

#include <string>

namespace cryptojs {

    std::string encrypt(const std::string &message, const std::string &passphrase_);

    std::string decrypt(const std::string &cipher_txt, const std::string &passphrase_);
}
#endif //METAVERSE_CRYPTJS_IMPL_H
