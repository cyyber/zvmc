# EVMC: Ethereum Client-VM Connector API.
# Copyright 2018 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

add_library(
    loader STATIC
    ${QRVMC_INCLUDE_DIR}/qrvmc/loader.h
    loader.c
)

add_library(qrvmc::loader ALIAS loader)
set_target_properties(loader PROPERTIES
    OUTPUT_NAME qrvmc-loader
    POSITION_INDEPENDENT_CODE TRUE
)
target_link_libraries(loader INTERFACE ${CMAKE_DL_LIBS} PUBLIC qrvmc::qrvmc)

if(QRVMC_INSTALL)
    install(TARGETS loader EXPORT qrvmcTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
