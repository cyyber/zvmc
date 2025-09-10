# EVMC: Ethereum Client-VM Connector API.
# Copyright 2018 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

add_library(
    instructions STATIC
    ${QRVMC_INCLUDE_DIR}/qrvmc/instructions.h
    instruction_metrics.c
    instruction_names.c
)

add_library(qrvmc::instructions ALIAS instructions)
set_target_properties(instructions PROPERTIES
    OUTPUT_NAME qrvmc-instructions
    POSITION_INDEPENDENT_CODE TRUE
)
target_include_directories(instructions PUBLIC
    $<BUILD_INTERFACE:${QRVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>
)

if(QRVMC_INSTALL)
    install(TARGETS instructions EXPORT qrvmcTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
