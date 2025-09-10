// EVMC: Ethereum Client-VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

/** This example shows how to use qrvmc INTERFACE library from qrvmc CMake package. */

#include <qrvmc/qrvmc.h>

int main()
{
    struct qrvmc_vm vm = {.abi_version = QRVMC_ABI_VERSION};
    return vm.abi_version - QRVMC_ABI_VERSION;
}
