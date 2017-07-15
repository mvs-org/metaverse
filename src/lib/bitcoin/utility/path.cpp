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
#include <boost/thread/once.hpp>

#ifdef _WIN32
#include <Shlobj.h>
#endif

namespace libbitcoin{

const boost::filesystem::path& default_data_path()
{
    static boost::filesystem::path default_path("");
    static boost::once_flag once = BOOST_ONCE_INIT;
    auto path_init = [&]() {
        namespace fs = boost::filesystem;
        // Windows < Vista: C:\Documents and Settings\Username\Application Data\Metaverse
        // Windows >= Vista: C:\Users\Username\AppData\Roaming\Metaverse
        // Mac: ~/Library/Application Support/Metaverse
        // Unix: ~/.metaverse
#ifdef _WIN32
        // Windows
#ifdef UNICODE
        wchar_t file_path[MAX_PATH] = { 0 };
#else
        char file_path[MAX_PATH] = { 0 };
#endif
        SHGetSpecialFolderPath(NULL, file_path, CSIDL_APPDATA, true);
        fs::path pathRet = boost::filesystem::path(file_path) / "Metaverse";
        fs::create_directories(pathRet);
        default_path = pathRet;
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
        data_path = pathRet / "Metaverse";
#else
        // Unix
        fs::create_directories(pathRet / ".metaverse");
        default_path = pathRet / ".metaverse";
#endif
#endif
    };
    boost::call_once(path_init, once);
    return default_path;
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


