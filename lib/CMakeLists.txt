# EVMC: Ethereum Client-VM Connector API.
# Copyright 2018 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

add_library(qrvmc INTERFACE)
add_library(qrvmc::qrvmc ALIAS qrvmc)
target_compile_features(qrvmc INTERFACE c_std_99)
target_include_directories(qrvmc INTERFACE $<BUILD_INTERFACE:${QRVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>)

add_library(qrvmc_cpp INTERFACE)
add_library(qrvmc::qrvmc_cpp ALIAS qrvmc_cpp)
target_compile_features(qrvmc_cpp INTERFACE cxx_std_17)
target_include_directories(qrvmc_cpp INTERFACE $<BUILD_INTERFACE:${QRVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>)
target_link_libraries(qrvmc_cpp INTERFACE qrvmc::qrvmc)

add_subdirectory(instructions)
add_subdirectory(loader)
add_subdirectory(mocked_host)
add_subdirectory(tooling)

if(QRVMC_INSTALL)
    install(TARGETS qrvmc qrvmc_cpp EXPORT qrvmcTargets)
endif()
