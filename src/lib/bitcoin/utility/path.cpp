/**
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/utility/path.hpp>

#ifdef _WIN32
#include <Shlobj.h>
#endif

namespace libbitcoin{

boost::filesystem::path default_data_path()
{
    namespace fs = boost::filesystem;
    // Windows < Vista: C:\Documents and Settings\Username\Application Data\Metaverse
    // Windows >= Vista: C:\Users\Username\AppData\Roaming\Metaverse
    // Mac: ~/Library/Application Support/Metaverse
    // Unix: ~/.metaverse
#ifdef _WIN32
    // Windows
    char file_path[MAX_PATH];
    SHGetSpecialFolderPath(NULL, file_path, CSIDL_APPDATA, true);
    return boost::filesystem::path(file_path) / "Metaverse";
#else
    fs::path pathRet;
    char* pszHome = getenv("HOME");
    if (pszHome == nullptr || strlen(pszHome) == 0)
        pathRet = fs::path("/");
    else
        pathRet = fs::path(pszHome);
#ifdef MAC_OSX
    // Mac
    pathRet /= "Library/Application Support";
    fs::create_directories(pathRet / "Metaverse");
    return pathRet / "Metaverse";
#else
    // Unix
    fs::create_directories(pathRet / ".metaverse");
    return pathRet / ".metaverse";
#endif
#endif
}


boost::filesystem::path webpage_path()
{
#ifdef _WIN32
	return "mvs-htmls";
#else
	return default_data_path() / "mvs-htmls";
#endif
}

}//namespace libbitcoin


