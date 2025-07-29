# EVMC: Ethereum Client-VM Connector API.
# Copyright 2016 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

include(GNUInstallDirs)

if(MSVC AND CMAKE_C_COMPILER_VERSION VERSION_LESS 19.20)
    # For Visual Studio _before_ 2019
    # allow non-constant aggregate initializer.
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C>:/wd4204>
        $<$<COMPILE_LANGUAGE:C>:/wd4221>
    )
endif()

add_subdirectory(example_vm)
add_subdirectory(example_precompiles_vm)

add_library(qrvmc-example-host STATIC example_host.cpp)
target_link_libraries(qrvmc-example-host PRIVATE qrvmc::qrvmc_cpp)

add_executable(qrvmc-example-static example.c)
target_compile_features(qrvmc-example-static PRIVATE c_std_99)
target_link_libraries(qrvmc-example-static PRIVATE qrvmc-example-host qrvmc::example-vm-static qrvmc::qrvmc)
target_compile_definitions(qrvmc-example-static PRIVATE STATICALLY_LINKED_EXAMPLE)

add_executable(qrvmc-example example.c)
target_compile_features(qrvmc-example PRIVATE c_std_99)
target_link_libraries(qrvmc-example PRIVATE qrvmc-example-host qrvmc::loader)
