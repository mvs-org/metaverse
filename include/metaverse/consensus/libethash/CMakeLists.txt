set(LIBRARY ethash)

if (CPPETHEREUM)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif ()

set(CMAKE_BUILD_TYPE Release)

if (NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
endif()

set(FILES 	util.h
          	io.c
          	internal.c
          	ethash.h
          	endian.h
          	compiler.h
          	fnv.h
          	data_sizes.h)

if (MSVC)
	list(APPEND FILES util_win32.c io_win32.c mmap_win32.c)
else()
	list(APPEND FILES io_posix.c)
endif()

if (NOT CRYPTOPP_FOUND)
    # libethash has implement of cryption source code if not found CryptoPP LIBRARY.
    find_package(CryptoPP 5.6.2 QUIET)
endif()

list(APPEND FILES sha3.c sha3.h)

add_library(ethash ${FILES})

if (CRYPTOPP_FOUND)
    MESSAGE(STATUS "CryptoPP found")
	TARGET_LINK_LIBRARIES(ethash ${CRYPTOPP_LIBRARIES})
else()
    MESSAGE(STATUS "CryptoPP not found, use existed implementation")
endif()

if (UNIX AND NOT APPLE)
	if (ANDROID) 
		TARGET_LINK_LIBRARIES(ethash c++_static c++abi atomic m)
	else()
		TARGET_LINK_LIBRARIES(ethash pthread m)
	endif()
endif()

install(TARGETS ethash RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
