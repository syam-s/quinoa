# CMake generated Testfile for 
# Source directory: /home/aditya/quinoa/regression/walker/MassFracBeta
# Build directory: /home/aditya/quinoa/cmake/regression/walker/MassFracBeta
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(walker:MassFracBeta_pe1 "/usr/bin/cmake" "-DTEST_NAME=walker:MassFracBeta_pe1" "-DWORKDIR=/home/aditya/quinoa/cmake/regression/walker/MassFracBeta/walker:MassFracBeta_pe1" "-DRUNNER=/home/aditya/quinoa/cmake/charmrun" "-DRUNNER_NCPUS_ARG=+p" "-DRUNNER_ARGS=--bind-to none --map-by node" "-DTEST_EXECUTABLE=/home/aditya/quinoa/cmake/Main/walker" "-DTEST_EXECUTABLE_ARGS=-c massfracbeta.q -v" "-DTEST_LABELS=LABELS walker" "-DNUMPES=1" "-DTEXT_DIFF_PROG=/home/aditya/quinoa/tpl/install/gnu-x86_64/bin/maddiff" "-DTEXT_DIFF_PROG_ARGS=" "-DTEXT_DIFF_PROG_CONF=massfracbeta.ndiff.cfg" "-DTEXT_BASELINE=stat.txt.std" "-DTEXT_RESULT=stat.txt" "-DBIN_DIFF_PROG=/home/aditya/quinoa/tpl/install/gnu-x86_64/bin/exodiff" "-DBIN_DIFF_PROG_ARGS=" "-DBIN_DIFF_PROG_CONF=" "-DBIN_BASELINE=" "-DBIN_RESULT=" "-DPOSTPROCESS_PROG=" "-DPOSTPROCESS_PROG_ARGS=" "-DPOSTPROCESS_PROG_OUTPUT=" "-P" "/home/aditya/quinoa/src/../cmake/test_runner.cmake")
set_tests_properties(walker:MassFracBeta_pe1 PROPERTIES  FAIL_REGULAR_EXPRESSION "exodiff: ERROR;exodiff: Files are different" LABELS "walker" PASS_REGULAR_EXPRESSION ".*walker:MassFracBeta_pe1.*PASS;Failed statistics;All tests passed;exodiff: Files are the same;would be required for this test to be rigorous" PROCESSORS "1" WORKING_DIRECTORY "/home/aditya/quinoa/cmake/regression/walker/MassFracBeta/walker:MassFracBeta_pe1")
add_test(walker:MassFracBeta_pe2 "/usr/bin/cmake" "-DTEST_NAME=walker:MassFracBeta_pe2" "-DWORKDIR=/home/aditya/quinoa/cmake/regression/walker/MassFracBeta/walker:MassFracBeta_pe2" "-DRUNNER=/home/aditya/quinoa/cmake/charmrun" "-DRUNNER_NCPUS_ARG=+p" "-DRUNNER_ARGS=--bind-to none --map-by node" "-DTEST_EXECUTABLE=/home/aditya/quinoa/cmake/Main/walker" "-DTEST_EXECUTABLE_ARGS=-c massfracbeta.q -v" "-DTEST_LABELS=LABELS walker" "-DNUMPES=2" "-DTEXT_DIFF_PROG=/home/aditya/quinoa/tpl/install/gnu-x86_64/bin/maddiff" "-DTEXT_DIFF_PROG_ARGS=" "-DTEXT_DIFF_PROG_CONF=massfracbeta.ndiff.cfg" "-DTEXT_BASELINE=stat.txt.std" "-DTEXT_RESULT=stat.txt" "-DBIN_DIFF_PROG=/home/aditya/quinoa/tpl/install/gnu-x86_64/bin/exodiff" "-DBIN_DIFF_PROG_ARGS=" "-DBIN_DIFF_PROG_CONF=" "-DBIN_BASELINE=" "-DBIN_RESULT=" "-DPOSTPROCESS_PROG=" "-DPOSTPROCESS_PROG_ARGS=" "-DPOSTPROCESS_PROG_OUTPUT=" "-P" "/home/aditya/quinoa/src/../cmake/test_runner.cmake")
set_tests_properties(walker:MassFracBeta_pe2 PROPERTIES  FAIL_REGULAR_EXPRESSION "exodiff: ERROR;exodiff: Files are different" LABELS "walker" PASS_REGULAR_EXPRESSION ".*walker:MassFracBeta_pe2.*PASS;Failed statistics;All tests passed;exodiff: Files are the same;would be required for this test to be rigorous" PROCESSORS "2" WORKING_DIRECTORY "/home/aditya/quinoa/cmake/regression/walker/MassFracBeta/walker:MassFracBeta_pe2")
add_test(walker:MassFracBeta_pe4 "/usr/bin/cmake" "-DTEST_NAME=walker:MassFracBeta_pe4" "-DWORKDIR=/home/aditya/quinoa/cmake/regression/walker/MassFracBeta/walker:MassFracBeta_pe4" "-DRUNNER=/home/aditya/quinoa/cmake/charmrun" "-DRUNNER_NCPUS_ARG=+p" "-DRUNNER_ARGS=--bind-to none --map-by node" "-DTEST_EXECUTABLE=/home/aditya/quinoa/cmake/Main/walker" "-DTEST_EXECUTABLE_ARGS=-c massfracbeta.q -v" "-DTEST_LABELS=LABELS walker" "-DNUMPES=4" "-DTEXT_DIFF_PROG=/home/aditya/quinoa/tpl/install/gnu-x86_64/bin/maddiff" "-DTEXT_DIFF_PROG_ARGS=" "-DTEXT_DIFF_PROG_CONF=massfracbeta.ndiff.cfg" "-DTEXT_BASELINE=stat.txt.std" "-DTEXT_RESULT=stat.txt" "-DBIN_DIFF_PROG=/home/aditya/quinoa/tpl/install/gnu-x86_64/bin/exodiff" "-DBIN_DIFF_PROG_ARGS=" "-DBIN_DIFF_PROG_CONF=" "-DBIN_BASELINE=" "-DBIN_RESULT=" "-DPOSTPROCESS_PROG=" "-DPOSTPROCESS_PROG_ARGS=" "-DPOSTPROCESS_PROG_OUTPUT=" "-P" "/home/aditya/quinoa/src/../cmake/test_runner.cmake")
set_tests_properties(walker:MassFracBeta_pe4 PROPERTIES  FAIL_REGULAR_EXPRESSION "exodiff: ERROR;exodiff: Files are different" LABELS "walker" PASS_REGULAR_EXPRESSION ".*walker:MassFracBeta_pe4.*PASS;Failed statistics;All tests passed;exodiff: Files are the same;would be required for this test to be rigorous" PROCESSORS "4" WORKING_DIRECTORY "/home/aditya/quinoa/cmake/regression/walker/MassFracBeta/walker:MassFracBeta_pe4")
add_test(walker:MassFracBeta_u0.9_pe4 "/usr/bin/cmake" "-DTEST_NAME=walker:MassFracBeta_u0.9_pe4" "-DWORKDIR=/home/aditya/quinoa/cmake/regression/walker/MassFracBeta/walker:MassFracBeta_u0.9_pe4" "-DRUNNER=/home/aditya/quinoa/cmake/charmrun" "-DRUNNER_NCPUS_ARG=+p" "-DRUNNER_ARGS=--bind-to none --map-by node" "-DTEST_EXECUTABLE=/home/aditya/quinoa/cmake/Main/walker" "-DTEST_EXECUTABLE_ARGS=-c massfracbeta.q -v -u 0.9" "-DTEST_LABELS=LABELS walker" "-DNUMPES=4" "-DTEXT_DIFF_PROG=/home/aditya/quinoa/tpl/install/gnu-x86_64/bin/maddiff" "-DTEXT_DIFF_PROG_ARGS=" "-DTEXT_DIFF_PROG_CONF=massfracbeta.ndiff.cfg" "-DTEXT_BASELINE=stat.txt.std" "-DTEXT_RESULT=stat.txt" "-DBIN_DIFF_PROG=/home/aditya/quinoa/tpl/install/gnu-x86_64/bin/exodiff" "-DBIN_DIFF_PROG_ARGS=" "-DBIN_DIFF_PROG_CONF=" "-DBIN_BASELINE=" "-DBIN_RESULT=" "-DPOSTPROCESS_PROG=" "-DPOSTPROCESS_PROG_ARGS=" "-DPOSTPROCESS_PROG_OUTPUT=" "-P" "/home/aditya/quinoa/src/../cmake/test_runner.cmake")
set_tests_properties(walker:MassFracBeta_u0.9_pe4 PROPERTIES  FAIL_REGULAR_EXPRESSION "exodiff: ERROR;exodiff: Files are different" LABELS "walker" PASS_REGULAR_EXPRESSION ".*walker:MassFracBeta_u0.9_pe4.*PASS;Failed statistics;All tests passed;exodiff: Files are the same;would be required for this test to be rigorous" PROCESSORS "4" WORKING_DIRECTORY "/home/aditya/quinoa/cmake/regression/walker/MassFracBeta/walker:MassFracBeta_u0.9_pe4")
