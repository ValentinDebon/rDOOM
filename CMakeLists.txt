cmake_minimum_required(VERSION 3.18)

project(rDOOM LANGUAGES C)

#########
# Build #
#########

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

find_library(XCB_LIBRARY xcb REQUIRED)
find_library(XCB_RENDER_LIBRARY xcb-render REQUIRED)

add_definitions(-DNORMALUNIX -DLINUX)

include_directories(src)

file(GLOB RDOOM_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c)
add_executable(rdoom ${RDOOM_SOURCES})

file(GLOB READWAD_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/tools/readwad/*.c)
list(APPEND READWAD_SOURCES src/w_wad.c src/i_filemap.c src/i_error.c src/m_swap.c)
add_executable(readwad ${READWAD_SOURCES})

target_link_libraries(rdoom PUBLIC ${XCB_LIBRARY} ${XCB_RENDER_LIBRARY})

