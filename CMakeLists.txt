cmake_minimum_required(VERSION 3.24.0)
project(universe LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(binary_dir "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${binary_dir})


set(PLATFORM_EXECUTABLE ${PROJECT_NAME})
set(GAME_DLL game_${PROJECT_NAME})

# app exe
set(SOURCES 
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
)
message(STATUS "PLATFORM EXE SOURCE FILES:")
foreach (element ${SOURCES})
    message(STATUS "\t ${element}")
endforeach ()

add_executable(${PLATFORM_EXECUTABLE} ${SOURCES})
target_include_directories(${PLATFORM_EXECUTABLE} PRIVATE "${CMAKE_SOURCE_DIR}/src")

# game dll
set(GAME_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/game.cpp"
)
message(STATUS "GAME DLL SOURCE FILES:")
foreach (element ${GAME_SOURCES})
    message(STATUS "\t ${element}")
endforeach ()
add_library(${GAME_DLL} SHARED ${GAME_SOURCES})

# defines
add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_MBCS)

# flags
string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
target_compile_features(${PLATFORM_EXECUTABLE} PRIVATE cxx_std_20)
target_compile_features(${GAME_DLL} PRIVATE cxx_std_20)
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

    target_compile_options(${PLATFORM_EXECUTABLE} PRIVATE ${COMMON_COMPILER_FLAGS})
    target_compile_options(${GAME_DLL} PRIVATE ${COMMON_COMPILER_FLAGS})
endif()

# dependencies
function (use_local_dependency library)
    add_subdirectory(third_party/${library})
    target_link_libraries(${PLATFORM_EXECUTABLE} PRIVATE ${library})    
    target_link_libraries(${GAME_DLL} PRIVATE ${library})
endfunction()

function (use_shared_lib lib)
    set(lib_PATH "third_party/${lib}")
    file(GLOB lib_SOURCES ${lib_PATH}/*.cpp)

    add_library(${lib} SHARED ${lib_SOURCES})

    include_directories(PUBLIC ${lib_PATH})

    target_link_libraries(${PLATFORM_EXECUTABLE} PUBLIC ${lib})
    target_link_libraries(${GAME_DLL} PUBLIC ${lib})

endfunction()

set(BUILD_SHARED_LIBS ON)
use_shared_lib(imgui)
set(BUILD_SHARED_LIBS OFF)

use_local_dependency(glm)

target_link_libraries(${GAME_DLL} PUBLIC d3d11)
target_link_libraries(${GAME_DLL} PUBLIC d3dcompiler)

# post-build events 
add_custom_command(TARGET ${PLATFORM_EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)

add_custom_command(TARGET ${PLATFORM_EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to source directory"
)
