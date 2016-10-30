FILE(GLOB_RECURSE protocol_SOURCES "*.cpp")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")

ADD_DEFINITIONS(-DBCP_STATIC=1)

ADD_LIBRARY(protocol_static STATIC ${protocol_SOURCES})
SET_TARGET_PROPERTIES(protocol_static PROPERTIES OUTPUT_NAME mvs_protocol)
TARGET_LINK_LIBRARIES(protocol_static libzmq.a ${Boost_LIBRARIES} ${bitcoin_LIBRARY})
INSTALL(TARGETS protocol_static DESTINATION lib)

IF(ENABLE_SHARED_LIBS)
    ADD_DEFINITIONS(-DBCP_DLL=1)
  ADD_LIBRARY(protocol_shared SHARED ${protocol_SOURCES})
  SET_TARGET_PROPERTIES(protocol_shared PROPERTIES OUTPUT_NAME mvs_protocol)
  TARGET_LINK_LIBRARIES(protocol_shared zmq ${Boost_LIBRARIES} ${bitcoin_LIBRARY})
  INSTALL(TARGETS protocol_shared DESTINATION lib)
ENDIF()
