// EVMC: Ethereum Client-VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

use std::collections::HashMap;
use qrvmc_declare::qrvmc_declare_vm;
use qrvmc_vm::ExecutionContext;
use qrvmc_vm::ExecutionMessage;
use qrvmc_vm::ExecutionResult;
use qrvmc_vm::SetOptionError;
use qrvmc_vm::QrvmcVm;

#[qrvmc_declare_vm("Foo VM", "zwasm, qrvm", "1.42-alpha.gamma.starship")]
pub struct FooVM {
    options: HashMap<String, String>,
}

impl QrvmcVm for FooVM {
    fn init() -> Self {
        Self {
            options: Default::default(),
        }
    }

    fn set_option(&mut self, key: &str, value: &str) -> Result<(), SetOptionError> {
        self.options.insert(key.to_string(), value.to_string());

        Ok(())
    }

    fn execute(
        &self,
        _revision: qrvmc_sys::qrvmc_revision,
        _code: &[u8],
        _message: &ExecutionMessage,
        _context: Option<&mut ExecutionContext>,
    ) -> ExecutionResult {
        ExecutionResult::success(1337, 21, None)
    }
}
