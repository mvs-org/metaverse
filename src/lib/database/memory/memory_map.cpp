/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
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
#include <metaverse/database/memory/memory_map.hpp>

#include <iostream>

#ifdef _WIN32
    #include <io.h>
    #include "../mman-win32/mman.h"
    #define FILE_OPEN_PERMISSIONS _S_IREAD | _S_IWRITE
#else
    #include <unistd.h>
    #include <stddef.h>
    #include <sys/mman.h>
    #define FILE_OPEN_PERMISSIONS S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#endif
#include <cstddef>
#include <cstdint>
#include <fcntl.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/accessor.hpp>
#include <metaverse/database/memory/allocator.hpp>
#include <metaverse/database/memory/memory.hpp>

// memory_map is be able to support 32 bit but because the database
// requires a larger file this is not validated or supported.
#ifndef __ANDROID__
static_assert(sizeof(void*) == sizeof(uint64_t), "Not a 64 bit system!");
#endif

namespace libbitcoin {
namespace database {

using boost::filesystem::path;

#define EXPANSION_NUMERATOR 150
#define EXPANSION_DENOMINATOR 100

size_t memory_map::file_size(int file_handle)
{
    if (file_handle == -1)
        return 0;

    // This is required because off_t is defined as long, whcih is 32 bits in
    // msvc and 64 bits in linux/osx, and stat contains off_t.
#ifdef _WIN32
#ifdef _WIN64
    struct _stat64 sbuf;
    if (_fstat64(file_handle, &sbuf) == -1)
        return 0;
#else
    struct _stat32 sbuf;
    if (_fstat32(file_handle, &sbuf) == -1)
        return 0;
#endif
#else
    struct stat sbuf;
    if (fstat(file_handle, &sbuf) == -1)
        return 0;
#endif

    // Convert signed to unsigned size.
    BITCOIN_ASSERT_MSG(sbuf.st_size > 0, "File size cannot be 0 bytes.");
    return static_cast<size_t>(sbuf.st_size);
}

int memory_map::open_file(const path& filename)
{
#ifdef _WIN32
    int handle = _wopen(filename.wstring().c_str(), O_RDWR,
        FILE_OPEN_PERMISSIONS);
#else
    int handle = open(filename.string().c_str(), O_RDWR,
        FILE_OPEN_PERMISSIONS);
#endif
    return handle;
}

bool memory_map::handle_error(const std::string& context,
    const path& filename)
{
#ifdef _WIN32
    const auto error = GetLastError();
#else
    const auto error = errno;
#endif
    log::fatal(LOG_DATABASE)
        << "The file failed to " << context << ": "
        << filename << " : " << error;
    return false;
}

void memory_map::log_mapping()
{
    log::debug(LOG_DATABASE)
        << "Mapping: " << filename_ << " [" << file_size_
        << "] (" << page() << ")";
}

void memory_map::log_resizing(size_t size)
{
    log::debug(LOG_DATABASE)
        << "Resizing: " << filename_ << " [" << size << "]";
}

void memory_map::log_unmapped()
{
    log::debug(LOG_DATABASE)
        << "Unmapped: " << filename_ << " [" << logical_size_ << "]";
}

// mmap documentation: tinyurl.com/hnbw8t5
memory_map::memory_map(const path& filename)
  : file_handle_(open_file(filename)),
    filename_(filename),
    data_(nullptr),
    file_size_(file_size(file_handle_)),
    logical_size_(file_size_),
    closed_(true),
    stopped_(true)
{
}

memory_map::memory_map(const path& filename, mutex_ptr mutex)
  : memory_map(filename)
{
    remap_mutex_ = mutex;
}

// Database threads must be joined before close is called (or destruct).
memory_map::~memory_map()
{
    close();
}

// Startup and shutdown.
// ----------------------------------------------------------------------------

// Map the database file and signal start.
bool memory_map::start()
{
    // Critical Section (internal/unconditional)
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (!stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        // Start is not idempotent (should be called on single thread).
        return false;
    }

    mutex_.unlock_upgrade_and_lock();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    std::string error_name;

    // Initialize data_.
    if (!map(file_size_))
        error_name = "map";
    else if (madvise(data_, 0, MADV_RANDOM) == -1)
        error_name = "madvise";
    else
    {
        closed_ = false;
        stopped_ = false;
    }

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    // Keep logging out of the critical section.
    if (!error_name.empty())
        return handle_error(error_name, filename_);

    log_mapping();
    return true;
}

bool memory_map::stop()
{
    // Critical Section (internal/unconditional)
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        // Stop is idempotent (may be called from multiple threads).
        return true;
    }

