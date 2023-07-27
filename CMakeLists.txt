project(fine)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-variable")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")

set(CMAKE_CURRENT_BINARY_DIR "${CMAKE_CURRENT_LIST_DIR}/build")
set(GENERATED_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/src")
set(GENERATED_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")

set(SCRIPTS_DIR "${CMAKE_CURRENT_LIST_DIR}/scripts")

add_custom_command(
	PRE_BUILD
	COMMAND mkdir -p "${GENERATED_SOURCE_DIR}"
	OUTPUT "${GENERATED_SOURCE_DIR}"
)
add_custom_command(
	PRE_BUILD
	COMMAND mkdir -p "${GENERATED_HEADERS_DIR}"
	OUTPUT "${GENERATED_HEADERS_DIR}"
)

# dirs for generated gperf files
set(GPERF_INPUT_DIR "${CMAKE_CURRENT_LIST_DIR}/gperf")
set(GPERF_SOURCE_DIR "${GENERATED_SOURCE_DIR}/gperf")
set(GPERF_HEADERS_DIR "${GENERATED_HEADERS_DIR}/gperf")
execute_process(COMMAND sh -c "echo '${GPERF_HEADERS_DIR}' | sed 's|${GENERATED_HEADERS_DIR}/||'" OUTPUT_VARIABLE gperf_include_base_path OUTPUT_STRIP_TRAILING_WHITESPACE)
add_custom_command(
	PRE_BUILD
	COMMAND mkdir -p "${GPERF_SOURCE_DIR}"
	OUTPUT "${GPERF_SOURCE_DIR}"
)
add_custom_command(
	PRE_BUILD
	COMMAND mkdir -p "${GPERF_HEADERS_DIR}"
	OUTPUT "${GPERF_HEADERS_DIR}"
)

# generate gperf files
file (GLOB GPERF_INPUT
	"${GPERF_INPUT_DIR}/*.gperf"
)
foreach(gperf_input_file ${GPERF_INPUT})
	execute_process(COMMAND sh -c "echo ${gperf_input_file} | sed 's|${GPERF_INPUT_DIR}/\\(.*\\)\\.gperf|${GPERF_SOURCE_DIR}/\\1.c|'" OUTPUT_VARIABLE gperf_output_source OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND sh -c "echo ${gperf_input_file} | sed 's|${GPERF_INPUT_DIR}/\\(.*\\)\\.gperf|${GPERF_HEADERS_DIR}/\\1.h|'" OUTPUT_VARIABLE gperf_output_header OUTPUT_STRIP_TRAILING_WHITESPACE)
	add_custom_command(
		COMMAND ${SCRIPTS_DIR}/gperf.sh --readonly-tables --strncmp-no-null "${gperf_input_file}" --struct-type --output-source "${gperf_output_source}" --output-header "${gperf_output_header}"
		DEPENDS "${gperf_input_file}" "${GPERF_SOURCE_DIR}" "${GPERF_HEADERS_DIR}"
		OUTPUT "${gperf_output_source}" "${gperf_output_header}"
	)
	list(APPEND GPERF_OUTPUT "${gperf_output_source}" "${gperf_output_header}")
endforeach()
add_library(gperf OBJECT ${GPERF_OUTPUT})
set_target_properties(gperf PROPERTIES LINKER_LANGUAGE C)
target_include_directories(gperf PUBLIC "${GPERF_HEADERS_DIR}/" "${CMAKE_CURRENT_LIST_DIR}/include")

file(GLOB SRC
	"${CMAKE_CURRENT_LIST_DIR}/src/*.c"
	"${CMAKE_CURRENT_LIST_DIR}/include/*.h"
	"${GENERATED_HEADERS_DIR}/*.h"
)
add_executable(fine ${SRC})

target_include_directories(fine PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/include/"
	"${GENERATED_HEADERS_DIR}/"
)
target_link_libraries(fine gperf)
