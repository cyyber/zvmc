// EVMC: Ethereum Client-VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

/** This example shows how to use qrvmc::instructions library from qrvmc CMake package. */

#include <qrvmc/instructions.h>

int main()
{
    return qrvmc_get_instruction_metrics_table(QRVMC_SHANGHAI)[OP_STOP].gas_cost;
}
