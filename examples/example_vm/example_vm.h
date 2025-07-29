// EVMC: Ethereum Client-VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#pragma once

#include <qrvmc/utils.h>
#include <qrvmc/qrvmc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates QRVMC Example VM.
 */
QRVMC_EXPORT struct qrvmc_vm* qrvmc_create_example_vm(void);

#ifdef __cplusplus
}
#endif
