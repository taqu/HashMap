cmake_minimum_required(VERSION 3.6)

set(ProjectName HashMapTest)
project(${ProjectName})

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")

include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})

set(FILES "main.cpp;HashMap.h;xxHash.h;xxHash.cpp;common.h")

add_executable(${ProjectName} ${FILES})

if(MSVC)
    set_target_properties(${ProjectName} PROPERTIES
        LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
        LINK_FLAGS_RELEASE "/LTCG /SUBSYSTEM:CONSOLE")

elseif(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

elseif(APPLE)
endif()

