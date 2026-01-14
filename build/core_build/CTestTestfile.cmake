# CMake generated Testfile for 
# Source directory: /home/unhidra/projects/sentinel-core
# Build directory: /home/unhidra/projects/sentinel-demo/build/core_build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(determinism "/home/unhidra/projects/sentinel-demo/build/core_build/test_determinism")
set_tests_properties(determinism PROPERTIES  _BACKTRACE_TRIPLES "/home/unhidra/projects/sentinel-core/CMakeLists.txt;45;add_test;/home/unhidra/projects/sentinel-core/CMakeLists.txt;0;")
add_test(rewind "/home/unhidra/projects/sentinel-demo/build/core_build/test_rewind")
set_tests_properties(rewind PROPERTIES  _BACKTRACE_TRIPLES "/home/unhidra/projects/sentinel-core/CMakeLists.txt;49;add_test;/home/unhidra/projects/sentinel-core/CMakeLists.txt;0;")
add_test(input_replay "/home/unhidra/projects/sentinel-demo/build/core_build/test_input_replay")
set_tests_properties(input_replay PROPERTIES  _BACKTRACE_TRIPLES "/home/unhidra/projects/sentinel-core/CMakeLists.txt;53;add_test;/home/unhidra/projects/sentinel-core/CMakeLists.txt;0;")
add_test(input_file_replay "/home/unhidra/projects/sentinel-demo/build/core_build/test_input_file_replay")
set_tests_properties(input_file_replay PROPERTIES  _BACKTRACE_TRIPLES "/home/unhidra/projects/sentinel-core/CMakeLists.txt;57;add_test;/home/unhidra/projects/sentinel-core/CMakeLists.txt;0;")
add_test(rollback_network "/home/unhidra/projects/sentinel-demo/build/core_build/test_rollback_network")
set_tests_properties(rollback_network PROPERTIES  _BACKTRACE_TRIPLES "/home/unhidra/projects/sentinel-core/CMakeLists.txt;61;add_test;/home/unhidra/projects/sentinel-core/CMakeLists.txt;0;")
add_test(invariant_violation "/home/unhidra/projects/sentinel-demo/build/core_build/test_invariant_violation")
set_tests_properties(invariant_violation PROPERTIES  _BACKTRACE_TRIPLES "/home/unhidra/projects/sentinel-core/CMakeLists.txt;65;add_test;/home/unhidra/projects/sentinel-core/CMakeLists.txt;0;")
