FILE(GLOB_RECURSE mvsd_SOURCES "*.cpp")

ADD_EXECUTABLE(mvsd ${mvsd_SOURCES})

IF (APPLE)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-range-loop-analysis")
ELSE()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-range-loop-construct")
ENDIF()

IF(ENABLE_SHARED_LIBS)
    ADD_DEFINITIONS(-DBCS_DLL=1)
    TARGET_LINK_LIBRARIES(mvsd ${Boost_LIBRARIES} ${network_LIBRARY} ${database_LIBRARY} ${consensus_LIBRARY}
    ${blockchain_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY} ${node_LIBRARY}
    ${protocol_LIBRARY} ${client_LIBRARY} ${explorer_LIBRARY} ${cryptojs_LIBRARY} ${sodium_LIBRARY})
ELSE()
    ADD_DEFINITIONS(-DBCS_STATIC=1)
    TARGET_LINK_LIBRARIES(mvsd ${Boost_LIBRARIES} ${network_LIBRARY} ${database_LIBRARY} ${consensus_LIBRARY}
    ${blockchain_LIBRARY} ${sodium_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY} ${node_LIBRARY}
    ${protocol_LIBRARY} ${client_LIBRARY} ${explorer_LIBRARY} ${cryptojs_LIBRARY} )
ENDIF()

INSTALL(TARGETS mvsd DESTINATION bin)
