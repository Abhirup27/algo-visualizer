cmake_minimum_required(VERSION 4.0)
project(mygame)
set(CMAKE_CXX_STANDARD 20)
# Always generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------
# Keep compile_commands.json symlinked to project root
# --------------------------------
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR) # skip in in-source builds
    add_custom_target(link_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        BYPRODUCTS ${CMAKE_SOURCE_DIR}/compile_commands.json
        COMMENT "Updating symlink: compile_commands.json → project root"
    )
endif()

# -------------------------------
# EMSCRIPTEN CONFIG
# -------------------------------
if(EMSCRIPTEN)
    message(STATUS "Building with Emscripten")

    include_directories(
        ${CMAKE_SYSROOT}/include
        ${CMAKE_SYSROOT}/include/emscripten
    )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        -s USE_GLFW=3 \
        -s ASYNCIFY \
        -s WASM=1 \
        -s USE_PTHREADS=1 \
        -pthread \
        -s ALLOW_MEMORY_GROWTH=1")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        -s EXIT_RUNTIME=0 \
        -s ASSERTIONS=0 \
        --shell-file ${CMAKE_SOURCE_DIR}/src/shell.html")

    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    execute_process(COMMAND em++ --cflags OUTPUT_VARIABLE EM_CFLAGS)
    string(STRIP "${EM_CFLAGS}" EM_CFLAGS)
endif()

# -------------------------------
# DEBUG / RELEASE SETTINGS
# -------------------------------
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/arch:AVX2)
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # LTO

# -------------------------------
# COMPILER OPTIMIZATIONS
# -------------------------------
if(MSVC)
    add_compile_options(/arch:AVX2)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

    if(EMSCRIPTEN)
        add_compile_options(-mavx2 -msimd128)
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_compile_options(-O3 -flto -ftree-vectorize)
        endif()
    else()
        add_compile_options(-mavx2)
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_compile_options(-O3 -flto -ftree-vectorize -march=native)
        endif()
    endif()
endif()

# -------------------------------
# DEPENDENCIES
# -------------------------------
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Raylib as static libraries" FORCE)
add_subdirectory(thirdparty/raylib-5.5)
add_subdirectory(thirdparty/imgui-docking)
add_subdirectory(thirdparty/rlImGui)

# -------------------------------
# SOURCES
# -------------------------------
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# -------------------------------
# EXECUTABLE
# -------------------------------
add_executable("${PROJECT_NAME}" ${MY_SOURCES})
set_property(TARGET "${PROJECT_NAME}" PROPERTY CXX_STANDARD 20)

if(EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html" COMPILE_FLAGS "${EM_CFLAGS}")
endif()

# -------------------------------
# DEFINES
# -------------------------------
if(EMSCRIPTEN)
  target_compile_definitions("${PROJECT_NAME}" PUBLIC RESOURCES_PATH="/resources/")
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_definitions("${PROJECT_NAME}" PUBLIC
            RESOURCES_PATH="./resources/"
            PRODUCTION_BUILD=1
        )
    else()
        target_compile_definitions("${PROJECT_NAME}" PUBLIC
            RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/"
            PRODUCTION_BUILD=0
        )
    endif()
endif()

# -------------------------------
# INCLUDE + LIBS
# -------------------------------
target_include_directories("${PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_link_libraries("${PROJECT_NAME}" PRIVATE raylib_static imgui rlimgui)

if(MSVC)
    target_compile_definitions("${PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)
else()
    set_property(TARGET "${PROJECT_NAME}" PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# -------------------------------
# AUTO-COPY RESOURCES (native Release only)
# -------------------------------
if(NOT EMSCRIPTEN)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    )
endif()

