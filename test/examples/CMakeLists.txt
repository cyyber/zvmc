# EVMC: Ethereum Client-VM Connector API.
# Copyright 2020 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

set(PREFIX ${PROJECT_NAME}/examples)

add_test(NAME ${PREFIX}/example-static COMMAND qrvmc-example-static)
add_test(NAME ${PREFIX}/example-dynamic-load COMMAND qrvmc-example $<TARGET_FILE:qrvmc::example-vm>)
