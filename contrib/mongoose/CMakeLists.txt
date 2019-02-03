FILE(GLOB_RECURSE mongoose_SOURCES "*.c")

IF(MG_ENABLE_DEBUG)
ADD_DEFINITIONS(-DMG_ENABLE_DEBUG)
ELSE()
  ADD_DEFINITIONS(-DMG_DISABLE_STDIO)
ENDIF()
                                                                                
ADD_LIBRARY(mongoose_static STATIC ${mongoose_SOURCES})
SET_TARGET_PROPERTIES(mongoose_static PROPERTIES OUTPUT_NAME mongoose)
TARGET_LINK_LIBRARIES(mongoose_static)
INSTALL(TARGETS mongoose_static DESTINATION lib)

IF(ENABLE_SHARED_LIBS)
  ADD_LIBRARY(mongoose_shared SHARED ${mongoose_SOURCES})
  SET_TARGET_PROPERTIES(mongoose_shared PROPERTIES OUTPUT_NAME mongoose)
  TARGET_LINK_LIBRARIES(mongoose_shared)
  INSTALL(TARGETS mongoose_shared DESTINATION lib)
ENDIF()
