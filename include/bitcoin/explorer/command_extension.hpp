/**
 * Copyright (c) 2011-2015 mvs developers 
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
            .add("stop", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_stop_argument(), "stop", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_stop_argument() 
    {
    }

	//fixme
	virtual void set_stop_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("getinfo", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getinfo_argument(), "getinfo", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getinfo_argument() 
    {
    }

	//fixme
	virtual void set_getinfo_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("getpeerinfo", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getpeerinfo_argument(), "getpeerinfo", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getpeerinfo_argument() 
    {
    }

	//fixme
	virtual void set_getpeerinfo_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("ping", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_ping_argument(), "ping", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_ping_argument() 
    {
    }

	//fixme
	virtual void set_ping_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("addnode", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_addnode_argument(), "addnode", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_addnode_argument() 
    {
    }

	//fixme
	virtual void set_addnode_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("getmininginfo", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getmininginfo_argument(), "getmininginfo", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getmininginfo_argument() 
    {
    }

	//fixme
	virtual void set_getmininginfo_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("backupwallet", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_backupwallet_argument(), "backupwallet", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_backupwallet_argument() 
    {
    }

	//fixme
	virtual void set_backupwallet_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("importwallet", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_importwallet_argument(), "importwallet", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_importwallet_argument() 
    {
    }

	//fixme
	virtual void set_importwallet_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ dumpprivkey *************************/

