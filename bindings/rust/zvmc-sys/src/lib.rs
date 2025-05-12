// EVMC: Ethereum Client-VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

// Defining zvmc_host_context here, because bindgen cannot create a useful declaration yet.

/// This is a void type given host context is an opaque pointer. Functions allow it to be a null ptr.
pub type zvmc_host_context = ::std::os::raw::c_void;

// TODO: add `.derive_default(true)` to bindgen instead?

impl Default for zvmc_address {
    fn default() -> Self {
        zvmc_address { bytes: [0u8; 24] }
    }
}

impl Default for zvmc_bytes32 {
    fn default() -> Self {
        zvmc_bytes32 { bytes: [0u8; 32] }
    }
}

#[cfg(test)]
mod tests {
    use std::mem::size_of;

    use super::*;

    #[test]
    fn container_new() {
        // TODO: add other checks from test/unittests/test_helpers.cpp
        assert_eq!(size_of::<zvmc_bytes32>(), 32);
        assert_eq!(size_of::<zvmc_address>(), 24);
        assert!(size_of::<zvmc_vm>() <= 64);
    }
}
