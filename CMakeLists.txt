cmake_minimum_required(VERSION 3.10)
project(input_switch C)

set(CMAKE_C_STANDARD 99)

if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT /wd4996 /wd4068")
    set(CompilerFlags
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE)
    foreach (CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach ()
endif ()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS "-pipe -Wall -Wextra -nostartfiles -Wl,--gc-sections,--build-id=none -mwindows -D_NO_START_FILES")
    set(CMAKE_C_FLAGS_DEBUG "-g -D_DEBUG")
    set(CMAKE_C_FLAGS_RELEASE "-s -O3 -Os -fmerge-all-constants -fno-asynchronous-unwind-tables -fdata-sections -ffunction-sections")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_C_FLAGS}")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64 bits
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/x86_64.ldscript")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/x86_64.ldscript")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        # 32 bits
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/i386.ldscript")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/i386.ldscript")
    endif()
endif ()

add_executable(input_switch main.c)
target_link_libraries(input_switch ntdll)