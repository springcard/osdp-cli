cmake_minimum_required(VERSION 3.16)

project(osdp-cli LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

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

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c")
list(FILTER SOURCES EXCLUDE REGEX "src/platform/serial_(posix|win32)\\.c")

if(WIN32)
    list(APPEND SOURCES src/platform/serial_win32.c)
else()
    list(APPEND SOURCES src/platform/serial_posix.c)
endif()

add_executable(osdp-cli ${SOURCES})

if(WIN32)
    target_compile_definitions(osdp-cli PRIVATE WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
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
        src/core/commissioning_control.c src/util/utils.c)
    target_include_directories(test-commissioning PRIVATE src)
    add_test(NAME commissioning-regressions COMMAND test-commissioning)
    add_executable(test-redaction tests/test_redaction.c src/renderer/render_redaction.c
        src/renderer/render_frame.c src/renderer/render_hexdump.c src/renderer/render_packet.c
        src/renderer/render_json.c src/renderer/render_message.c src/renderer/render_message_json.c
        src/renderer/osdp_message_printer.c src/renderer/osdp_message_printer_json.c src/util/utils.c)
    target_include_directories(test-redaction PRIVATE src)
    add_test(NAME diagnostic-redaction COMMAND test-redaction)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME machine-process-contract COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_machine_contract.py
            $<TARGET_FILE:test-commissioning> $<TARGET_FILE:osdp-cli>)
        if(UNIX)
            add_test(NAME linux-serial-integration COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_linux_serial.py $<TARGET_FILE:osdp-cli>)
            set_tests_properties(linux-serial-integration PROPERTIES TIMEOUT 45)
        endif()
    endif()
    set_target_properties(test-sckmgmt test-protocol test-random test-commissioning test-redaction PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${config}" config_upper)
        set_target_properties(test-sckmgmt test-protocol test-random test-commissioning test-redaction PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_BINARY_DIR}/tests")
    endforeach()
endif()
