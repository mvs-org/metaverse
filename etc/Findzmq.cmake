find_path(ZeroMQ_ROOT_DIR
    NAMES include/zmq.h
)
find_library(ZeroMQ_LIBRARIES
    NAMES zmq libzmq
    HINTS ${ZeroMQ_ROOT_DIR}/lib
)
find_path(ZeroMQ_INCLUDE_DIRS
    NAMES zmq.h
    HINTS ${ZeroMQ_ROOT_DIR}/include
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZeroMQ DEFAULT_MSG
    ZeroMQ_LIBRARIES
    ZeroMQ_INCLUDE_DIRS
)
mark_as_advanced(
    ZeroMQ_ROOT_DIR
    ZeroMQ_LIBRARIES
    ZeroMQ_INCLUDE_DIRS
)

#if (ZeroMQ_INCLUDE_DIRS)
#	set(ZeroMQ_LIB_VERSION 0)
#	file(STRINGS "${Boost_INCLUDE_DIR}/zmq.h" _zmq_VERSION_H_CONTENTS REGEX "#define ZMQ_VERSION_*")
#endif()
