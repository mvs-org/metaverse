FILE(GLOB_RECURSE mvs_net_test_SOURCES "*.cpp")

ADD_EXECUTABLE(mpc1-test ${mvs_net_test_SOURCES})

IF(ENABLE_SHARED_LIBS)
TARGET_LINK_LIBRARIES(mpc1-test boost_unit_test_framework ${Boost_LIBRARIES}
    ${network_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY}
    ${database_LIBRARY} ${consensus_LIBRARY})
ELSE()
TARGET_LINK_LIBRARIES(mpc1-test libboost_unit_test_framework.a ${Boost_LIBRARIES}
    ${network_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY}
    ${database_LIBRARY}
    ${consensus_LIBRARY} ${blockchain_LIBRARY})
ENDIF()

INSTALL(TARGETS mpc1-test DESTINATION bin)
