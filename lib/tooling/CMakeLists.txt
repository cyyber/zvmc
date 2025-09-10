# EVMC: Ethereum Client-VM Connector API.
# Copyright 2021 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

add_library(tooling STATIC)
add_library(qrvmc::tooling ALIAS tooling)
target_compile_features(tooling PUBLIC cxx_std_17)
target_link_libraries(tooling PUBLIC qrvmc::qrvmc_cpp qrvmc::mocked_host)

target_sources(
    tooling PRIVATE
    ${QRVMC_INCLUDE_DIR}/qrvmc/tooling.hpp
    run.cpp
)

if(QRVMC_INSTALL)
    install(TARGETS tooling EXPORT qrvmcTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
