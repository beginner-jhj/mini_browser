cmake_minimum_required(VERSION 3.15)
project(MINI_BROWSER LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
set(CMAKE_AUTOMOC ON) # Qt meta-object compiler


add_executable(browser 
src/main.cpp)

# Library: parsing logic
add_library(parsing_lib
    src/parsing/html_tokenizer.cpp
    src/parsing/html_parser.cpp
    src/parsing/node.cpp
)

target_include_directories(parsing_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(rendering_lib
    src/rendering/browser_widget.cpp
)

target_include_directories(rendering_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rendering_lib PUBLIC 
    parsing_lib
    Qt6::Widgets
)

target_link_libraries(browser PRIVATE parsing_lib Qt6::Widgets)

# Enable testing
enable_testing()
add_subdirectory(test)