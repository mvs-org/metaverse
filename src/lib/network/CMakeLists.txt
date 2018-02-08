FILE(GLOB_RECURSE network_SOURCES "*.cpp")

ADD_DEFINITIONS(-DBCT_STATIC=1)
# gcc-6 boost/multiprecision/cpp_int.hpp:181
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

IF(USE_UPNP)
    # use tinyformat.h
    INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src/lib/consensus/clone")

    IF(ENABLE_SHARED_LIBS)
        ADD_LIBRARY(miniupnpc_shared SHARED IMPORTED)
        SET_TARGET_PROPERTIES(miniupnpc_shared PROPERTIES IMPORTED_LOCATION ${miniupnpc_ROOT_DIR}/lib/libminiupnpc.so)
    ELSE()
        ADD_LIBRARY(miniupnpc_static STATIC IMPORTED)
        SET_TARGET_PROPERTIES(miniupnpc_static PROPERTIES IMPORTED_LOCATION ${miniupnpc_ROOT_DIR}/lib/libminiupnpc.a)
    ENDIF()
ENDIF()

ADD_LIBRARY(network_static STATIC ${network_SOURCES})
SET_TARGET_PROPERTIES(network_static PROPERTIES OUTPUT_NAME mvs_network)
TARGET_LINK_LIBRARIES(network_static ${Boost_LIBRARIES} ${miniupnpc_LIBRARY})
INSTALL(TARGETS network_static DESTINATION lib)

IF(ENABLE_SHARED_LIBS)
    ADD_DEFINITIONS(-DBCT_DLL=1)
  ADD_LIBRARY(network_shared SHARED ${network_SOURCES})
  SET_TARGET_PROPERTIES(network_shared PROPERTIES OUTPUT_NAME mvs_network)
  TARGET_LINK_LIBRARIES(network_shared ${Boost_LIBRARIES} ${miniupnpc_LIBRARY})
  INSTALL(TARGETS network_shared DESTINATION lib)
ENDIF()
