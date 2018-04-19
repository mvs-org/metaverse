FILE(GLOB_RECURSE jsoncpp_SOURCES "*.cpp")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

ADD_LIBRARY(jsoncpp_static STATIC ${jsoncpp_SOURCES})
SET_TARGET_PROPERTIES(jsoncpp_static PROPERTIES OUTPUT_NAME jsoncpp)
TARGET_LINK_LIBRARIES(jsoncpp_static)
INSTALL(TARGETS jsoncpp_static DESTINATION lib)

IF(ENABLE_SHARED_LIBS)
  ADD_LIBRARY(jsoncpp_shared SHARED ${jsoncpp_SOURCES})
  SET_TARGET_PROPERTIES(jsoncpp_shared PROPERTIES OUTPUT_NAME jsoncpp)
  TARGET_LINK_LIBRARIES(jsoncpp_shared)
  INSTALL(TARGETS jsoncpp_shared DESTINATION lib)
ENDIF()
