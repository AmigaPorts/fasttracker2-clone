cmake_minimum_required(VERSION 3.7)

project(ft2-clone)

find_package(SDL REQUIRED)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ft2-clone_SOURCE_DIR}/release/other/")


file(GLOB ft2-clone_SRC
#		"${ft2-clone_SOURCE_DIR}/src/rtmidi/*.cpp"
		"${ft2-clone_SOURCE_DIR}/src/sdl1_wrapper.cpp"
		"${ft2-clone_SOURCE_DIR}/src/*.c"
		"${ft2-clone_SOURCE_DIR}/src/gfxdata/*.c"
)

add_executable(ft2-clone ${ft2-clone_SRC})

target_include_directories(ft2-clone SYSTEM
    PRIVATE ${SDL_INCLUDE_DIR})
message("SDL include dirs: " ${SDL_INCLUDE_DIR})

if(AMIGA)
	target_link_libraries(ft2-clone PRIVATE m pthread ${SDL_LIBRARY} -ldebug)
else()
	target_link_libraries(ft2-clone PRIVATE m pthread ${SDL_LIBRARY})
endif()
target_compile_definitions(ft2-clone PRIVATE __LINUX_ALSA__)


install(TARGETS ft2-clone
        RUNTIME DESTINATION bin )
