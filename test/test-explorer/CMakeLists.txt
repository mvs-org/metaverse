
FILE(GLOB_RECURSE test-explorer_SOURCES "*.cpp")

ADD_EXECUTABLE(test-explorer ${test-explorer_SOURCES})


IF(ENABLE_SHARED_LIBS)
    TARGET_LINK_LIBRARIES(test-explorer boost_unit_test_framework ${Boost_LIBRARIES} ${bitcoin_LIBRARY}
        ${explorer_LIBRARY})
ELSE()
    TARGET_LINK_LIBRARIES(test-explorer libboost_unit_test_framework.a ${Boost_LIBRARIES} ${bitcoin_LIBRARY}
        ${explorer_LIBRARY})
ENDIF()

INSTALL(TARGETS test-explorer DESTINATION bin)
