#https://github.com/meemknight/raylibCmakeSetup
#Version 1.0.0

cmake_minimum_required(VERSION 4.0)
project(mygame)
set(CMAKE_CXX_STANDARD 20)

if(EMSCRIPTEN)
    message(STATUS "Building with Emscripten")


    # Optimization and useful linker flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3 -s ASYNCIFY -s WASM=1 -s USE_PTHREADS=1 -pthread -s ALLOW_MEMORY_GROWTH=1")

    # Preload resources (so RESOURCES_PATH works)
    #  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/resources@/resources")

    # No console spam in browser
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXIT_RUNTIME=0 -s ASSERTIONS=0")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --shell-file ${CMAKE_SOURCE_DIR}/src/shell.html")
    add_executable("${CMAKE_PROJECT_NAME}")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PUBLIC -sUSE_GLFW=3)

endif()
#! ! ! ! ! ! !
#set this to true to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!
option(PRODUCTION_BUILD "Make this a production build" OFF)
#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!

if(MSVC) 
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
else()
    # GCC/Clang equivalent settings
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization

# Ensure raylib is built as static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Raylib as static libraries" FORCE)

# Compiler-specific optimizations
if(MSVC)
    add_compile_options(/arch:AVX2) # SIMD optimizations for MSVC
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC and Clang equivalent for AVX2
    if(EMSCRIPTEN)
    add_compile_options(-mavx2 -msimd128)
    else()
      add_compile_options(-mavx2)
    endif()
    # Additional optimization flags for GCC
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(EMSCRIPTEN)
          add_compile_options(-O3 -flto -ftree-vectorize)
        elseif()
          add_compile_options(-O3 -flto -ftree-vectorize -march=native)
    
        endif()
    endif()
endif()






#Add raylib, and here you would add other libraries if needed!
add_subdirectory(thirdparty/raylib-5.5)
add_subdirectory(thirdparty/imgui-docking)
add_subdirectory(thirdparty/rlImGui)


# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(NOT EMSCRIPTEN)
  add_executable("${CMAKE_PROJECT_NAME}")

#else()
#add_executable("${CMAKE_PROJECT_NAME}")
endif()
set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 20)



if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") 

	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1) 

else()
  if(EMSCRIPTEN)
        target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="/resources/")
    else()
      # This is useful to get an ASSETS_PATH in your IDE during development
	    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0) 
    endif()
endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )


if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)


	#YOU CAN REMOVE THE CONSOLE WITH THIS LINE! YOU CAN EVEN DO AN IF PRODUCTION_BUILD TO REMOVE IT ONLY IN PRODUCTION BUILDS
    # if(PRODUCTION_BUILD)
    #     set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    # endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Additional warnings for GCC/Clang
    target_compile_options("${CMAKE_PROJECT_NAME}" PRIVATE -Wall -Wextra)
    
    # Position Independent Code - often needed for modern GCC
    set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()


target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")



# Link raylib library statically, Here wou would add other libraries!
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE 
	raylib_static imgui rlimgui
)
