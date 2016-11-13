
FILE(GLOB_RECURSE mvs_net_test_SOURCES "*.cpp")

ADD_EXECUTABLE(net-test ${mvs_net_test_SOURCES})

IF(ENABLE_SHARED_LIBS)
TARGET_LINK_LIBRARIES(net-test boost_unit_test_framework ${Boost_LIBRARIES} ${network_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY})
ELSE()
TARGET_LINK_LIBRARIES(net-test libboost_unit_test_framework.a ${Boost_LIBRARIES} ${network_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY})
ENDIF()

INSTALL(TARGETS net-test DESTINATION bin)
