cmake_minimum_required(VERSION 3.18)

project(rDOOM LANGUAGES C)

#########
# Build #
#########

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(X11 REQUIRED)
# As of 3.20, cmake doesn't support xcb-render in X11 lookup.
# But it can find the library if available.
# This might be a problem for include paths if xcb/render.h is non-standard
find_library(X11_xcb_render_LIB xcb-render REQUIRED)

include_directories(src ${PROJECT_BINARY_DIR} ${X11_xcb_INCLUDE_PATH})

macro(add_language language)
	add_custom_command(
		OUTPUT "l_${language}.inc"
		COMMAND genlang -o "l_${language}.inc" "${PROJECT_SOURCE_DIR}/langs/${language}.lang"
		DEPENDS "langs/${language}.lang"
	)
	list(APPEND RDOOM_LANGS "l_${language}.inc")
endmacro()

add_language(english)
add_language(french)

file(GLOB RDOOM_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c)
add_executable(rdoom ${RDOOM_SOURCES} ${RDOOM_LANGS})

file(GLOB READWAD_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/tools/readwad/*.c)
list(APPEND READWAD_SOURCES src/w_wad.c src/i_filemap.c src/i_error.c src/m_swap.c)
add_executable(readwad ${READWAD_SOURCES})

file(GLOB GENLANG_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/tools/genlang/*.c)
add_executable(genlang ${GENLANG_SOURCES})

target_link_libraries(rdoom PUBLIC ${X11_xcb_LIB} ${X11_xcb_render_LIB})

