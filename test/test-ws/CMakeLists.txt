
FILE(GLOB_RECURSE mvs_ws_test_SOURCES "*.cpp")

ADD_EXECUTABLE(mvs_ws_test ${mvs_ws_test_SOURCES})

IF(ENABLE_SHARED_LIBS)
TARGET_LINK_LIBRARIES(mvs_ws_test boost_unit_test_framework ${Boost_LIBRARIES}
    ${bitcoin_LIBRARY} ${mongoose_LIBRARY}
    ${blockchain_LIBRARY})
ELSE()
TARGET_LINK_LIBRARIES(mvs_ws_test libboost_unit_test_framework.a ${Boost_LIBRARIES}
    ${bitcoin_LIBRARY} ${mongoose_LIBRARY}
    ${blockchain_LIBRARY})
ENDIF()

INSTALL(TARGETS mvs_ws_test DESTINATION bin)
