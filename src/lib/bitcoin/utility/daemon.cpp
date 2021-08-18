/**
 * Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/bitcoin/utility/daemon.hpp>

#ifndef _WIN32

#include <cerrno>
#include <cstdlib> // quick_exit()
#include <fcntl.h> // open()
#include <unistd.h> // fork()
#include <stdexcept>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#endif

namespace libbitcoin {
void daemon() {
#ifndef _WIN32
    pid_t pid { fork() };
    if (pid < 0) {
        throw std::runtime_error { "fork() failed" };
    }

    if (pid != 0) {
        // Exit parent process using system version of exit() to avoid flushing standard streams.
        // FIXME: use quick_exit() when available on OSX.
        _exit(0);
    }

    // Detach from controlling terminal by making process a session leader.
    if (setsid() < 0) {
        throw std::runtime_error { "setsid() failed" };
    }

    // Forking again ensures that the daemon process is not a session leader, and therefore cannot
    // regain access to a controlling terminal.
    pid = fork();
    if (pid < 0) {
        throw std::runtime_error { "fork() failed" };
    }

    if (pid != 0) {
        // FIXME: use quick_exit() when available on OSX.
        _exit(0);
    }

    // Re-open standard input.
    close(STDIN_FILENO);
    if (open("/dev/null", O_RDONLY) < 0) {
        throw std::runtime_error { "open() failed" };
    }

    // Close all non-standard file handles.
    const long fds{ sysconf(_SC_OPEN_MAX) };
    for (long fd { STDERR_FILENO + 1 }; fd < fds; ++fd) {
        close(fd);
    }

    // Note that the standard output handles are unchanged.
#endif
}

}        //libbitcoin

