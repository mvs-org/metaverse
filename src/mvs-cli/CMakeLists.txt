
FILE(GLOB_RECURSE mvs-cli_SOURCES "*.cpp")

ADD_DEFINITIONS(-DBCX_STATIC=1)

ADD_EXECUTABLE(mvs-cli ${mvs-cli_SOURCES})

# for experimental/string_view
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

IF(ENABLE_SHARED_LIBS)
    TARGET_LINK_LIBRARIES(mvs-cli ${bitcoin_LIBRARY} ${network_LIBRARY}
        ${protocol_LIBRARY} ${client_LIBRARY} ${explorer_LIBRARY}
        ${database_LIBRARY})
ELSE()
    TARGET_LINK_LIBRARIES(mvs-cli ${bitcoin_LIBRARY} ${network_LIBRARY}
        ${protocol_LIBRARY} ${client_LIBRARY} ${explorer_LIBRARY}
        ${database_LIBRARY})
ENDIF()

INSTALL(TARGETS mvs-cli DESTINATION bin)
