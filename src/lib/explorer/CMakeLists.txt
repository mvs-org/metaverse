FILE(GLOB_RECURSE explorer_SOURCES "*.cpp")

ADD_DEFINITIONS(-DBCX_STATIC=1)

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

ADD_LIBRARY(explorer_static STATIC ${explorer_SOURCES})
SET_TARGET_PROPERTIES(explorer_static PROPERTIES OUTPUT_NAME mvs_explorer)
TARGET_LINK_LIBRARIES(explorer_static libzmq.a ${Boost_LIBRARIES}
    ${bitcoin_LIBRARY} ${network_LIBRARY} ${protocol_LIBRARY} ${client_LIBRARY}
    ${blockchain_LIBRARY})
INSTALL(TARGETS explorer_static DESTINATION lib)

IF(ENABLE_SHARED_LIBS)
    ADD_DEFINITIONS(-DBCX_DLL=1)
  ADD_LIBRARY(explorer_shared SHARED ${explorer_SOURCES})
  SET_TARGET_PROPERTIES(explorer_shared PROPERTIES OUTPUT_NAME mvs_explorer)
  TARGET_LINK_LIBRARIES(explorer_shared zmq ${Boost_LIBRARIES} 
    ${bitcoin_LIBRARY} ${network_LIBRARY} ${protocol_LIBRARY} ${client_LIBRARY} 
    ${blockchain_LIBRARY})
  INSTALL(TARGETS explorer_shared DESTINATION lib)
ENDIF()
