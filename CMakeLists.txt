cmake_minimum_required(VERSION 3.24.0)
project(universe LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/")

include_directories("src")

# app exe
set(SOURCES "${CMAKE_SOURCE_DIR}/src/app/main.cpp" "${CMAKE_SOURCE_DIR}/src/app/platform_win32.cpp")
foreach (element ${SOURCES})
    message(STATUS "including source file to build: ${element}")
endforeach ()
add_executable(${PROJECT_NAME} ${SOURCES})

# game dll
set(GAME_SOURCES "${CMAKE_SOURCE_DIR}/src/game/game.cpp")
foreach (element ${GAME_SOURCES})
    message(STATUS "including game source file to build: ${element}")
endforeach ()
add_library(game_${PROJECT_NAME} SHARED ${GAME_SOURCES})

# defines
add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

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

        # Release config
        $<$<CONFIG:Release>:
            /O2
        >)

    target_compile_options(${PROJECT_NAME} PRIVATE ${COMMON_COMPILER_FLAGS})
    target_compile_options(game_${PROJECT_NAME} PRIVATE ${COMMON_COMPILER_FLAGS})
endif()

function (link_dependency package_name library)
    find_package(${package_name} CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${library})    
    target_link_libraries(game_${PROJECT_NAME} PRIVATE ${library})    
endfunction()

link_dependency(glm glm::glm)
link_dependency(raylib raylib)
link_dependency(imgui imgui::imgui)

#####################
# post-build events #
#####################
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)
