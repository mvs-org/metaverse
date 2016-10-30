find_path(secp256k1_ROOT_DIR
    NAMES include/secp256k1.h
)
find_library(secp256k1_LIBRARIES
    NAMES secp256k1 libsecp256k1
    HINTS ${secp256k1_ROOT_DIR}/lib
)
find_path(secp256k1_INCLUDE_DIRS
    NAMES secp256k1.h
    HINTS ${secp256k1_ROOT_DIR}/include
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(secp256k1 DEFAULT_MSG
    secp256k1_LIBRARIES
    secp256k1_INCLUDE_DIRS
)
mark_as_advanced(
    secp256k1_ROOT_DIR
    secp256k1_LIBRARIES
    secp256k1_INCLUDE_DIRS
)

#if (secp256k1_INCLUDE_DIRS)
#	set(secp256k1_LIB_VERSION 0)
#	file(STRINGS "${Boost_INCLUDE_DIR}/zmq.h" _zmq_VERSION_H_CONTENTS REGEX "#define ZMQ_VERSION_*")
#endif()
