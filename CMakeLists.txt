cmake_minimum_required(VERSION 3.16)

project(osdp-cli LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# The Microsoft CRT deprecates standard C functions such as tmpfile().  Keep
# the portable C API and silence that vendor-specific diagnostic on Windows.
if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

set(APP_VERSION "1.0.0")
string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    if(WIN32)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/windows)
    else()
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/linux)
    endif()
endif()

if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${config}" config_upper)
        foreach(kind RUNTIME LIBRARY ARCHIVE)
            if(CMAKE_${kind}_OUTPUT_DIRECTORY AND NOT CMAKE_${kind}_OUTPUT_DIRECTORY_${config_upper})
                set(CMAKE_${kind}_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_${kind}_OUTPUT_DIRECTORY}")
            endif()
        endforeach()
    endforeach()
endif()

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Discover sources at configure time. CMake's CONFIGURE_DEPENDS verification
# currently hangs with the Windows Ninja toolchain used by this project.
file(GLOB_RECURSE SOURCES "src/*.c")
list(FILTER SOURCES EXCLUDE REGEX "src/platform/serial_(posix|win32)\\.c")

if(WIN32)
    list(APPEND SOURCES src/platform/serial_win32.c)
else()
    list(APPEND SOURCES src/platform/serial_posix.c)
endif()

add_executable(osdp-cli ${SOURCES})

