# Module for locating the miniupnpc library.
find_path(miniupnpc_ROOT_DIR
    NAMES include/miniupnpc/miniupnpc.h
)
find_library(miniupnpc_LIBRARIES
    NAMES miniupnpc libminiupnpc
    HINTS ${miniupnpc_ROOT_DIR}/lib
)
find_path(miniupnpc_INCLUDE_DIRS
    NAMES miniupnpc/miniupnpc.h
    HINTS ${miniupnpc_ROOT_DIR}/include
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(miniupnpc DEFAULT_MSG
    miniupnpc_LIBRARIES
    miniupnpc_INCLUDE_DIRS
)
mark_as_advanced(
    miniupnpc_ROOT_DIR
    miniupnpc_LIBRARIES
    miniupnpc_INCLUDE_DIRS
)
