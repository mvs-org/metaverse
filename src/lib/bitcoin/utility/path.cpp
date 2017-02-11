/*
 * dictionaries.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: jiang
 */

#include <bitcoin/bitcoin/utility/path.hpp>

namespace libbitcoin{

boost::filesystem::path default_data_path()
{
    namespace fs = boost::filesystem;
    // Windows < Vista: C:\Documents and Settings\Username\Application Data\Metaverse
    // Windows >= Vista: C:\Users\Username\AppData\Roaming\Metaverse
    // Mac: ~/Library/Application Support/Metaverse
    // Unix: ~/.metaverse
#ifdef WIN32
    // Windows
    return GetSpecialFolderPath(CSIDL_APPDATA) / "Metaverse";
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
	return default_data_path() / "mvs-htmls";
}

}//namespace libbitcoin


