/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
#include "executor.hpp"

#include <algorithm>
#include <csignal>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <metaverse/server.hpp>
#include <metaverse/macros_define.hpp>
#include <metaverse/bitcoin/utility/backtrace.hpp>
#include <metaverse/bitcoin/utility/path.hpp>

namespace libbitcoin {
namespace server {

using boost::format;
using namespace std::placeholders;
using namespace boost::system;
using namespace bc::config;
using namespace bc::database;
using namespace bc::network;

static constexpr int initialize_stop = 0;
static constexpr int directory_exists = 0;
static constexpr int directory_not_found = 2;
static constexpr auto append = std::ofstream::out | std::ofstream::app;
static const auto application_name = "bs";

std::promise<code> executor::stopping_;

executor::executor(parser& metadata, std::istream& input,
    std::ostream& output, std::ostream& error)
  : metadata_(metadata), output_(output),
    debug_file_((default_data_path() / metadata_.configured.network.debug_file).string(), append),
    error_file_((default_data_path() / metadata_.configured.network.error_file).string(), append)
{
    initialize_logging(debug_file_, error_file_, output, error, metadata_.configured.server.log_level);
    handle_stop(initialize_stop);
}


// Command line options.
// ----------------------------------------------------------------------------
// Emit directly to standard output (not the log).

void executor::do_help()
{
    const auto options = metadata_.load_options();
    printer help(options, application_name, BS_INFORMATION_MESSAGE);
    help.initialize();
    help.commandline(output_);
}

void executor::do_settings()
{
    const auto settings = metadata_.load_settings();
    printer print(settings, application_name, BS_SETTINGS_MESSAGE);
    print.initialize();
    print.settings(output_);
}

void executor::do_version()
{
    output_ << format(BS_VERSION_MESSAGE) %
        MVS_VERSION %
        MVS_SERVER_VERSION %
        MVS_PROTOCOL_VERSION %
        MVS_NODE_VERSION %
        MVS_BLOCKCHAIN_VERSION %
        MVS_VERSION << std::endl;
}

void executor::set_admin()
{
    data_base db(metadata_.configured.database);
    db.start();
    db.set_admin("administerator", "mvsgo");
    db.stop();
}

void executor::set_blackhole_did()
{
    data_base db(metadata_.configured.database);
    db.start();
    db.set_blackhole_did();
    db.stop();
}

// Emit to the log.
bool executor::do_initchain()
{
    initialize_output();

    boost::system::error_code ec;

    const auto& directory = metadata_.configured.database.directory;
    const auto& data_path = directory;

    if (create_directories(data_path, ec))
    {
        log::info(LOG_SERVER) << format(BS_INITIALIZING_CHAIN) % data_path;

        // Unfortunately we are still limited to a choice of hardcoded chains.
        //const auto genesis = metadata_.configured.chain.use_testnet_rules ?
         //   chain::block::genesis_testnet() : chain::block::genesis_mainnet();
        auto genesis = consensus::miner::create_genesis_block(!metadata_.configured.chain.use_testnet_rules);

        const auto result = data_base::initialize(data_path, *genesis);
        if (!result) {
            //rm directories
            remove_all(data_path);
            throw std::runtime_error{ "initialize chain failed" };
        }
        // init admin account
        set_admin();
        // init blackhole DID
        set_blackhole_did();
        log::info(LOG_SERVER) << BS_INITCHAIN_COMPLETE;
        return true;
    }
    else {
        if (MVS_DATABASE_VERSION_NUMBER >= 63) {
            if (!data_base::upgrade_version_63(data_path)) {
                throw std::runtime_error{ " upgrade database to version 63 failed!" };
            }
        }

        if (MVS_DATABASE_VERSION_NUMBER >= 64) {
            if (!data_base::upgrade_version_64(data_path)) {
                throw std::runtime_error{ " upgrade database to version 63 failed!" };
            }
        }
    }

    if (ec.value() == directory_exists)
    {
        return false;
    }

    auto error_info = format(BS_INITCHAIN_NEW) % data_path % ec.message();
    throw std::runtime_error{error_info.str()};
    return false;
}

// Menu selection.
// ----------------------------------------------------------------------------

bool executor::menu()
{
    const auto& config = metadata_.configured;

    if (config.help)
    {
        do_help();
        return true;
    }

    if (config.settings)
    {
        do_settings();
        return true;
    }

    if (config.version)
    {
        do_version();
        return true;
    }

    try
    {
#ifdef NDEBUG
        std::string running_mode = " (release mode) ";
#else
        std::string running_mode = " (debug mode) ";
#endif
        log::info(LOG_SERVER) << "mvsd version " << MVS_VERSION << running_mode;
        // set block data absolute path
        const auto& directory = metadata_.configured.database.directory ;
        if (!directory.is_absolute()) {
            const auto& home = metadata_.configured.data_dir ;
            metadata_.configured.database.directory = home / directory ;
        } else {
            const auto& default_directory = metadata_.configured.database.default_directory;
            metadata_.configured.database.directory = directory / default_directory;
        }

        auto result = do_initchain(); // false means no need to initial chain

        if (config.initchain)
        {
            return result;
        }
    }
    catch(const std::exception& e){ // initialize failed
        //log::error(LOG_SERVER) << format(BS_INITCHAIN_EXISTS) % data_path;
        log::error(LOG_SERVER) << "initialize chain failed," << e.what();
        return false;
    }

    // There are no command line arguments, just run the server.
    return run();
}

// Run.
// ----------------------------------------------------------------------------

bool executor::run()
{
//    initialize_output();

    log::info(LOG_SERVER) << BS_NODE_STARTING;

    if (!verify_directory())
        return false;

    // Ensure all configured services can function.
    set_minimum_threadpool_size();

    // Now that the directory is verified we can create the node for it.
    node_ = std::make_shared<server_node>(metadata_.configured);

#ifdef PRIVATE_CHAIN
    log::info(LOG_SERVER) << "running (private net)";
#else
    log::info(LOG_SERVER)
        << (!node_->is_use_testnet_rules() ? "running (mainnet)" : "running (testnet)");
#endif

    // The callback may be returned on the same thread.
    node_->start(
        std::bind(&executor::handle_started,
            this, _1));

    // Wait for stop.
    stopping_.get_future().wait();

    log::info(LOG_SERVER) << BS_NODE_STOPPING;

    // Close must be called from main thread.
    if (node_->close())
        log::info(LOG_NODE) << BS_NODE_STOPPED;
    else
        log::info(LOG_NODE) << BS_NODE_STOP_FAIL;

    return true;
}

// Handle the completion of the start sequence and begin the run sequence.
void executor::handle_started(const code& ec)
{
    if (ec && ec.value() != error::operation_failed)
    {
        log::error(LOG_SERVER) << format(BS_NODE_START_FAIL) % ec.message();
        stop(ec);
        return;
    }

    log::info(LOG_SERVER) << BS_NODE_SEEDED;

    // This is the beginning of the stop sequence.
    node_->subscribe_stop(
        std::bind(&executor::handle_stopped,
            this, _1));

    // This is the beginning of the run sequence.
    node_->run(
        std::bind(&executor::handle_running,
            this, _1));
}

// This is the end of the run sequence.
void executor::handle_running(const code& ec)
{
    if (ec)
    {
        if (ec.value() != error::service_stopped) {
            log::info(LOG_SERVER) << format(BS_NODE_START_FAIL) % ec.message();
        }
        stop(ec);
        return;
    }

    log::info(LOG_SERVER) << BS_NODE_STARTED;
}

// This is the end of the stop sequence.
void executor::handle_stopped(const code& ec)
{
    stop(ec);
}

// Stop signal.
// ----------------------------------------------------------------------------

void executor::handle_stop(int code)
{
    // Reinitialize after each capture to prevent hard shutdown.
    std::signal(SIGINT, handle_stop);
    std::signal(SIGTERM, handle_stop);
    std::signal(SIGABRT, handle_stop);

    if (code == initialize_stop)
        return;

    if(SIGINT != code)
    {
        do_backtrace("signal.out");
    }

    log::info(LOG_SERVER) << format(BS_NODE_SIGNALED) % code;
    stop(error::success);
}

void executor::stop(const code& ec)
{
    static std::once_flag stop_mutex;
    std::call_once(stop_mutex, [&](){ stopping_.set_value(ec); });
}

// Utilities.
// ----------------------------------------------------------------------------

// Set up logging.
void executor::initialize_output()
{
    // log::debug(LOG_SERVER) << BS_LOG_HEADER;
    log::info(LOG_SERVER) << BS_LOG_HEADER;
    // log::warning(LOG_SERVER) << BS_LOG_HEADER;
    // log::error(LOG_SERVER) << BS_LOG_HEADER;
    // log::fatal(LOG_SERVER) << BS_LOG_HEADER;

    auto file = default_data_path() / metadata_.configured.file;

    if (file.empty())
        log::info(LOG_SERVER) << BS_USING_DEFAULT_CONFIG;
    else
        log::info(LOG_SERVER) << format(BS_USING_CONFIG_FILE) % file;
}

// Use missing directory as a sentinel indicating lack of initialization.
bool executor::verify_directory()
{
    boost::system::error_code ec;
    const auto& directory = metadata_.configured.database.directory;
    auto data_path = directory;

    if (exists(data_path, ec))
        return true;

    if (ec.value() == directory_not_found)
    {
        log::error(LOG_SERVER) << format(BS_UNINITIALIZED_CHAIN) % data_path;
        return false;
    }

    const auto message = ec.message();
    log::error(LOG_SERVER) << format(BS_INITCHAIN_TRY) % data_path % message;
    return false;
}

// Increase the configured minomum as required to operate the service.
void executor::set_minimum_threadpool_size()
{
    metadata_.configured.network.threads =
        std::max(metadata_.configured.network.threads,
            server_node::threads_required(metadata_.configured));
}

} // namespace server
} // namespace libbitcoin
