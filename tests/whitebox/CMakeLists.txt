# ------------------------------------------------------------------------------
# include directories
# ------------------------------------------------------------------------------
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/ext
)
# ------------------------------------------------------------------------------
# flags
# ------------------------------------------------------------------------------
set(CMAKE_C_FLAGS "-std=c99")
set(CMAKE_CXX_FLAGS "-Wall -Weffc++ -Wcast-qual -Werror -std=c++11")
# ------------------------------------------------------------------------------
# macros
# ------------------------------------------------------------------------------
macro (add_ntrnt_wb_test test_name)
    SET(SRCS "wb_${test_name}.cc")
    add_executable("wb_${test_name}" ${SRCS})
	target_compile_definitions("wb_${test_name}" PUBLIC TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
    target_link_libraries("wb_${test_name}"
        ${DEBUG_LIBRARIES}
        ntrnt_compiled_catch
        ntrnt
        ${LIBRARIES}
    )
    add_test(${test_name} "wb_${test_name}")
endmacro()
# ------------------------------------------------------------------------------
# build catch
# ------------------------------------------------------------------------------
add_library(ntrnt_compiled_catch STATIC ${CMAKE_SOURCE_DIR}/ext/catch/catch.hpp _catch.cc)
# ------------------------------------------------------------------------------
# build tests
# ------------------------------------------------------------------------------
add_subdirectory(support)
