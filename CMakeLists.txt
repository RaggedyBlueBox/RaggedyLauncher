cmake_minimum_required(VERSION 3.20)
project(raggedy_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(raggedy_core
    src/app/launcher.cpp
    src/core/target.cpp
    src/core/launch_plan.cpp
    src/core/version.cpp
    src/core/profile.cpp
    src/core/json.cpp
    src/core/paths.cpp
    src/catalog/catalog.cpp
    src/catalog/built_in.cpp
    src/auth/device_code.cpp
    src/download/queue.cpp
    src/runtime/launch_plan_builder.cpp
    src/runtime/session.cpp
    src/mods/mod_plan.cpp
    src/mods/safety.cpp
    src/mods/mod_source.cpp
    src/mods/mod_pack.cpp
    src/performance/options.cpp
    src/controller/navigation.cpp
    src/graphics/translation.cpp
    src/graphics/layer.cpp
    src/storage/settings.cpp
    src/storage/profile_store.cpp
    src/ui/device_code_view.cpp
    src/ui/screen_models.cpp
    src/diagnostics/report.cpp
    src/remote/session.cpp
)

target_include_directories(raggedy_core PUBLIC include)

add_executable(test_core tests/core/test_core.cpp)
target_link_libraries(test_core PRIVATE raggedy_core)

add_executable(test_rewrite tests/core/test_rewrite.cpp)
target_link_libraries(test_rewrite PRIVATE raggedy_core)

add_executable(test_full tests/core/test_full.cpp)
target_link_libraries(test_full PRIVATE raggedy_core)

enable_testing()
add_test(NAME core COMMAND test_core)
add_test(NAME rewrite COMMAND test_rewrite)
add_test(NAME full COMMAND test_full)
