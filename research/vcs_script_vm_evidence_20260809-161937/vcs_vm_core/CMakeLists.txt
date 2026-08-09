cmake_minimum_required(VERSION 3.20)

project(vcs_vm_core LANGUAGES CXX)

add_library(vcs_vm_core
    src/ScriptVmCore.cpp
)

target_include_directories(vcs_vm_core PUBLIC include)
target_compile_features(vcs_vm_core PUBLIC cxx_std_17)
target_compile_options(vcs_vm_core PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /EHsc>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

add_library(vcs_mission_host
    src/VcsDiscFs.cpp
    src/VcsMissionHost.cpp
)
target_include_directories(vcs_mission_host PUBLIC include)
target_compile_features(vcs_mission_host PUBLIC cxx_std_17)
target_link_libraries(vcs_mission_host PUBLIC vcs_vm_core)
target_compile_options(vcs_mission_host PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /EHsc>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

enable_testing()

add_executable(vcs_vm_core_tests tests/ScriptVmCoreTests.cpp)
target_link_libraries(vcs_vm_core_tests PRIVATE vcs_vm_core)
target_compile_options(vcs_vm_core_tests PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)
add_test(NAME vcs_vm_core_tests COMMAND vcs_vm_core_tests)

add_executable(vcs_mission_host_tests tests/VcsMissionHostTests.cpp)
target_link_libraries(vcs_mission_host_tests PRIVATE vcs_mission_host)
target_compile_options(vcs_mission_host_tests PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)
add_test(NAME vcs_mission_host_tests COMMAND vcs_mission_host_tests)

# A read-only proof harness for the first two known opcodes at the real VCS
# MAIN.SCM entrypoint.  It intentionally owns only recording adapters: it
# never starts a game, patches an image, or writes the supplied SCM file.
add_executable(vcs_vm_main_scm_prefix_harness tools/MainScmPrefixHarness.cpp)
target_link_libraries(vcs_vm_main_scm_prefix_harness PRIVATE vcs_vm_core)
target_compile_options(vcs_vm_main_scm_prefix_harness PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)

# This read-only harness stages one exact offset-table-selected mission slice
# and demonstrates the native 0289 ordering around the core's intentionally
# explicit host boundary. It never patches or writes the supplied SCM file.
add_executable(vcs_vm_mission_stream_harness tools/MissionStreamHarness.cpp)
target_link_libraries(vcs_vm_mission_stream_harness PRIVATE vcs_vm_core)
target_compile_options(vcs_vm_mission_stream_harness PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)

# Read-only vertical-slice executable: extracted PS2 tree or ISO9660 image ->
# MAIN.SCM -> native-shaped 0289 host -> child ScriptThread -> first explicit
# host fault. It ships no game data and never writes its input source.
add_executable(gta-vcs-bootstrap tools/VcsBootstrap.cpp)
target_link_libraries(gta-vcs-bootstrap PRIVATE vcs_mission_host)
target_compile_options(gta-vcs-bootstrap PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)
