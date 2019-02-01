FILE(GLOB bitcoinmath_SOURCES "*.c")

ADD_LIBRARY(bitcoinmath_static STATIC ${bitcoinmath_SOURCES})
SET_TARGET_PROPERTIES(bitcoinmath_static PROPERTIES OUTPUT_NAME mvs_bitcoinmath)
TARGET_LINK_LIBRARIES(bitcoinmath_static)
INSTALL(TARGETS bitcoinmath_static DESTINATION lib)

IF(ENABLE_SHARED_LIBS)
  ADD_LIBRARY(bitcoinmath_shared SHARED ${bitcoinmath_SOURCES})
  SET_TARGET_PROPERTIES(bitcoinmath_shared PROPERTIES OUTPUT_NAME mvs_bitcoinmath)
  TARGET_LINK_LIBRARIES(bitcoinmath_shared)
  INSTALL(TARGETS bitcoinmath_shared DESTINATION lib)
ENDIF()
