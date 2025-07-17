set(TEST_DRIVER ${CMAKE_CURRENT_SOURCE_DIR}/run_and_compare.sh)
set(ROBOT_EXE   $<TARGET_FILE:robot>)

# перечислите здесь все пары .in / .out
set(TEST_CASES
    01_vars
    02_if
    03_else
    04_loop
    05_arith
    06_logic
    07_cell
    08_array
    09_field
    10_dim
    11_type
    12_array
    13_maze
    14_func
)

foreach(case ${TEST_CASES})
    if("${case}" STREQUAL "13_maze")
        add_test(NAME ${case}
                 COMMAND ${TEST_DRIVER} ${ROBOT_EXE}
                         ${CMAKE_CURRENT_SOURCE_DIR}/${case}.in
                         ${CMAKE_CURRENT_SOURCE_DIR}/${case}.out
                         ${CMAKE_CURRENT_SOURCE_DIR}/maze.txt)
    else()
        add_test(NAME ${case}
                 COMMAND ${TEST_DRIVER} ${ROBOT_EXE}
                         ${CMAKE_CURRENT_SOURCE_DIR}/${case}.in
                         ${CMAKE_CURRENT_SOURCE_DIR}/${case}.out)
    endif()
endforeach()