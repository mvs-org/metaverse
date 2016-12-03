#ADD_DEFINITIONS(-DACCOUNT_TESTS=1)
#ADD_DEFINITIONS(-DDATABASE_TESTS=1)
ADD_DEFINITIONS(-DBLOCK_CHAIN_IMPL_TESTS=1)
FILE(GLOB_RECURSE mvs_net_test_SOURCES "*.cpp")

ADD_EXECUTABLE(database-test ${mvs_net_test_SOURCES})

IF(ENABLE_SHARED_LIBS)
TARGET_LINK_LIBRARIES(database-test boost_unit_test_framework ${Boost_LIBRARIES}
    ${network_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY}
    ${explorer_LIBRARY} ${database_LIBRARY} ${consensus_LIBRARY})
ELSE()
TARGET_LINK_LIBRARIES(database-test libboost_unit_test_framework.a ${Boost_LIBRARIES}
    ${network_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY}
    ${explorer_LIBRARY} ${database_LIBRARY}
    ${consensus_LIBRARY} ${blockchain_LIBRARY})
ENDIF()

INSTALL(TARGETS database-test DESTINATION bin)
