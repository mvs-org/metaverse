# for C library
ADD_SUBDIRECTORY(math/external)

FILE(GLOB_RECURSE bitcoin_SOURCES "*.cpp")

ADD_DEFINITIONS(-DBC_STATIC=1)
ADD_LIBRARY(bitcoin_static STATIC ${bitcoin_SOURCES})
SET_TARGET_PROPERTIES(bitcoin_static PROPERTIES OUTPUT_NAME mvs_bitcoin)
TARGET_LINK_LIBRARIES(bitcoin_static ${Boost_LIBRARIES} libsecp256k1.a ${bitcoinmath_LIBRARY})
INSTALL(TARGETS bitcoin_static DESTINATION lib)

IF(ENABLE_SHARED_LIBS)
    ADD_DEFINITIONS(-DBC_DLL=1)
  ADD_LIBRARY(bitcoin_shared SHARED ${bitcoin_SOURCES})
  SET_TARGET_PROPERTIES(bitcoin_shared PROPERTIES OUTPUT_NAME mvs_bitcoin)
  TARGET_LINK_LIBRARIES(bitcoin_shared ${Boost_LIBRARIES} secp256k1 ${bitcoinmath_LIBRARY})
  INSTALL(TARGETS bitcoin_shared DESTINATION lib)
ENDIF()
