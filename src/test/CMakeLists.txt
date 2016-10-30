
FILE(GLOB_RECURSE mvs-http_SOURCES "*.cpp")

ADD_EXECUTABLE(mvs-http ${mvs-http_SOURCES})

# for experimental/string_view
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

IF(ENABLE_SHARED_LIBS)
TARGET_LINK_LIBRARIES(mvs-http ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY})
ELSE()
TARGET_LINK_LIBRARIES(mvs-http ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY})
ENDIF()

INSTALL(TARGETS mvs-http DESTINATION bin)
