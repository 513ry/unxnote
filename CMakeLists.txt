cmake_minimum_required(VERSION 3.16)
project(unxnote VERSION 0.1.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS OFF)

# Options to enable/disable builds
option(ENABLE_DAEMON "Enable unxnote daemon build" ON)
option(ENABLE_C_CLIENT "Enable unxnote client build" ON)

# Optional code flags
set(UNX_NO_STDIO OFF CACHE BOOL "Build UNX without stdio")

if (UNX_NO_STDIO)
  add_compile_definitions(UNX_NO_STDIO)
endif()

# Source
file(GLOB SOURCE_FILES src/[a-z]*.c)
file(GLOB PSHM_FILES src/pshm/[a-z]*.c)
file(GLOB CLIENT_FILES src/client/[a-z]*.c)

find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(XCB_RANDR REQUIRED xcb-randr)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Daemon build
if (ENABLE_DAEMON)
  list(APPEND SOURCE_FILES ${PSHM_FILES})
  add_executable(${PROJECT_NAME} ${SOURCE_FILES})

  target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${XCB_LIBRARIES}
    ${XCB_RANDR_LIBRARIES}
    ${FREETYPE_LIBRARIES})

  target_include_directories(${PROJECT_NAME}
    PRIVATE
    include
    ${XCB_INCLUDE_DIRS}
    ${XCB_RANDR_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS})
endif()

# Client build
if(ENABLE_C_CLIENT)
  list(APPEND CLIENT_FILES ${PSHM_FILES})
  add_executable(${PROJECT_NAME}_client ${CLIENT_FILES})

  target_link_libraries(${PROJECT_NAME}_client
    PRIVATE)

  target_include_directories(${PROJECT_NAME}_client
    PRIVATE
    include)
endif()
