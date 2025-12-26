add_executable(parser_test parser_test.cpp)

# Link with parsing library
target_link_libraries(parser_test PRIVATE parsing_lib)

# Add as CTest
add_test(NAME ParserTest COMMAND parser_test)

add_executable(renderer_test renderer_test.cpp)
target_link_libraries(renderer_test PRIVATE 
    rendering_lib
)