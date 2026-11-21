cmake_minimum_required(VERSION 3.20)

# Hot Reload for MSVC (kept from the VS template)
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# ===== SINGLE SOURCE OF TRUTH: change the language identity here only =====
set(BYTELET_LANG_NAME "Bytelet")
set(BYTELET_FILE_EXT  ".byt")
set(BYTELET_CLI_NAME  "bytelet")   # executable / command name
# =========================================================================

project(${BYTELET_LANG_NAME} VERSION 0.1.0 LANGUAGES CXX)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR
        "Bytelet requires a 64-bit build (got ${CMAKE_SIZEOF_VOID_P}-byte pointers). "
        "Open an x64 developer shell, or use the x64 preset in Visual Studio.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate BuildInfo.h from the variables above.
configure_file(src/BuildInfo.h.in generated/BuildInfo.h @ONLY)

add_executable(${BYTELET_CLI_NAME} src/main.cpp)
target_include_directories(${BYTELET_CLI_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

if (MSVC)
    target_compile_options(${BYTELET_CLI_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${BYTELET_CLI_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- Tests (CTest) ----
enable_testing()

add_test(NAME version_flag COMMAND ${BYTELET_CLI_NAME} --version)
set_tests_properties(version_flag PROPERTIES
    PASS_REGULAR_EXPRESSION "${BYTELET_CLI_NAME} ${PROJECT_VERSION}")

add_test(NAME help_flag COMMAND ${BYTELET_CLI_NAME} --help)
set_tests_properties(help_flag PROPERTIES PASS_REGULAR_EXPRESSION "Usage:")

add_test(NAME unknown_option_fails COMMAND ${BYTELET_CLI_NAME} --bogus)
set_tests_properties(unknown_option_fails PROPERTIES WILL_FAIL TRUE)

add_test(NAME no_args_fails COMMAND ${BYTELET_CLI_NAME})
set_tests_properties(no_args_fails PROPERTIES WILL_FAIL TRUE)