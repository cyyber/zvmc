# EVMC: Ethereum Client-VM Connector API.
# Copyright 2019 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.


# Adds a CMake test to check the given QRVMC VM implementation with the qrvmc-vmtester tool.
#
# qrvmc_add_vm_test(NAME <test_name> TARGET <vm>)
# - NAME argument specifies the name of the added test,
# - TARGET argument specifies the CMake target being a shared library with QRVMC VM implementation.
function(qrvmc_add_vm_test)
    if(NOT TARGET qrvmc::qrvmc-vmtester)
        message(FATAL_ERROR "The qrvmc-vmtester has not been installed with this QRVMC package")
    endif()

    cmake_parse_arguments("" "" NAME;TARGET "" ${ARGN})
    add_test(NAME ${_NAME} COMMAND qrvmc::qrvmc-vmtester $<TARGET_FILE:${_TARGET}>)
endfunction()
