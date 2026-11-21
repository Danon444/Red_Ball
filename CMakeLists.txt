cmake_minimum_required(VERSION 3.28)
project(RedBall LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(SFML)

FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Исправление для MinGW, если в пути к проекту есть пробелы.
if(MINGW)
    if(TARGET freetype)
        get_target_property(freetype_sources freetype SOURCES)
        list(FILTER freetype_sources EXCLUDE REGEX "\\.rc$")
        set_property(TARGET freetype PROPERTY SOURCES ${freetype_sources})
    endif()
endif()

set(REDBALL_SOURCES
    src/main.cpp
    src/Ball.cpp
    src/Game.cpp
    src/Level.cpp
    src/MovingPlatform.cpp
)

if(WIN32)
    add_executable(RedBall WIN32 ${REDBALL_SOURCES})
    target_link_libraries(RedBall PRIVATE SFML::Graphics SFML::Main)
else()
    add_executable(RedBall ${REDBALL_SOURCES})
    target_link_libraries(RedBall PRIVATE SFML::Graphics)
endif()
target_include_directories(RedBall PRIVATE include)
target_link_libraries(RedBall PRIVATE nlohmann_json::nlohmann_json)

add_custom_target(RedBallAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:RedBall>/assets"
)
add_dependencies(RedBall RedBallAssets)

# Enabling automatic tests
option(REDBALL_BUILD_CHECKS "Build the level traversal check" OFF)
if(REDBALL_BUILD_CHECKS)
    enable_testing()
    add_executable(LevelTraversalCheck 
        tests/LevelTraversal.cpp 
        src/Ball.cpp 
        src/Level.cpp
        src/MovingPlatform.cpp
    )
    target_include_directories(LevelTraversalCheck PRIVATE include)
    target_link_libraries(LevelTraversalCheck PRIVATE SFML::Graphics nlohmann_json::nlohmann_json)
    add_test(NAME LevelTraversal COMMAND LevelTraversalCheck)
    set_tests_properties(LevelTraversal PROPERTIES
        TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

