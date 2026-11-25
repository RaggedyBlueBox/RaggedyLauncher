cmake_minimum_required(VERSION 3.20)
project(raggedy_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(raggedy_core
    src/core/target.cpp
    src/core/launch_plan.cpp
    src/core/version.cpp
    src/core/profile.cpp
    src/catalog/catalog.cpp
    src/auth/device_code.cpp
    src/graphics/translation.cpp
    src/ui/device_code_view.cpp
)

target_include_directories(raggedy_core PUBLIC include)

add_executable(test_core tests/core/test_core.cpp)
target_link_libraries(test_core PRIVATE raggedy_core)

add_executable(test_rewrite tests/core/test_rewrite.cpp)
target_link_libraries(test_rewrite PRIVATE raggedy_core)

enable_testing()
add_test(NAME core COMMAND test_core)
add_test(NAME rewrite COMMAND test_rewrite)
