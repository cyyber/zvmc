# QRVMC – QRL Client-VM Connector API {#mainpage}

**ABI version 1**

The QRVMC is the low-level ABI between Quantum Resistant Virtual Machines (QRVMs) and QRL Clients. On the QRVM-side it supports classic QRVM1.
On the Client-side it defines the interface for QRVM implementations
to access QRL environment and state.


# Guides {#guides}

- [Host Implementation Guide](@ref hostguide)
- [VM Implementation Guide](@ref vmguide)


# Versioning {#versioning}

The QRVMC project uses [Semantic Versioning](https://semver.org).
The version format is `MAJOR.MINOR.PATCH`.

The _MAJOR_ version number is also referenced as the **QRVMC ABI version**.
This ABI version is available to VM and Host implementations by ::QRVMC_ABI_VERSION.
For example QRVMC 3.2.1 would have ABI version 3 and therefore this project release
can be referenced as QRVMC ABIv3 or just QRVMC 3.
Every C ABI breaking change requires increasing the _MAJOR_ version number.

The releases with _MINOR_ version change allow adding new API features
and modifying the language bindings API.
Backward incompatible API changes are allowed but should be avoided if possible.

The releases with _PATCH_ should only include bug fixes. Exceptionally,
API changes are allowed when required to fix a broken feature.


# Modules {#modules}

- [QRVMC](@ref QRVMC)
   – the main component that defines API for VMs and Clients (Hosts).
- [QRVMC C++ API](@ref qrvmc)
   – the wrappers and bindings for C++.
- [QRVMC Loader](@ref loader)
   – the library for loading VMs implemented as Dynamically Loaded Libraries (DLLs, shared objects).
- [QRVMC Helpers](@ref helpers)
   – a collection of utility functions for easier integration with QRVMC.
- [QRVM Instructions](@ref instructions)
   – the library with collection of metrics for QRVM1 instruction set.
- [QRVMC VM Tester](@ref vmtester)
   – the QRVMC-compatibility testing tool for VM implementations.

## Terms

1. **VM** – A Quantum Resistant Virtual Machine instance/implementation.
2. **Host** – An entity controlling the VM.
   The Host requests code execution and responses to VM queries by callback
   functions. This usually represents a QRL Client.


## Responsibilities

### VM

- Executes the code (obviously).
- Calculates the running gas cost and manages the gas counter except the refund
  counter.
- Controls the call depth, including the exceptional termination of execution
  in case the maximum depth is reached.


### Host

- Provides access to State.
- Creates new accounts (with code being a result of VM execution).
- Handles refunds entirely.
- Manages the set of precompiled contracts and handles execution of messages
  coming to them.
