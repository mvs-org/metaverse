/**
 * Copyright (c) 2016 mvs developers 
 *
 * This file is part of libbitcoin-explorer.
 *
 * libbitcoin-explorer is free software: you can redistribute it and/or
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

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/explorer/define.hpp>
#include <bitcoin/explorer/command.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {



/************************ stop *************************/

class stop: public command
{
public:
    stop() = default; 
    virtual ~stop() = default; 
    stop(const stop&) = default; 
    stop(stop&&) = default; 
    stop& operator=(stop&&) = default; 
    stop& operator=(const stop&) = default; 

public:
    static const char* symbol(){ return "stop";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "stop "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("STOP", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "STOP", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ start *************************/

class start: public command
{
public:
    start() = default; 
    virtual ~start() = default; 
    start(const start&) = default; 
    start(start&&) = default; 
    start& operator=(start&&) = default; 
    start& operator=(const start&) = default; 

public:
    static const char* symbol(){ return "start";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "start "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("START", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "START", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ setadmin *************************/

class setadmin: public command
{
public:
    setadmin() = default; 
    virtual ~setadmin() = default; 
    setadmin(const setadmin&) = default; 
    setadmin(setadmin&&) = default; 
    setadmin& operator=(setadmin&&) = default; 
    setadmin& operator=(const setadmin&) = default; 

public:
    static const char* symbol(){ return "setadmin";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "setadmin "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SETADMIN", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SETADMIN", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getinfo *************************/

class getinfo: public command
{
public:
    getinfo() = default; 
    virtual ~getinfo() = default; 
    getinfo(const getinfo&) = default; 
    getinfo(getinfo&&) = default; 
    getinfo& operator=(getinfo&&) = default; 
    getinfo& operator=(const getinfo&) = default; 

public:
    static const char* symbol(){ return "getinfo";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getinfo "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETINFO", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETINFO", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getpeerinfo *************************/

class getpeerinfo: public command
{
public:
    getpeerinfo() = default; 
    virtual ~getpeerinfo() = default; 
    getpeerinfo(const getpeerinfo&) = default; 
    getpeerinfo(getpeerinfo&&) = default; 
    getpeerinfo& operator=(getpeerinfo&&) = default; 
    getpeerinfo& operator=(const getpeerinfo&) = default; 

public:
    static const char* symbol(){ return "getpeerinfo";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getpeerinfo "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETPEERINFO", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETPEERINFO", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ ping *************************/

class ping: public command
{
public:
    ping() = default; 
    virtual ~ping() = default; 
    ping(const ping&) = default; 
    ping(ping&&) = default; 
    ping& operator=(ping&&) = default; 
    ping& operator=(const ping&) = default; 

public:
    static const char* symbol(){ return "ping";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "ping "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("PING", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "PING", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ addnode *************************/

class addnode: public command
{
public:
    addnode() = default; 
    virtual ~addnode() = default; 
    addnode(const addnode&) = default; 
    addnode(addnode&&) = default; 
    addnode& operator=(addnode&&) = default; 
    addnode& operator=(const addnode&) = default; 

public:
    static const char* symbol(){ return "addnode";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "addnode "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ADDNODE", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "ADDNODE", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getmininginfo *************************/

class getmininginfo: public command
{
public:
    getmininginfo() = default; 
    virtual ~getmininginfo() = default; 
    getmininginfo(const getmininginfo&) = default; 
    getmininginfo(getmininginfo&&) = default; 
    getmininginfo& operator=(getmininginfo&&) = default; 
    getmininginfo& operator=(const getmininginfo&) = default; 

public:
    static const char* symbol(){ return "getmininginfo";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getmininginfo "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETMININGINFO", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETMININGINFO", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ backupwallet *************************/

class backupwallet: public command
{
public:
    backupwallet() = default; 
    virtual ~backupwallet() = default; 
    backupwallet(const backupwallet&) = default; 
    backupwallet(backupwallet&&) = default; 
    backupwallet& operator=(backupwallet&&) = default; 
    backupwallet& operator=(const backupwallet&) = default; 

public:
    static const char* symbol(){ return "backupwallet";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "backupwallet "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("BACKUPWALLET", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "BACKUPWALLET", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ importwallet *************************/

class importwallet: public command
{
public:
    importwallet() = default; 
    virtual ~importwallet() = default; 
    importwallet(const importwallet&) = default; 
    importwallet(importwallet&&) = default; 
    importwallet& operator=(importwallet&&) = default; 
    importwallet& operator=(const importwallet&) = default; 

public:
    static const char* symbol(){ return "importwallet";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "importwallet "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("IMPORTWALLET", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "IMPORTWALLET", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ lockwallet *************************/

class lockwallet: public command
{
public:
    lockwallet() = default; 
    virtual ~lockwallet() = default; 
    lockwallet(const lockwallet&) = default; 
    lockwallet(lockwallet&&) = default; 
    lockwallet& operator=(lockwallet&&) = default; 
    lockwallet& operator=(const lockwallet&) = default; 

public:
    static const char* symbol(){ return "lockwallet";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "lockwallet "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("LOCKWALLET", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "LOCKWALLET", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ backupaccount *************************/

class backupaccount: public command
{
public:
    backupaccount() = default; 
    virtual ~backupaccount() = default; 
    backupaccount(const backupaccount&) = default; 
    backupaccount(backupaccount&&) = default; 
    backupaccount& operator=(backupaccount&&) = default; 
    backupaccount& operator=(const backupaccount&) = default; 

public:
    static const char* symbol(){ return "backupaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "backupaccount "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("BACKUPACCOUNT", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "BACKUPACCOUNT", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ importaccount *************************/

class importaccount: public command
{
public:
    importaccount() = default; 
    virtual ~importaccount() = default; 
    importaccount(const importaccount&) = default; 
    importaccount(importaccount&&) = default; 
    importaccount& operator=(importaccount&&) = default; 
    importaccount& operator=(const importaccount&) = default; 

public:
    static const char* symbol(){ return "importaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "importaccount "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("IMPORTACCOUNT", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "IMPORTACCOUNT", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ switchaccount *************************/

class switchaccount: public command
{
public:
    switchaccount() = default; 
    virtual ~switchaccount() = default; 
    switchaccount(const switchaccount&) = default; 
    switchaccount(switchaccount&&) = default; 
    switchaccount& operator=(switchaccount&&) = default; 
    switchaccount& operator=(const switchaccount&) = default; 

public:
    static const char* symbol(){ return "switchaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "switchaccount "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SWITCHACCOUNT", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SWITCHACCOUNT", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ listaccounts *************************/

class listaccounts: public command
{
public:
    listaccounts() = default; 
    virtual ~listaccounts() = default; 
    listaccounts(const listaccounts&) = default; 
    listaccounts(listaccounts&&) = default; 
    listaccounts& operator=(listaccounts&&) = default; 
    listaccounts& operator=(const listaccounts&) = default; 

public:
    static const char* symbol(){ return "listaccounts";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "listaccounts "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("LISTACCOUNTS", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "LISTACCOUNTS", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getnewaccount *************************/

class getnewaccount: public command
{
public:
    getnewaccount() = default; 
    virtual ~getnewaccount() = default; 
    getnewaccount(const getnewaccount&) = default; 
    getnewaccount(getnewaccount&&) = default; 
    getnewaccount& operator=(getnewaccount&&) = default; 
    getnewaccount& operator=(const getnewaccount&) = default; 

public:
    static const char* symbol(){ return "getnewaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getnewaccount "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(argument_.auth, "ACCOUNTAUTH", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
	    )
	    (
            "ACCOUNTNAME",
            value<std::string>(&argument_.name)->required(),
            "account name."
	    )
	    (
            "ACCOUNTAUTH",
            value<std::string>(&argument_.auth)->required(),
            "account auth."
    	);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
	    std::string name;
	    std::string auth;
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaccount *************************/

class getaccount: public command
{
public:
    getaccount() = default; 
    virtual ~getaccount() = default; 
    getaccount(const getaccount&) = default; 
    getaccount(getaccount&&) = default; 
    getaccount& operator=(getaccount&&) = default; 
    getaccount& operator=(const getaccount&) = default; 

public:
    static const char* symbol(){ return "getaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaccount "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETACCOUNT", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETACCOUNT", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ lockaccount *************************/

class lockaccount: public command
{
public:
    lockaccount() = default; 
    virtual ~lockaccount() = default; 
    lockaccount(const lockaccount&) = default; 
    lockaccount(lockaccount&&) = default; 
    lockaccount& operator=(lockaccount&&) = default; 
    lockaccount& operator=(const lockaccount&) = default; 

public:
    static const char* symbol(){ return "lockaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "lockaccount "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("LOCKACCOUNT", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "LOCKACCOUNT", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ setaccountinfo *************************/

class setaccountinfo: public command
{
public:
    setaccountinfo() = default; 
    virtual ~setaccountinfo() = default; 
    setaccountinfo(const setaccountinfo&) = default; 
    setaccountinfo(setaccountinfo&&) = default; 
    setaccountinfo& operator=(setaccountinfo&&) = default; 
    setaccountinfo& operator=(const setaccountinfo&) = default; 

public:
    static const char* symbol(){ return "setaccountinfo";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "setaccountinfo "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SETACCOUNTINFO", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SETACCOUNTINFO", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ listaddresses *************************/

class listaddresses: public command
{
public:
    listaddresses() = default; 
    virtual ~listaddresses() = default; 
    listaddresses(const listaddresses&) = default; 
    listaddresses(listaddresses&&) = default; 
    listaddresses& operator=(listaddresses&&) = default; 
    listaddresses& operator=(const listaddresses&) = default; 

public:
    static const char* symbol(){ return "listaddresses";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "listaddresses "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("LISTADDRESSES", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "LISTADDRESSES", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getnewaddress *************************/

class getnewaddress: public command
{
public:
    getnewaddress() = default; 
    virtual ~getnewaddress() = default; 
    getnewaddress(const getnewaddress&) = default; 
    getnewaddress(getnewaddress&&) = default; 
    getnewaddress& operator=(getnewaddress&&) = default; 
    getnewaddress& operator=(const getnewaddress&) = default; 

public:
    static const char* symbol(){ return "getnewaddress";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getnewaddress "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETNEWADDRESS", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETNEWADDRESS", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaddress *************************/

class getaddress: public command
{
public:
    getaddress() = default; 
    virtual ~getaddress() = default; 
    getaddress(const getaddress&) = default; 
    getaddress(getaddress&&) = default; 
    getaddress& operator=(getaddress&&) = default; 
    getaddress& operator=(const getaddress&) = default; 

public:
    static const char* symbol(){ return "getaddress";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaddress "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETADDRESS", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETADDRESS", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getblock *************************/

class getblock: public command
{
public:
    getblock() = default; 
    virtual ~getblock() = default; 
    getblock(const getblock&) = default; 
    getblock(getblock&&) = default; 
    getblock& operator=(getblock&&) = default; 
    getblock& operator=(const getblock&) = default; 

public:
    static const char* symbol(){ return "getblock";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getblock "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETBLOCK", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETBLOCK", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ signmessage *************************/

class signmessage: public command
{
public:
    signmessage() = default; 
    virtual ~signmessage() = default; 
    signmessage(const signmessage&) = default; 
    signmessage(signmessage&&) = default; 
    signmessage& operator=(signmessage&&) = default; 
    signmessage& operator=(const signmessage&) = default; 

public:
    static const char* symbol(){ return "signmessage";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "signmessage "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SIGNMESSAGE", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SIGNMESSAGE", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ verifymessage *************************/

class verifymessage: public command
{
public:
    verifymessage() = default; 
    virtual ~verifymessage() = default; 
    verifymessage(const verifymessage&) = default; 
    verifymessage(verifymessage&&) = default; 
    verifymessage& operator=(verifymessage&&) = default; 
    verifymessage& operator=(const verifymessage&) = default; 

public:
    static const char* symbol(){ return "verifymessage";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "verifymessage "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("VERIFYMESSAGE", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "VERIFYMESSAGE", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ createmultisig *************************/

class createmultisig: public command
{
public:
    createmultisig() = default; 
    virtual ~createmultisig() = default; 
    createmultisig(const createmultisig&) = default; 
    createmultisig(createmultisig&&) = default; 
    createmultisig& operator=(createmultisig&&) = default; 
    createmultisig& operator=(const createmultisig&) = default; 

public:
    static const char* symbol(){ return "createmultisig";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "createmultisig "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("CREATEMULTISIG", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "CREATEMULTISIG", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ addmultisigaddress *************************/

class addmultisigaddress: public command
{
public:
    addmultisigaddress() = default; 
    virtual ~addmultisigaddress() = default; 
    addmultisigaddress(const addmultisigaddress&) = default; 
    addmultisigaddress(addmultisigaddress&&) = default; 
    addmultisigaddress& operator=(addmultisigaddress&&) = default; 
    addmultisigaddress& operator=(const addmultisigaddress&) = default; 

public:
    static const char* symbol(){ return "addmultisigaddress";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "addmultisigaddress "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ADDMULTISIGADDRESS", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "ADDMULTISIGADDRESS", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ validateaddress *************************/

class validateaddress: public command
{
public:
    validateaddress() = default; 
    virtual ~validateaddress() = default; 
    validateaddress(const validateaddress&) = default; 
    validateaddress(validateaddress&&) = default; 
    validateaddress& operator=(validateaddress&&) = default; 
    validateaddress& operator=(const validateaddress&) = default; 

public:
    static const char* symbol(){ return "validateaddress";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "validateaddress "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("VALIDATEADDRESS", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "VALIDATEADDRESS", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ listbalances *************************/

class listbalances: public command
{
public:
    listbalances() = default; 
    virtual ~listbalances() = default; 
    listbalances(const listbalances&) = default; 
    listbalances(listbalances&&) = default; 
    listbalances& operator=(listbalances&&) = default; 
    listbalances& operator=(const listbalances&) = default; 

public:
    static const char* symbol(){ return "listbalances";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "listbalances "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("LISTBALANCES", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "LISTBALANCES", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getbalance *************************/

class getbalance: public command
{
public:
    getbalance() = default; 
    virtual ~getbalance() = default; 
    getbalance(const getbalance&) = default; 
    getbalance(getbalance&&) = default; 
    getbalance& operator=(getbalance&&) = default; 
    getbalance& operator=(const getbalance&) = default; 

public:
    static const char* symbol(){ return "getbalance";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getbalance "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETBALANCE", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETBALANCE", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaddressbalance *************************/

class getaddressbalance: public command
{
public:
    getaddressbalance() = default; 
    virtual ~getaddressbalance() = default; 
    getaddressbalance(const getaddressbalance&) = default; 
    getaddressbalance(getaddressbalance&&) = default; 
    getaddressbalance& operator=(getaddressbalance&&) = default; 
    getaddressbalance& operator=(const getaddressbalance&) = default; 

public:
    static const char* symbol(){ return "getaddressbalance";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaddressbalance "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETADDRESSBALANCE", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETADDRESSBALANCE", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaccountbalance *************************/

class getaccountbalance: public command
{
public:
    getaccountbalance() = default; 
    virtual ~getaccountbalance() = default; 
    getaccountbalance(const getaccountbalance&) = default; 
    getaccountbalance(getaccountbalance&&) = default; 
    getaccountbalance& operator=(getaccountbalance&&) = default; 
    getaccountbalance& operator=(const getaccountbalance&) = default; 

public:
    static const char* symbol(){ return "getaccountbalance";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaccountbalance "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETACCOUNTBALANCE", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETACCOUNTBALANCE", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ listtxs *************************/

class listtxs: public command
{
public:
    listtxs() = default; 
    virtual ~listtxs() = default; 
    listtxs(const listtxs&) = default; 
    listtxs(listtxs&&) = default; 
    listtxs& operator=(listtxs&&) = default; 
    listtxs& operator=(const listtxs&) = default; 

public:
    static const char* symbol(){ return "listtxs";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "listtxs "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("LISTTXS", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "LISTTXS", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ gettx *************************/

class gettx: public command
{
public:
    gettx() = default; 
    virtual ~gettx() = default; 
    gettx(const gettx&) = default; 
    gettx(gettx&&) = default; 
    gettx& operator=(gettx&&) = default; 
    gettx& operator=(const gettx&) = default; 

public:
    static const char* symbol(){ return "gettx";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "gettx "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETTX", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETTX", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaddresstx *************************/

class getaddresstx: public command
{
public:
    getaddresstx() = default; 
    virtual ~getaddresstx() = default; 
    getaddresstx(const getaddresstx&) = default; 
    getaddresstx(getaddresstx&&) = default; 
    getaddresstx& operator=(getaddresstx&&) = default; 
    getaddresstx& operator=(const getaddresstx&) = default; 

public:
    static const char* symbol(){ return "getaddresstx";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaddresstx "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETADDRESSTX", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETADDRESSTX", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaccounttx *************************/

class getaccounttx: public command
{
public:
    getaccounttx() = default; 
    virtual ~getaccounttx() = default; 
    getaccounttx(const getaccounttx&) = default; 
    getaccounttx(getaccounttx&&) = default; 
    getaccounttx& operator=(getaccounttx&&) = default; 
    getaccounttx& operator=(const getaccounttx&) = default; 

public:
    static const char* symbol(){ return "getaccounttx";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaccounttx "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETACCOUNTTX", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETACCOUNTTX", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ send *************************/

class send: public command
{
public:
    send() = default; 
    virtual ~send() = default; 
    send(const send&) = default; 
    send(send&&) = default; 
    send& operator=(send&&) = default; 
    send& operator=(const send&) = default; 

public:
    static const char* symbol(){ return "send";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "send "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SEND", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SEND", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendmore *************************/

class sendmore: public command
{
public:
    sendmore() = default; 
    virtual ~sendmore() = default; 
    sendmore(const sendmore&) = default; 
    sendmore(sendmore&&) = default; 
    sendmore& operator=(sendmore&&) = default; 
    sendmore& operator=(const sendmore&) = default; 

public:
    static const char* symbol(){ return "sendmore";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendmore "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SENDMORE", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SENDMORE", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendfrom *************************/

class sendfrom: public command
{
public:
    sendfrom() = default; 
    virtual ~sendfrom() = default; 
    sendfrom(const sendfrom&) = default; 
    sendfrom(sendfrom&&) = default; 
    sendfrom& operator=(sendfrom&&) = default; 
    sendfrom& operator=(const sendfrom&) = default; 

public:
    static const char* symbol(){ return "sendfrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendfrom "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SENDFROM", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SENDFROM", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendwithmsg *************************/

class sendwithmsg: public command
{
public:
    sendwithmsg() = default; 
    virtual ~sendwithmsg() = default; 
    sendwithmsg(const sendwithmsg&) = default; 
    sendwithmsg(sendwithmsg&&) = default; 
    sendwithmsg& operator=(sendwithmsg&&) = default; 
    sendwithmsg& operator=(const sendwithmsg&) = default; 

public:
    static const char* symbol(){ return "sendwithmsg";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendwithmsg "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SENDWITHMSG", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SENDWITHMSG", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendwithmsgfrom *************************/

class sendwithmsgfrom: public command
{
public:
    sendwithmsgfrom() = default; 
    virtual ~sendwithmsgfrom() = default; 
    sendwithmsgfrom(const sendwithmsgfrom&) = default; 
    sendwithmsgfrom(sendwithmsgfrom&&) = default; 
    sendwithmsgfrom& operator=(sendwithmsgfrom&&) = default; 
    sendwithmsgfrom& operator=(const sendwithmsgfrom&) = default; 

public:
    static const char* symbol(){ return "sendwithmsgfrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendwithmsgfrom "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SENDWITHMSGFROM", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SENDWITHMSGFROM", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ listassets *************************/

class listassets: public command
{
public:
    listassets() = default; 
    virtual ~listassets() = default; 
    listassets(const listassets&) = default; 
    listassets(listassets&&) = default; 
    listassets& operator=(listassets&&) = default; 
    listassets& operator=(const listassets&) = default; 

public:
    static const char* symbol(){ return "listassets";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "listassets "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("LISTASSETS", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "LISTASSETS", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getasset *************************/

class getasset: public command
{
public:
    getasset() = default; 
    virtual ~getasset() = default; 
    getasset(const getasset&) = default; 
    getasset(getasset&&) = default; 
    getasset& operator=(getasset&&) = default; 
    getasset& operator=(const getasset&) = default; 

public:
    static const char* symbol(){ return "getasset";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getasset "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETASSET", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETASSET", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaddressasset *************************/

class getaddressasset: public command
{
public:
    getaddressasset() = default; 
    virtual ~getaddressasset() = default; 
    getaddressasset(const getaddressasset&) = default; 
    getaddressasset(getaddressasset&&) = default; 
    getaddressasset& operator=(getaddressasset&&) = default; 
    getaddressasset& operator=(const getaddressasset&) = default; 

public:
    static const char* symbol(){ return "getaddressasset";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaddressasset "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETADDRESSASSET", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETADDRESSASSET", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaccountasset *************************/

class getaccountasset: public command
{
public:
    getaccountasset() = default; 
    virtual ~getaccountasset() = default; 
    getaccountasset(const getaccountasset&) = default; 
    getaccountasset(getaccountasset&&) = default; 
    getaccountasset& operator=(getaccountasset&&) = default; 
    getaccountasset& operator=(const getaccountasset&) = default; 

public:
    static const char* symbol(){ return "getaccountasset";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaccountasset "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETACCOUNTASSET", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETACCOUNTASSET", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ createasset *************************/

class createasset: public command
{
public:
    createasset() = default; 
    virtual ~createasset() = default; 
    createasset(const createasset&) = default; 
    createasset(createasset&&) = default; 
    createasset& operator=(createasset&&) = default; 
    createasset& operator=(const createasset&) = default; 

public:
    static const char* symbol(){ return "createasset";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "createasset "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("CREATEASSET", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "CREATEASSET", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ issue *************************/

class issue: public command
{
public:
    issue() = default; 
    virtual ~issue() = default; 
    issue(const issue&) = default; 
    issue(issue&&) = default; 
    issue& operator=(issue&&) = default; 
    issue& operator=(const issue&) = default; 

public:
    static const char* symbol(){ return "issue";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "issue "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ISSUE", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "ISSUE", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ issuefrom *************************/

class issuefrom: public command
{
public:
    issuefrom() = default; 
    virtual ~issuefrom() = default; 
    issuefrom(const issuefrom&) = default; 
    issuefrom(issuefrom&&) = default; 
    issuefrom& operator=(issuefrom&&) = default; 
    issuefrom& operator=(const issuefrom&) = default; 

public:
    static const char* symbol(){ return "issuefrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "issuefrom "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ISSUEFROM", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "ISSUEFROM", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ issuemore *************************/

class issuemore: public command
{
public:
    issuemore() = default; 
    virtual ~issuemore() = default; 
    issuemore(const issuemore&) = default; 
    issuemore(issuemore&&) = default; 
    issuemore& operator=(issuemore&&) = default; 
    issuemore& operator=(const issuemore&) = default; 

public:
    static const char* symbol(){ return "issuemore";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "issuemore "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ISSUEMORE", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "ISSUEMORE", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ issuemorefrom *************************/

class issuemorefrom: public command
{
public:
    issuemorefrom() = default; 
    virtual ~issuemorefrom() = default; 
    issuemorefrom(const issuemorefrom&) = default; 
    issuemorefrom(issuemorefrom&&) = default; 
    issuemorefrom& operator=(issuemorefrom&&) = default; 
    issuemorefrom& operator=(const issuemorefrom&) = default; 

public:
    static const char* symbol(){ return "issuemorefrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "issuemorefrom "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ISSUEMOREFROM", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "ISSUEMOREFROM", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getdid *************************/

class getdid: public command
{
public:
    getdid() = default; 
    virtual ~getdid() = default; 
    getdid(const getdid&) = default; 
    getdid(getdid&&) = default; 
    getdid& operator=(getdid&&) = default; 
    getdid& operator=(const getdid&) = default; 

public:
    static const char* symbol(){ return "getdid";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getdid "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("GETDID", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "GETDID", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ setdid *************************/

class setdid: public command
{
public:
    setdid() = default; 
    virtual ~setdid() = default; 
    setdid(const setdid&) = default; 
    setdid(setdid&&) = default; 
    setdid& operator=(setdid&&) = default; 
    setdid& operator=(const setdid&) = default; 

public:
    static const char* symbol(){ return "setdid";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "setdid "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SETDID", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SETDID", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendwithdid *************************/

class sendwithdid: public command
{
public:
    sendwithdid() = default; 
    virtual ~sendwithdid() = default; 
    sendwithdid(const sendwithdid&) = default; 
    sendwithdid(sendwithdid&&) = default; 
    sendwithdid& operator=(sendwithdid&&) = default; 
    sendwithdid& operator=(const sendwithdid&) = default; 

public:
    static const char* symbol(){ return "sendwithdid";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendwithdid "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SENDWITHDID", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SENDWITHDID", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ settxfee *************************/

class settxfee: public command
{
public:
    settxfee() = default; 
    virtual ~settxfee() = default; 
    settxfee(const settxfee&) = default; 
    settxfee(settxfee&&) = default; 
    settxfee& operator=(settxfee&&) = default; 
    settxfee& operator=(const settxfee&) = default; 

public:
    static const char* symbol(){ return "settxfee";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "settxfee "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("SETTXFEE", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(argument_.xxx, "SETTXFEE", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
		(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

