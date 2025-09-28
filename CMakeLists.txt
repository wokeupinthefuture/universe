cmake_minimum_required(VERSION 3.24.0)
project(universe LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(binary_dir "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${binary_dir})

include_directories("src")

# app exe
set(SOURCES 
    "${CMAKE_SOURCE_DIR}/src/app/main.cpp"
     "${CMAKE_SOURCE_DIR}/src/app/platform_win32.cpp"
     "${CMAKE_SOURCE_DIR}/src/app/renderer_dx11.cpp"
     )
message(STATUS "APP SOURCE FILES:")
foreach (element ${SOURCES})
    message(STATUS "\t ${element}")
endforeach ()

add_executable(${PROJECT_NAME} ${SOURCES})

# game dll
set(GAME_SOURCES "${CMAKE_SOURCE_DIR}/src/game/game.cpp")
message(STATUS "GAME SOURCE FILES:")
foreach (element ${GAME_SOURCES})
    message(STATUS "\t ${element}")
endforeach ()
add_library(game_${PROJECT_NAME} SHARED ${GAME_SOURCES})

# defines
add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_MBCS)

# flags
string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_features(game_${PROJECT_NAME} PRIVATE cxx_std_20)
if(MSVC)
    set(COMMON_COMPILER_FLAGS 
        /GR-
        /EHsc-
        /W4
        /Oi
        /wd4100

        $<$<CONFIG:Debug>:
            /Od
            /Zi
        >

        $<$<CONFIG:Release>:
            /O2
        >)

    target_compile_options(${PROJECT_NAME} PRIVATE ${COMMON_COMPILER_FLAGS})
    target_compile_options(game_${PROJECT_NAME} PRIVATE ${COMMON_COMPILER_FLAGS})
endif()

# dependencies
function (use_local_dependency library)
    add_subdirectory(third_party/${library})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${library})    
    target_link_libraries(game_${PROJECT_NAME} PRIVATE ${library})
endfunction()

function (use_shared_lib lib)
    set(lib_PATH "third_party/${lib}")
    file(GLOB lib_SOURCES ${lib_PATH}/*.cpp)

    add_library(${lib} SHARED ${lib_SOURCES})

    include_directories(PUBLIC ${lib_PATH})

    target_link_libraries(${PROJECT_NAME} PUBLIC ${lib})
    target_link_libraries(game_${PROJECT_NAME} PUBLIC ${lib})
endfunction()

set(BUILD_SHARED_LIBS ON)
use_shared_lib(imgui)
set(BUILD_SHARED_LIBS OFF)

use_local_dependency(glm)

target_link_libraries(${PROJECT_NAME} PUBLIC d3d11)
target_link_libraries(${PROJECT_NAME} PUBLIC d3dcompiler)

# post-build events 
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to source directory"
)
