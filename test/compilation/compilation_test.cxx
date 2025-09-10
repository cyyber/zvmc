// EVMC: Ethereum Client-VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

// Test compilation of C and C++ public headers.

#include <qrvmc/qrvmc.h>
#include <qrvmc/qrvmc.hpp>
#include <qrvmc/filter_iterator.hpp>
#include <qrvmc/helpers.h>
#include <qrvmc/hex.hpp>
#include <qrvmc/instructions.h>
#include <qrvmc/loader.h>
#include <qrvmc/mocked_host.hpp>
#include <qrvmc/utils.h>

// Include again to check if headers have proper include guards.
#include <qrvmc/qrvmc.h>               //NOLINT(readability-duplicate-include)
#include <qrvmc/qrvmc.hpp>             //NOLINT(readability-duplicate-include)
#include <qrvmc/filter_iterator.hpp>  //NOLINT(readability-duplicate-include)
#include <qrvmc/helpers.h>            //NOLINT(readability-duplicate-include)
#include <qrvmc/hex.hpp>              //NOLINT(readability-duplicate-include)
#include <qrvmc/instructions.h>       //NOLINT(readability-duplicate-include)
#include <qrvmc/loader.h>             //NOLINT(readability-duplicate-include)
#include <qrvmc/mocked_host.hpp>      //NOLINT(readability-duplicate-include)
#include <qrvmc/utils.h>              //NOLINT(readability-duplicate-include)
