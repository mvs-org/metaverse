/**
 * Metaverse - The New Reality, a public blokchain project.
 * Copyright (C) 2013, 2016 viewfin Limited.
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/// @file options.hpp
/// @author hao.chen 
/// @date 20160905
 
#pragma once

#include <boost/filesystem.hpp>
#include <bitcoin/bitcoin.hpp>
#include <getopt/getopt.hpp>

namespace fs = boost::filesystem;

class ArgsOpt{
public:

	ArgsOpt() = delete;
	ArgsOpt(const ArgsOpt&) = delete;
	ArgsOpt(const ArgsOpt&&) = delete;

	ArgsOpt(int argc, char** argv) noexcept :args(argc, (const char**)argv) {}

	bool setup(){
		if( args.has("-h") || args.has("--help") || args.has("-?")) {
        	printUsage();
			return false;
		}
		if( args.has("-v") || args.has("--version") ) {
			bc::cout<<"Metaverse-core v0.1"<<std::endl;
			return false;
		}
		if (args.has("-d") || args.has("--daemon")){
			daemon=true;
		}
		if (args.has("-p") || args.has("--port")){
			std::string arg = args["-p"];
			if( arg.empty() ) arg = args["--port"];

			mainport = atoi(arg.c_str());
			bc::cout<<mainport<<std::endl;
		}
		return true;
	}

	void printUsage(){
		bc::cout<<R"==(Usage: mvsd [options]
Options:
  [-d |--daemon]
		daemon running.
  [-c=path|--conf=path]
		configure file path.
  [-w=path|--work=path]
		work data directory path.
  [-p=port|--port=port]
		main net serve port.
  [--rpcport=port]
		json rpc serve port.
  [--allowweb]
		web serve permission.
  [-h|--help|-?]
		Show help message.

Report bugs to: info@viewfin.com
)==";
	}

	bool daemon{false};
	bool closeweb{false};

	fs::path workdir{"."};
	fs::path configfile{"mvs.conf"};                                                          
	std::string debuglog{"debug.log"};                                            
	std::string errorlog{"error.log"};                                            
	uint32_t mainport{8822};                                                 
	uint32_t testport{18822};                                                 
	uint32_t rpcport{8821};                                                 
	uint32_t webport{18821};                                                 

	struct getopt args;
};
