cmake_minimum_required(VERSION 3.9)
project(algo-visualizer)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------------------------------------
# Symlink compile_commands.json → project root
# -------------------------------------------------------
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    add_custom_target(link_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        BYPRODUCTS ${CMAKE_SOURCE_DIR}/compile_commands.json
        COMMENT "Symlinking compile_commands.json → project root"
    )
endif()

# -------------------------------------------------------
# FETCH RAYLIB 5.5
# -------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/thirdparty")
set(BUILD_SHARED_LIBS     OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES        OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES           OFF CACHE BOOL "" FORCE)
set(PLATFORM              "Web" CACHE STRING "" FORCE)
set(OPENGL_VERSION        "ES 3.0" CACHE STRING "" FORCE)

FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

# -------------------------------------------------------
# EMSCRIPTEN CONFIG
# -------------------------------------------------------
if(EMSCRIPTEN)
    message(STATUS "Building with Emscripten")

    include_directories(
        ${CMAKE_SYSROOT}/include
        ${CMAKE_SYSROOT}/include/emscripten
    )

    # pthreads disabled at compile time
    add_compile_options(-s USE_PTHREADS=0 --bind)

    set(EM_LINK_FLAGS
        "-s ALLOW_MEMORY_GROWTH=1 \
         -s USE_GLFW=3 \
         -s ASYNCIFY \
         -s WASM=1 \
         -s MODULARIZE=1 \
         -s EXPORT_ES6=1 \
         -s EXPORT_NAME='AlgoVisualizer' \
         -s EXIT_RUNTIME=0 \
         -s ASSERTIONS=0 \
         --bind \
         --export-dynamic \
         -s EXPORTED_FUNCTIONS='[\"_main\",\"_update_mode\", \"_on_resize\",  \"_get_adj_json\", \"_get_root_node_json\", \"_get_node_list_json\", \"_set_root_node\", \"_toggle_keybind_overlay\", \"_reset_scene\", \"_reset_run_state\", \"_set_hover_state\", \"_save_camera_pos\", \"_set_camera_pos_to_old_pos\", \"_set_receive_inputs\", \"_set_algorithm\", \"_set_node_val\" , \"_script_stack_push\", \"_script_stack_pop\", \"_script_stack_size\", \"_script_queue_enqueue\", \"_script_queue_dequeue\", \"_script_queue_size\", \"_mark_visited\", \"_mark_discovered\", \"_set_active_node\", \"_add_node\", \"_add_edge\", \"_remove_node\", \"_remove_edge\", \"_set_algo_state\", \"_get_script_stack_json\", \"_get_script_queue_json\"]' \
         -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\", \"UTF8ToString\"]' \
         --preload-file ${CMAKE_SOURCE_DIR}/resources@/resources \
         --use-preload-plugins"
        )

         # --emit-tsd ${CMAKE_BINARY_DIR}/wasmmodule.d.ts"
endif()

# -------------------------------------------------------
# DEBUG / RELEASE FLAGS  (no LTO anywhere)
# -------------------------------------------------------
if(MSVC)
    add_compile_options(/arch:AVX2)
else()
    add_compile_options(
        -Wall -Wextra
        -Wno-missing-field-initializers
        -Wno-gnu-array-member-paren-init
    )

    if(EMSCRIPTEN)
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_compile_options(-O3 -ftree-vectorize)
        else()
            add_compile_options(-g)
        endif()
    else()
        add_compile_options(-mavx2)
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_compile_options(-O3 -ftree-vectorize -march=native)
        else()
            add_compile_options(-g)
        endif()
    endif()
endif()

# -------------------------------------------------------
# SOURCES
# -------------------------------------------------------
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*")
if(NOT EMSCRIPTEN)
    list(FILTER MY_SOURCES EXCLUDE REGEX "src/web\\.c$")
endif()

# -------------------------------------------------------
# EXECUTABLE
# -------------------------------------------------------
add_executable(${PROJECT_NAME} ${MY_SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

# -------------------------------------------------------
# INCLUDES & LINKS
# -------------------------------------------------------
target_include_directories(${PROJECT_NAME} PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/web/src"
)

if(EMSCRIPTEN)
    target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
        "${EMSCRIPTEN_ROOT_PATH}/cache/sysroot/include"
    )
endif()


target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# -------------------------------------------------------
# DEFINES
# -------------------------------------------------------
if(EMSCRIPTEN)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        PLATFORM_WEB=1
        RESOURCES_PATH="/resources/"
    )
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_definitions(${PROJECT_NAME} PRIVATE
            RESOURCES_PATH="./resources/"
            PRODUCTION_BUILD=1
        )
    else()
        target_compile_definitions(${PROJECT_NAME} PRIVATE
            RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/"
            PRODUCTION_BUILD=0
        )
    endif()
endif()

# -------------------------------------------------------
# PLATFORM-SPECIFIC TARGET PROPERTIES & POST-BUILD
# -------------------------------------------------------
if(EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/src/wasm"
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "${EM_LINK_FLAGS}"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/src/wasm/${PROJECT_NAME}.wasm"
            "${CMAKE_SOURCE_DIR}/public/wasm/${PROJECT_NAME}.wasm"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/src/wasm/${PROJECT_NAME}.data"
            "${CMAKE_SOURCE_DIR}/public/wasm/${PROJECT_NAME}.data"
        # COMMAND ${CMAKE_COMMAND} -E copy_if_different
        #     "${CMAKE_BINARY_DIR}/wasmmodule.d.ts"
        #     "${CMAKE_SOURCE_DIR}/src/types/wasmmodule.d.ts"
        COMMENT "Copying WASM artefacts → public/wasm/ and src/types/"
    )
else()
    if(MSVC)
        target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/resources"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
        COMMENT "Copying resources/ → build output dir"
    )
endif()
