# EVMC: Ethereum Client-VM Connector API.
# Copyright 2018 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

# This example shows how to use qrvmc INTERFACE library from qrvmc CMake package.

cmake_minimum_required(VERSION 3.5)

project(use_qrvmc_in_cmake)

find_package(qrvmc CONFIG REQUIRED)

add_executable(use_qrvmc_in_cmake use_qrvmc_in_cmake.c)
target_link_libraries(use_qrvmc_in_cmake PRIVATE qrvmc::qrvmc)



# Only for integration tests.
if(NOT COMMAND qrvmc_add_vm_test)
    message(FATAL_ERROR "Function qrvmc_add_vm_test() not in QRVMC.cmake module")
endif()
