// EVMC: Ethereum Client-VM Connector API.
// Copyright 2020 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include <qrvmc/qrvmc.hpp>
#include <iosfwd>
#include <string>

namespace qrvmc::tooling
{
int run(VM& vm,
        qrvmc_revision rev,
        int64_t gas,
        bytes_view code,
        bytes_view input,
        bool create,
        bool bench,
        std::ostream& out);
}  // namespace qrvmc::tooling
