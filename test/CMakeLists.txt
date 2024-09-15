cmake_minimum_required(VERSION 3.25)

set(ProjectName HashMapTest)
project(${ProjectName})

include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(FILES "main.cpp;${SOURCE_ROOT}/HashMap.h;${SOURCE_ROOT}/sphash.h;${SOURCE_ROOT}/sphash.cpp;${SOURCE_ROOT}/common.h")

add_executable(${ProjectName} ${FILES})

if(MSVC)
    set(DEFAULT_CXX_FLAGS "/DWIN32 /D_WINDOWS /D_UNICODE /DUNICODE /W4 /WX- /nologo /fp:precise /arch:AVX /Zc:wchar_t /TP /Gd /std:c11")
    if(MSVC_VERSION VERSION_LESS_EQUAL "1900")
        set(DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} /Zc:__cplusplus /std:c++latest")
    else()
        set(DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} /Zc:__cplusplus /std:c++20")
    endif()
    set(CMAKE_CXX_FLAGS "${DEFAULT_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od /RTC1 /Gy /GR- /GS /Gm- /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /Oi /GL /GR- /DNDEBUG /EHsc-")

    set_target_properties(${ProjectName} PROPERTIES
        LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
        LINK_FLAGS_RELEASE "/LTCG /SUBSYSTEM:CONSOLE")

elseif(UNIX)
    set(DEFAULT_C_FLAGS "-Wall -O2 -std=c++17 -std=gnu++17 -std=c99 -march=x86-64-v3 -fno-exceptions")
    set(CMAKE_C_FLAGS "${DEFAULT_C_FLAGS}")

elseif(APPLE)
endif()