    mutex_.unlock_upgrade_and_lock();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    stopped_ = true;

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return true;
}

// Close does not call stop because there is no way to detect thread join.
bool memory_map::close()
{
    std::string error_name;

    // Critical Section (internal/unconditional)
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (closed_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        // Close is idempotent (may be called from multiple threads).
        return true;
    }

    mutex_.unlock_upgrade_and_lock();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    closed_ = true;

    if (msync(data_, logical_size_, MS_SYNC) == -1)
        error_name = "msync";
    else if (munmap(data_, file_size_) == -1)
        error_name = "munmap";
    else if (ftruncate(file_handle_, logical_size_) == -1)
        error_name = "ftruncate";
    else if (fsync(file_handle_) == -1)
        error_name = "fsync";
    else if (::close(file_handle_) == -1)
        error_name = "close";

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    // Keep logging out of the critical section.
    if (!error_name.empty())
        return handle_error(error_name, filename_);

    log_unmapped();
    return true;
}

bool memory_map::stopped() const
{
    // Critical Section (internal/unconditional)
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    return stopped_;
    ///////////////////////////////////////////////////////////////////////////
}

// Operations.
// ----------------------------------------------------------------------------

size_t memory_map::size() const
{
    // Critical Section (internal)
    ///////////////////////////////////////////////////////////////////////////
    REMAP_READ(mutex_);

    return file_size_;
    ///////////////////////////////////////////////////////////////////////////
}

// throws runtime_error
memory_ptr memory_map::access()
{
    return REMAP_ACCESSOR(data_, mutex_);
}

// throws runtime_error
memory_ptr memory_map::resize(size_t size)
{
    return reserve(size, EXPANSION_DENOMINATOR);
}

// throws runtime_error
memory_ptr memory_map::reserve(size_t size)
{
    return reserve(size, EXPANSION_NUMERATOR);
}

// throws runtime_error
// There is no way to gracefully recover from a resize failure because there
// are integrity relationships across multiple database files. Stopping a write
// in one would require rolling back preceding write operations in others.
// To handle this situation without database corruption would require predicting
// the required allocation and all resizing before writing a block.
memory_ptr memory_map::reserve(size_t size, size_t expansion)
{
    ///////////////////////////////////////////////////////////////////////////
    // Internally preventing resize during close is not possible because of
    // cross-file integrity. So we must coalesce all threads before closing.

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    const auto memory = REMAP_ALLOCATOR(mutex_);

    // The store should only have been closed after all threads terminated.
    if (closed_)
    {
        REMAP_DOWNGRADE(memory, data_);
        throw std::runtime_error("Resize failure, store already closed.");
    }

    if (size > file_size_)
    {
        const size_t target = size * expansion / EXPANSION_DENOMINATOR;

        mutex_.unlock_upgrade_and_lock();
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        // TODO: isolate cause and if recoverable (disk size) return nullptr.
        // All existing database pointers are invalidated by this call.
        if (!truncate_mapped(target))
        {
            handle_error("resize", filename_);
            throw std::runtime_error("Resize failure, disk space may be low.");
        }

        //---------------------------------------------------------------------
        mutex_.unlock_and_lock_upgrade();
    }

    logical_size_ = size;
    REMAP_DOWNGRADE(memory, data_);

    // Always return in shared lock state.
    // The critical section does not end until this shared pointer is freed.
    return memory;
    ///////////////////////////////////////////////////////////////////////////
}

// privates
// ----------------------------------------------------------------------------

size_t memory_map::page()
{
#ifdef _WIN32
    SYSTEM_INFO configuration;
    GetSystemInfo(&configuration);
    return configuration.dwPageSize;
#else
    errno = 0;
    const auto page_size = sysconf(_SC_PAGESIZE);

    // -1 is both a return code and a potentially valid value, so use errno.
    if (errno != 0)
        handle_error("sysconf", filename_);

    BITCOIN_ASSERT(static_cast<size_t>(page_size) <= max_size_t);
    return static_cast<size_t>(page_size == -1 ? 0 : page_size);
#endif
}

bool memory_map::unmap()
{
    const auto success = (munmap(data_, file_size_) != -1);
    file_size_ = 0;
    data_ = nullptr;
    return success;
}

bool memory_map::map(size_t size)
{
    if (size == 0)
        return false;

    data_ = reinterpret_cast<uint8_t*>(mmap(0, size, PROT_READ | PROT_WRITE,
        MAP_SHARED, file_handle_, 0));

    return validate(size);
}

bool memory_map::remap(size_t size)
{
#ifdef MREMAP_MAYMOVE
    data_ = reinterpret_cast<uint8_t*>(mremap(data_, file_size_, size,
        MREMAP_MAYMOVE));

    return validate(size);
#else
    return unmap() && map(size);
#endif
}

bool memory_map::truncate(size_t size)
{
    return ftruncate(file_handle_, size) != -1;
}

bool memory_map::truncate_mapped(size_t size)
{
    log_resizing(size);

    // Critical Section (conditional/external)
    ///////////////////////////////////////////////////////////////////////////
    conditional_lock lock(remap_mutex_);

#ifndef MREMAP_MAYMOVE
    if (!unmap())
        return false;
#endif

    if (!truncate(size))
        return false;

#ifndef MREMAP_MAYMOVE
    return map(size);
#else
    return remap(size);
#endif
    ///////////////////////////////////////////////////////////////////////////
}

bool memory_map::validate(size_t size)
{
    if (data_ == MAP_FAILED)
    {
        file_size_ = 0;
        data_ = nullptr;
        return false;
    }

    file_size_ = size;
    return true;
}

} // namespace database
} // namespace libbitcoin
