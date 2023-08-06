cmake_minimum_required(VERSION 3.22)
project(nexus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_definitions(-DDEBUG_ENABLED)
#add_definitions(-DBENCHMARKS_ENABLED)
add_definitions(-DTESTS_ENABLED)


# Set the path to the Google Test source directory
set(GTEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
set(GBM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/benchmark)

# Add the Google Test library
add_subdirectory(${GTEST_SOURCE_DIR})
add_subdirectory(${GBM_SOURCE_DIR})

# Set up include directories for Google Test
include_directories(${GTEST_SOURCE_DIR}/googletest/include)
include_directories(${GTEST_SOURCE_DIR}/googlemock/include)
include_directories(${GBM_SOURCE_DIR}/include)

add_executable(nexus core/types/safe_refcount.h core/typedefs.h core/types/reference.h core/lock.h language/lexer.h language/lexer.cpp core/types/vstring.h core/types/vstring.cpp core/memf.h core/types/queue.h core/types/stack.h core/hashfuncs.h core/comparator.h core/mathlib.h core/types/hashmap.h language/parser.h language/parser.cpp core/types/object.h core/types/object.cpp core/hashfuncs.cpp core/types/vector.h core/types/cow.h core/types/tuple.h language/Bytecode.h language/Bytecode.cpp core/io/file_access.h core/io/file_access.cpp core/types/char_string.h core/types/char_string.cpp core/exception.h core/io/virtual_file_access.h core/io/x86_file_access.h core/io/x86_file_access.cpp core/io/file_access_server.h core/output.h core/types/linked_list.h runtime/system.h runtime/windows_system.h runtime/windows_system.cpp runtime/runtime.h runtime/runtime.cpp runtime/runtime_global_settings.h runtime/runtime_global_settings.cpp tests/test_stack.cpp main.cpp runtime/task.h runtime/ambigous_value.h runtime/task.cpp core/types/stack_struct.h core/types/stack_struct.cpp runtime/ambigous_value.cpp runtime/task_scheduler.h core/exception.cpp runtime/task_scheduler.cpp runtime/config.h runtime/system.cpp runtime/nexus_output.h core/output.cpp core/types/priority_queue.h core/types/priority_queue.cpp runtime/managed_thread.h runtime/managed_thread.cpp runtime/native_bindings/builtin.h core/types/interned_string.h core/types/interned_string.cpp tests/test_string_literal.cpp core/semaphore.h core/types/safe_priority_queue.h tests/test_thread_pool.cpp runtime/thread_pool.h runtime/command_queue.h core/types/hash_set.h runtime/nexus_stack.h runtime/nexus_stack.cpp tests/test_nexus_stack.cpp benchmarks/benchmark_multi_threaded_memcpy.cpp runtime/utils.h runtime/unix_system.h runtime/unix_system.cpp core/cmd_handler.h core/cmd_handler.cpp)
target_link_libraries(nexus gtest gtest_main)
target_link_libraries(nexus benchmark::benchmark)

