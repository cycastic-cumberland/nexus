cmake_minimum_required(VERSION 3.25)
project(nexus)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DDEBUG_ENABLED)

add_executable(nexus main.cpp core/types/safe_refcount.h core/typedefs.h core/types/reference.h core/types/cow.h core/types/lock.h core/language/lexer.h core/language/lexer.cpp core/types/vstring.h core/types/vstring.cpp core/memf.h core/types/vector.h core/types/queue.h core/types/stack.h core/hashfuncs.h core/comparator.h core/mathlib.h core/types/hashmap.h core/types/LinkedList.h)
