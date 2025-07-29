// EVMC: Ethereum Client-VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <qrvmc/utils.h>
#include <qrvmc/qrvmc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates QRVMC Example Precompiles VM.
 */
QRVMC_EXPORT struct qrvmc_vm* qrvmc_create_example_precompiles_vm(void);

#ifdef __cplusplus
}
#endif
