// EVMC: Ethereum Client-VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

use crate::QrvmcVm;

use std::ops::{Deref, DerefMut};

/// Container struct for QRVMC instances and user-defined data.
pub struct QrvmcContainer<T>
where
    T: QrvmcVm + Sized,
{
    #[allow(dead_code)]
    instance: ::qrvmc_sys::qrvmc_vm,
    vm: T,
}

impl<T> QrvmcContainer<T>
where
    T: QrvmcVm + Sized,
{
    /// Basic constructor.
    pub fn new(_instance: ::qrvmc_sys::qrvmc_vm) -> Box<Self> {
        Box::new(Self {
            instance: _instance,
            vm: T::init(),
        })
    }

    /// Take ownership of the given pointer and return a box.
    ///
    /// # Safety
    /// This function expects a valid instance to be passed.
    pub unsafe fn from_ffi_pointer(instance: *mut ::qrvmc_sys::qrvmc_vm) -> Box<Self> {
        assert!(!instance.is_null(), "from_ffi_pointer received NULL");
        Box::from_raw(instance as *mut QrvmcContainer<T>)
    }

    /// Convert boxed self into an FFI pointer, surrendering ownership of the heap data.
    ///
    /// # Safety
    /// This function will return a valid instance pointer.
    pub unsafe fn into_ffi_pointer(boxed: Box<Self>) -> *mut ::qrvmc_sys::qrvmc_vm {
        Box::into_raw(boxed) as *mut ::qrvmc_sys::qrvmc_vm
    }
}

impl<T> Deref for QrvmcContainer<T>
where
    T: QrvmcVm,
{
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.vm
    }
}

impl<T> DerefMut for QrvmcContainer<T>
where
    T: QrvmcVm,
{
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.vm
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::types::*;
    use crate::{ExecutionContext, ExecutionMessage, ExecutionResult};

    struct TestVm {}

    impl QrvmcVm for TestVm {
        fn init() -> Self {
            TestVm {}
        }
        fn execute(
            &self,
            _revision: qrvmc_sys::qrvmc_revision,
            _code: &[u8],
            _message: &ExecutionMessage,
            _context: Option<&mut ExecutionContext>,
        ) -> ExecutionResult {
            ExecutionResult::failure()
        }
    }

    unsafe extern "C" fn get_dummy_tx_context(
        _context: *mut qrvmc_sys::qrvmc_host_context,
    ) -> qrvmc_sys::qrvmc_tx_context {
        qrvmc_sys::qrvmc_tx_context {
            tx_gas_price: Uint256::default(),
            tx_origin: Address::default(),
            block_coinbase: Address::default(),
            block_number: 0,
            block_timestamp: 0,
            block_gas_limit: 0,
            block_prev_randao: Uint256::default(),
            chain_id: Uint256::default(),
            block_base_fee: Uint256::default(),
        }
    }

    #[test]
    fn container_new() {
        let instance = ::qrvmc_sys::qrvmc_vm {
            abi_version: ::qrvmc_sys::QRVMC_ABI_VERSION as i32,
            name: std::ptr::null(),
            version: std::ptr::null(),
            destroy: None,
            execute: None,
            get_capabilities: None,
            set_option: None,
        };

        let code = [0u8; 0];

        let message = ::qrvmc_sys::qrvmc_message {
            kind: ::qrvmc_sys::qrvmc_call_kind::QRVMC_CALL,
            flags: 0,
            depth: 0,
            gas: 0,
            recipient: ::qrvmc_sys::qrvmc_address::default(),
            sender: ::qrvmc_sys::qrvmc_address::default(),
            input_data: std::ptr::null(),
            input_size: 0,
            value: ::qrvmc_sys::qrvmc_uint256be::default(),
            create2_salt: ::qrvmc_sys::qrvmc_bytes32::default(),
            code_address: ::qrvmc_sys::qrvmc_address::default(),
        };
        let message: ExecutionMessage = (&message).into();

        let host = ::qrvmc_sys::qrvmc_host_interface {
            account_exists: None,
            get_storage: None,
            set_storage: None,
            get_balance: None,
            get_code_size: None,
            get_code_hash: None,
            copy_code: None,
            call: None,
            get_tx_context: Some(get_dummy_tx_context),
            get_block_hash: None,
            emit_log: None,
            access_account: None,
            access_storage: None,
        };
        let host_context = std::ptr::null_mut();

        let mut context = ExecutionContext::new(&host, host_context);
        let container = QrvmcContainer::<TestVm>::new(instance);
        assert_eq!(
            container
                .execute(
                    qrvmc_sys::qrvmc_revision::QRVMC_SHANGHAI,
                    &code,
                    &message,
                    Some(&mut context)
                )
                .status_code(),
            ::qrvmc_sys::qrvmc_status_code::QRVMC_FAILURE
        );

        let ptr = unsafe { QrvmcContainer::into_ffi_pointer(container) };

        let mut context = ExecutionContext::new(&host, host_context);
        let container = unsafe { QrvmcContainer::<TestVm>::from_ffi_pointer(ptr) };
        assert_eq!(
            container
                .execute(
                    qrvmc_sys::qrvmc_revision::QRVMC_SHANGHAI,
                    &code,
                    &message,
                    Some(&mut context)
                )
                .status_code(),
            ::qrvmc_sys::qrvmc_status_code::QRVMC_FAILURE
        );
    }
}