class dumpprivkey: public command
{
public:
    dumpprivkey() = default; 
    virtual ~dumpprivkey() = default; 
    dumpprivkey(const dumpprivkey&) = default; 
    dumpprivkey(dumpprivkey&&) = default; 
    dumpprivkey& operator=(dumpprivkey&&) = default; 
    dumpprivkey& operator=(const dumpprivkey&) = default; 

public:
    static const char* symbol(){ return "dumpprivkey";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "dumpprivkey "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("dumpprivkey", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_dumpprivkey_argument(), "dumpprivkey", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_dumpprivkey_argument() 
    {
    }

	//fixme
	virtual void set_dumpprivkey_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ importprivkey *************************/

class importprivkey: public command
{
public:
    importprivkey() = default; 
    virtual ~importprivkey() = default; 
    importprivkey(const importprivkey&) = default; 
    importprivkey(importprivkey&&) = default; 
    importprivkey& operator=(importprivkey&&) = default; 
    importprivkey& operator=(const importprivkey&) = default; 

public:
    static const char* symbol(){ return "importprivkey";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "importprivkey "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("importprivkey", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_importprivkey_argument(), "importprivkey", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_importprivkey_argument() 
    {
    }

	//fixme
	virtual void set_importprivkey_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ switchaccout *************************/

class switchaccout: public command
{
public:
    switchaccout() = default; 
    virtual ~switchaccout() = default; 
    switchaccout(const switchaccout&) = default; 
    switchaccout(switchaccout&&) = default; 
    switchaccout& operator=(switchaccout&&) = default; 
    switchaccout& operator=(const switchaccout&) = default; 

public:
    static const char* symbol(){ return "switchaccout";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "switchaccout "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("switchaccout", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_switchaccout_argument(), "switchaccout", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_switchaccout_argument() 
    {
    }

	//fixme
	virtual void set_switchaccout_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("getnewaccount", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getnewaccount_argument(), "getnewaccount", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getnewaccount_argument() 
    {
    }

	//fixme
	virtual void set_getnewaccount_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("getaccount", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getaccount_argument(), "getaccount", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getaccount_argument() 
    {
    }

	//fixme
	virtual void set_getaccount_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ getaddresses *************************/

class getaddresses: public command
{
public:
    getaddresses() = default; 
    virtual ~getaddresses() = default; 
    getaddresses(const getaddresses&) = default; 
    getaddresses(getaddresses&&) = default; 
    getaddresses& operator=(getaddresses&&) = default; 
    getaddresses& operator=(const getaddresses&) = default; 

public:
    static const char* symbol(){ return "getaddresses";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaddresses "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("getaddresses", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getaddresses_argument(), "getaddresses", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getaddresses_argument() 
    {
    }

	//fixme
	virtual void set_getaddresses_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("getnewaddress", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getnewaddress_argument(), "getnewaddress", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getnewaddress_argument() 
    {
    }

	//fixme
	virtual void set_getnewaddress_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("getblock", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getblock_argument(), "getblock", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getblock_argument() 
    {
    }

	//fixme
	virtual void set_getblock_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("signmessage", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_signmessage_argument(), "signmessage", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_signmessage_argument() 
    {
    }

	//fixme
	virtual void set_signmessage_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("verifymessage", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_verifymessage_argument(), "verifymessage", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_verifymessage_argument() 
    {
    }

	//fixme
	virtual void set_verifymessage_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("createmultisig", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_createmultisig_argument(), "createmultisig", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_createmultisig_argument() 
    {
    }

	//fixme
	virtual void set_createmultisig_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("addmultisigaddress", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_addmultisigaddress_argument(), "addmultisigaddress", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_addmultisigaddress_argument() 
    {
    }

	//fixme
	virtual void set_addmultisigaddress_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("validateaddress", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_validateaddress_argument(), "validateaddress", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_validateaddress_argument() 
    {
    }

	//fixme
	virtual void set_validateaddress_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("getbalance", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getbalance_argument(), "getbalance", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getbalance_argument() 
    {
    }

	//fixme
	virtual void set_getbalance_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("getaddressbalance", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getaddressbalance_argument(), "getaddressbalance", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getaddressbalance_argument() 
    {
    }

	//fixme
	virtual void set_getaddressbalance_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("getaccountbalance", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getaccountbalance_argument(), "getaccountbalance", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getaccountbalance_argument() 
    {
    }

	//fixme
	virtual void set_getaccountbalance_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ getaddresstransaction *************************/

class getaddresstransaction: public command
{
public:
    getaddresstransaction() = default; 
    virtual ~getaddresstransaction() = default; 
    getaddresstransaction(const getaddresstransaction&) = default; 
    getaddresstransaction(getaddresstransaction&&) = default; 
    getaddresstransaction& operator=(getaddresstransaction&&) = default; 
    getaddresstransaction& operator=(const getaddresstransaction&) = default; 

public:
    static const char* symbol(){ return "getaddresstransaction";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaddresstransaction "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("getaddresstransaction", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_getaddresstransaction_argument(), "getaddresstransaction", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_getaddresstransaction_argument() 
    {
    }

	//fixme
	virtual void set_getaddresstransaction_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ listaddresstransactions *************************/

class listaddresstransactions: public command
{
public:
    listaddresstransactions() = default; 
    virtual ~listaddresstransactions() = default; 
    listaddresstransactions(const listaddresstransactions&) = default; 
    listaddresstransactions(listaddresstransactions&&) = default; 
    listaddresstransactions& operator=(listaddresstransactions&&) = default; 
    listaddresstransactions& operator=(const listaddresstransactions&) = default; 

public:
    static const char* symbol(){ return "listaddresstransactions";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "listaddresstransactions "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("listaddresstransactions", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_listaddresstransactions_argument(), "listaddresstransactions", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_listaddresstransactions_argument() 
    {
    }

	//fixme
	virtual void set_listaddresstransactions_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ listaccounttransactions *************************/

class listaccounttransactions: public command
{
public:
    listaccounttransactions() = default; 
    virtual ~listaccounttransactions() = default; 
    listaccounttransactions(const listaccounttransactions&) = default; 
    listaccounttransactions(listaccounttransactions&&) = default; 
    listaccounttransactions& operator=(listaccounttransactions&&) = default; 
    listaccounttransactions& operator=(const listaccounttransactions&) = default; 

public:
    static const char* symbol(){ return "listaccounttransactions";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "listaccounttransactions "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("listaccounttransactions", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_listaccounttransactions_argument(), "listaccounttransactions", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_listaccounttransactions_argument() 
    {
    }

	//fixme
	virtual void set_listaccounttransactions_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("send", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_send_argument(), "send", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_send_argument() 
    {
    }

	//fixme
	virtual void set_send_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("sendfrom", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_sendfrom_argument(), "sendfrom", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_sendfrom_argument() 
    {
    }

	//fixme
	virtual void set_sendfrom_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ sendmessage *************************/

class sendmessage: public command
{
public:
    sendmessage() = default; 
    virtual ~sendmessage() = default; 
    sendmessage(const sendmessage&) = default; 
    sendmessage(sendmessage&&) = default; 
    sendmessage& operator=(sendmessage&&) = default; 
    sendmessage& operator=(const sendmessage&) = default; 

public:
    static const char* symbol(){ return "sendmessage";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendmessage "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("sendmessage", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_sendmessage_argument(), "sendmessage", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_sendmessage_argument() 
    {
    }

	//fixme
	virtual void set_sendmessage_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ sendmessagefrom *************************/

class sendmessagefrom: public command
{
public:
    sendmessagefrom() = default; 
    virtual ~sendmessagefrom() = default; 
    sendmessagefrom(const sendmessagefrom&) = default; 
    sendmessagefrom(sendmessagefrom&&) = default; 
    sendmessagefrom& operator=(sendmessagefrom&&) = default; 
    sendmessagefrom& operator=(const sendmessagefrom&) = default; 

public:
    static const char* symbol(){ return "sendmessagefrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendmessagefrom "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("sendmessagefrom", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_sendmessagefrom_argument(), "sendmessagefrom", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_sendmessagefrom_argument() 
    {
    }

	//fixme
	virtual void set_sendmessagefrom_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ startmining *************************/

class startmining: public command
{
public:
    startmining() = default; 
    virtual ~startmining() = default; 
    startmining(const startmining&) = default; 
    startmining(startmining&&) = default; 
    startmining& operator=(startmining&&) = default; 
    startmining& operator=(const startmining&) = default; 

public:
    static const char* symbol(){ return "startmining";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "startmining "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("startmining", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_startmining_argument(), "startmining", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_startmining_argument() 
    {
    }

	//fixme
	virtual void set_startmining_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ stopmining *************************/

class stopmining: public command
{
public:
    stopmining() = default; 
    virtual ~stopmining() = default; 
    stopmining(const stopmining&) = default; 
    stopmining(stopmining&&) = default; 
    stopmining& operator=(stopmining&&) = default; 
    stopmining& operator=(const stopmining&) = default; 

public:
    static const char* symbol(){ return "stopmining";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "stopmining "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("stopmining", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_stopmining_argument(), "stopmining", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_stopmining_argument() 
    {
    }

	//fixme
	virtual void set_stopmining_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("listassets", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_listassets_argument(), "listassets", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_listassets_argument() 
    {
    }

	//fixme
	virtual void set_listassets_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("issue", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_issue_argument(), "issue", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_issue_argument() 
    {
    }

	//fixme
	virtual void set_issue_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("issuefrom", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_issuefrom_argument(), "issuefrom", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_issuefrom_argument() 
    {
    }

	//fixme
	virtual void set_issuefrom_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("issuemore", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_issuemore_argument(), "issuemore", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_issuemore_argument() 
    {
    }

	//fixme
	virtual void set_issuemore_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("issuemorefrom", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_issuemorefrom_argument(), "issuemorefrom", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_issuemorefrom_argument() 
    {
    }

	//fixme
	virtual void set_issuemorefrom_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ sendasset *************************/

class sendasset: public command
{
public:
    sendasset() = default; 
    virtual ~sendasset() = default; 
    sendasset(const sendasset&) = default; 
    sendasset(sendasset&&) = default; 
    sendasset& operator=(sendasset&&) = default; 
    sendasset& operator=(const sendasset&) = default; 

public:
    static const char* symbol(){ return "sendasset";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendasset "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("sendasset", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_sendasset_argument(), "sendasset", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_sendasset_argument() 
    {
    }

	//fixme
	virtual void set_sendasset_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};



/************************ sendassetfrom *************************/

class sendassetfrom: public command
{
public:
    sendassetfrom() = default; 
    virtual ~sendassetfrom() = default; 
    sendassetfrom(const sendassetfrom&) = default; 
    sendassetfrom(sendassetfrom&&) = default; 
    sendassetfrom& operator=(sendassetfrom&&) = default; 
    sendassetfrom& operator=(const sendassetfrom&) = default; 

public:
    static const char* symbol(){ return "sendassetfrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendassetfrom "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("sendassetfrom", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_sendassetfrom_argument(), "sendassetfrom", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_sendassetfrom_argument() 
    {
    }

	//fixme
	virtual void set_sendassetfrom_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("sendwithmsg", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_sendwithmsg_argument(), "sendwithmsg", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_sendwithmsg_argument() 
    {
    }

	//fixme
	virtual void set_sendwithmsg_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("sendwithmsgfrom", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_sendwithmsgfrom_argument(), "sendwithmsgfrom", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_sendwithmsgfrom_argument() 
    {
    }

	//fixme
	virtual void set_sendwithmsgfrom_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

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
            .add("settxfee", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        //fixme
        //load_input(get_settxfee_argument(), "settxfee", variables, input, raw);
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr) override;

	//fixme
    virtual void get_settxfee_argument() 
    {
    }

	//fixme
	virtual void set_settxfee_option()
    {
    }

    struct argument
    {
        argument()
        {
        }

    } argument_;

    struct option
    {
        option()
        {
        }

    } option_;

private:

};


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