if(WIN32)
    target_compile_definitions(osdp-cli PRIVATE WIN32_LEAN_AND_MEAN)
    target_link_libraries(osdp-cli PRIVATE bcrypt)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(osdp-cli PRIVATE Threads::Threads)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(test-sckmgmt tests/test_sckmgmt.c src/commands/sckmgmt/sckmgmt_run.c
        src/commands/sckmgmt/sckmgmt_options_parser.c src/cli/parser_engine.c src/util/utils.c)
    target_include_directories(test-sckmgmt PRIVATE src)
    add_test(NAME sckmgmt-regressions COMMAND test-sckmgmt)
    add_executable(test-protocol tests/test_protocol.c src/moduleOSDP/osdp_cp.c
        src/moduleOSDP/osdp_cmd.c src/moduleOSDP/osdp_packet.c src/moduleOSDP/osdp_sc.c
        src/moduleOSDP/osdp_util.c src/moduleOSDP/aes.c src/core/osdp_event.c
        src/core/commissioning_control.c src/util/utils.c)
    target_include_directories(test-protocol PRIVATE src)
    add_test(NAME protocol-regressions COMMAND test-protocol)
    add_executable(test-random tests/test_random.c src/platform/random.c src/util/utils.c)
    target_include_directories(test-random PRIVATE src)
    if(WIN32)
        target_link_libraries(test-random PRIVATE bcrypt)
    endif()
    add_test(NAME operating-system-random COMMAND test-random)
    add_executable(test-commissioning tests/test_commissioning.c
        src/commands/commissioning/commissioning.c src/commands/probe/probe_run.c
        src/commands/probe/probe_options_parser.c src/cli/parser_engine.c
        src/core/commissioning_control.c src/util/utils.c)
    target_include_directories(test-commissioning PRIVATE src)
    add_test(NAME commissioning-regressions COMMAND test-commissioning)
    add_executable(test-redaction tests/test_redaction.c src/renderer/render_redaction.c
        src/renderer/render_frame.c src/renderer/render_hexdump.c src/renderer/render_packet.c
        src/renderer/render_json.c src/renderer/render_message.c src/renderer/render_message_json.c
        src/renderer/osdp_message_printer.c src/renderer/osdp_message_printer_json.c src/util/utils.c)
    target_include_directories(test-redaction PRIVATE src)
    add_test(NAME diagnostic-redaction COMMAND test-redaction)
    add_executable(test-list tests/test_list.c src/commands/list/list_run.c
        src/commands/list/list_options_parser.c src/cli/parser_engine.c)
    target_include_directories(test-list PRIVATE src)
    add_test(NAME list-discovery COMMAND test-list)
    add_executable(test-istat tests/test_istat.c src/commands/istat/istat_run.c
        src/commands/istat/istat_options_parser.c src/cli/parser_engine.c
        src/core/commissioning_control.c src/util/utils.c)
    target_include_directories(test-istat PRIVATE src)
    add_test(NAME istat-regressions COMMAND test-istat)
    add_executable(test-output tests/test_output.c src/commands/output/output_run.c
        src/commands/output/output_options_parser.c src/cli/parser_engine.c
        src/core/commissioning_control.c src/util/utils.c)
    target_include_directories(test-output PRIVATE src)
    add_test(NAME output-regressions COMMAND test-output)
    add_executable(test-put-parser tests/test_put_parser.c src/commands/file/put/put_options_parser.c
        src/commands/file/put/put_subcommand.c src/cli/parser_engine.c src/util/utils.c)
    target_include_directories(test-put-parser PRIVATE src)
    add_test(NAME file-put-parser COMMAND test-put-parser)
    add_executable(test-logging-defaults tests/test_logging_defaults.c
        src/logger/logger_console.c src/logger/logger_file.c)
    target_include_directories(test-logging-defaults PRIVATE src)
    add_test(NAME logging-defaults COMMAND test-logging-defaults)
    if(UNIX)
        add_executable(test-serial-tx-control tests/test_serial_tx_control.c
            src/platform/serial_posix.c src/platform/serial_tx_control.c src/core/commissioning_control.c src/util/utils.c)
        target_include_directories(test-serial-tx-control PRIVATE src)
        target_link_options(test-serial-tx-control PRIVATE "-Wl,--wrap=open" "-Wl,--wrap=write" "-Wl,--wrap=close" "-Wl,--wrap=lseek" "-Wl,--wrap=access" "-Wl,--wrap=ioctl" "-Wl,--wrap=tcflush" "-Wl,--wrap=poll")
        add_test(NAME serial-tx-control COMMAND test-serial-tx-control)
        add_executable(test-serial-zero-read tests/test_serial_zero_read.c
            src/platform/serial_posix.c src/platform/serial_tx_control.c src/util/utils.c)
        target_include_directories(test-serial-zero-read PRIVATE src)
        target_link_options(test-serial-zero-read PRIVATE "-Wl,--wrap=read" "-Wl,--wrap=poll")
        add_test(NAME serial-zero-read COMMAND test-serial-zero-read)
    endif()
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME output-process-contract COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_output_process.py
            $<TARGET_FILE:test-output> $<TARGET_FILE:osdp-cli>)
        add_test(NAME istat-process-contract COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_istat_process.py
            $<TARGET_FILE:test-istat> $<TARGET_FILE:osdp-cli>)
        add_test(NAME machine-process-contract COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_machine_contract.py
            $<TARGET_FILE:test-commissioning> $<TARGET_FILE:osdp-cli>)
        if(UNIX)
            add_test(NAME linux-serial-integration COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_linux_serial.py $<TARGET_FILE:osdp-cli>)
            set_tests_properties(linux-serial-integration PROPERTIES TIMEOUT 45)
        endif()
    endif()
    set_target_properties(test-sckmgmt test-protocol test-random test-commissioning test-redaction test-list test-istat test-output test-put-parser PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${config}" config_upper)
        set_target_properties(test-sckmgmt test-protocol test-random test-commissioning test-redaction test-list test-istat test-output test-put-parser PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_BINARY_DIR}/tests")
    endforeach()
endif()

if(BUILD_TESTING)
    add_executable(test-command-results tests/test_command_results.c
        src/commands/config/config_run.c src/commands/info/info_run.c src/commands/cp/cp_run.c
        src/commands/cmd/buzzer/buzzer_run.c src/commands/cmd/led/led_run.c
        src/commands/cmd/relay/relay_run.c src/commands/cmd/text/text_run.c
        src/commands/cmd/raw/raw_run.c src/commands/file/put/put_run.c)
    target_include_directories(test-command-results PRIVATE src)
    if(Python3_Interpreter_FOUND)
        add_test(NAME command-final-results COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_command_results.py
            $<TARGET_FILE:test-command-results> $<TARGET_FILE:osdp-cli> $<TARGET_FILE:test-protocol>)
    endif()
endif()

# Test executables exercise command/parser/protocol sources with the shared diagnostics.
if(BUILD_TESTING)
    get_property(project_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS project_targets)
        if(target MATCHES "^test-")
            target_sources(${target} PRIVATE src/cli/command_result.c)
        endif()
    endforeach()
endif()
