cmake_minimum_required(VERSION 3.22.1)

if (NOT DEFINED GADGET_NAME)
    message(WARNING "Module name is not set, defaulting to gotcha-gadget")
    project("gotcha-gadget")
else ()
    project(${GADGET_NAME})
endif()

message("Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 20)

set(LINKER_FLAGS "-ffixed-x18 -Wl,--hash-style=both")
set(C_FLAGS "-fdata-sections -ffunction-sections")
set(CXX_FLAGS "${CXX_FLAGS} -fno-exceptions -fno-rtti")

# Strip and optimize release builds
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(C_FLAGS "${C_FLAGS} -O2 -fvisibility=hidden -fvisibility-inlines-hidden")
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all")
else ()
    set(CFLAGS "${C_FLAGS} -O0")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS} ${CXX_FLAGS}")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}")

# Declares the gotcha gadget that is dynamically injected by the Zygisk module
add_library(${PROJECT_NAME} SHARED gadget.c)

# Propagate debug flag from gradle (-DGOTCHA_DEBUG:BOOL=ON) into a compiler #define
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE GOTCHA_DEBUG)
endif()

target_link_libraries(${PROJECT_NAME} android log)
