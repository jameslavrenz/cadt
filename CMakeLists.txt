cmake_minimum_required(VERSION 3.10)
project(cadt VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(cadt
    src/common.c
    src/span.c
    src/vector.c
    src/deque.c
    src/list.c
    src/forward_list.c
    src/array.c
    src/stack.c
    src/queue.c
    src/priority_queue.c
    src/rbtree.c
    src/set.c
    src/map.c
    src/hashtable.c
    src/unordered_set.c
    src/unordered_map.c
    src/string.c
    src/bitset.c
    src/optional.c
    src/algo.c
)
target_include_directories(cadt
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(cadt PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes)

enable_testing()
add_executable(cadt_tests
    tests/test_main.c
    tests/track_alloc.c
    tests/test_vector.c
    tests/test_deque.c
    tests/test_list.c
    tests/test_adapters.c
    tests/test_assoc.c
    tests/test_utility.c
)
target_include_directories(cadt_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(cadt_tests PRIVATE cadt)
target_compile_options(cadt_tests PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes)
add_test(NAME cadt_tests COMMAND cadt_tests)

add_executable(cadt_demo examples/demo.c)
target_link_libraries(cadt_demo PRIVATE cadt)
add_executable(cadt_word_count examples/word_count.c)
target_link_libraries(cadt_word_count PRIVATE cadt)

install(TARGETS cadt ARCHIVE DESTINATION lib)
install(FILES include/cadt.h DESTINATION include)
install(DIRECTORY include/cadt DESTINATION include)
