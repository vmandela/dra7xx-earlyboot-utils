

add_library(utils utils.c)
target_include_directories(utils
        PUBLIC ${CMAKE_SOURCE_DIR}/inc)

add_library(rsc_utils rsc_utils.c
                      rsc_print_handler.c
                      rsc_vring_checker.c)
target_include_directories(rsc_utils
        PUBLIC ${CMAKE_SOURCE_DIR}/inc)

add_library(ti_fdt_utils ti_fdt_utils.c)
target_include_directories(ti_fdt_utils
        PUBLIC ${CMAKE_SOURCE_DIR}/inc)
target_link_libraries(ti_fdt_utils fdt)

add_executable (dump_rsc_table dump_rsc_table.c)
target_link_libraries(dump_rsc_table utils rsc_utils)

add_executable (check_late_props check_late_props.c)
target_link_libraries(check_late_props utils fdt)

add_executable (print_macros_for_mlo print_macros_for_mlo.c)
target_link_libraries(print_macros_for_mlo utils fdt ti_fdt_utils)

add_executable (check_rsc_table_dtb check_rsc_table_dtb.c)
target_link_libraries(check_rsc_table_dtb utils
                                          ti_fdt_utils
                                          rsc_utils)

install(TARGETS dump_rsc_table
                check_late_props
                print_macros_for_mlo
                check_rsc_table_dtb
                DESTINATION bin)
