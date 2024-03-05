# CMake generated Testfile for 
# Source directory: /Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests
# Build directory: /Users/thao_admin/Documents/Cancer_simuscale/simuscale/build/src/tests/integration_tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(int_test_small "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/simple_test.sh" "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/build/bin" "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/ODE_stem_niche_small")
set_tests_properties(int_test_small PROPERTIES  _BACKTRACE_TRIPLES "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/CMakeLists.txt;2;add_test;/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/CMakeLists.txt;0;")
add_test(int_test_reprod "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/reprod_test.sh" "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/build/bin" "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/ODE_stem_niche_reprod")
set_tests_properties(int_test_reprod PROPERTIES  _BACKTRACE_TRIPLES "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/CMakeLists.txt;7;add_test;/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/CMakeLists.txt;0;")
add_test(int_test_large "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/simple_test.sh" "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/build/bin" "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/ODE_stem_niche")
set_tests_properties(int_test_large PROPERTIES  _BACKTRACE_TRIPLES "/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/CMakeLists.txt;12;add_test;/Users/thao_admin/Documents/Cancer_simuscale/simuscale/src/tests/integration_tests/CMakeLists.txt;0;")
