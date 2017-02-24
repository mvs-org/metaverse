# for C library
ADD_SUBDIRECTORY(math/external)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src/lib/consensus/clone")
FILE(GLOB_RECURSE bitcoin_SOURCES "*.cpp")

ADD_DEFINITIONS(-DBC_STATIC=1)

ADD_LIBRARY(secp256k1_static STATIC IMPORTED)                                         
SET_TARGET_PROPERTIES(secp256k1_static PROPERTIES IMPORTED_LOCATION ${secp256k1_ROOT_DIR}/lib/libsecp256k1.a)

ADD_LIBRARY(bitcoin_static STATIC ${bitcoin_SOURCES})
SET_TARGET_PROPERTIES(bitcoin_static PROPERTIES OUTPUT_NAME mvs_bitcoin)
TARGET_LINK_LIBRARIES(bitcoin_static ${Boost_LIBRARIES} secp256k1_static ${bitcoinmath_LIBRARY})
INSTALL(TARGETS bitcoin_static DESTINATION lib)

IF(ENABLE_SHARED_LIBS)
    ADD_DEFINITIONS(-DBC_DLL=1)
  ADD_LIBRARY(bitcoin_shared SHARED ${bitcoin_SOURCES})
  SET_TARGET_PROPERTIES(bitcoin_shared PROPERTIES OUTPUT_NAME mvs_bitcoin)
  TARGET_LINK_LIBRARIES(bitcoin_shared ${Boost_LIBRARIES} secp256k1 ${bitcoinmath_LIBRARY})
  INSTALL(TARGETS bitcoin_shared DESTINATION lib)
ENDIF()
