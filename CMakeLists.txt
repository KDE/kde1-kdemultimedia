cmake_minimum_required (VERSION 3.0)

project (kdemultimedia)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

find_package(KDE1 REQUIRED)

add_definitions(-DHAVE_CONFIG_H)

include(CMakePackageConfigHelpers)
include(Qt1Macros)
include(KDE1Macros)
include(KDE1InstallDirs)

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -std=c++98")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

configure_file(common/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(kmedia)
#add_subdirectory(kmid)
#add_subdirectory(kmidi)
#add_subdirectory(kmix)
#add_subdirectory(kscd)

#add_subdirectory(po)