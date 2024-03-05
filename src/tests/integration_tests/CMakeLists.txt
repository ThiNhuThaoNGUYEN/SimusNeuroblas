
add_test(NAME int_test_small
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/simple_test.sh
                 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                 ${CMAKE_CURRENT_SOURCE_DIR}/ODE_stem_niche_small)

add_test(NAME int_test_reprod
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/reprod_test.sh
                 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                 ${CMAKE_CURRENT_SOURCE_DIR}/ODE_stem_niche_reprod)

add_test(NAME int_test_large
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/simple_test.sh
                 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                 ${CMAKE_CURRENT_SOURCE_DIR}/ODE_stem_niche)


#set_property(TEST int_test_small
#             PROPERTY FAIL_REGULAR_EXPRESSION "diff")
