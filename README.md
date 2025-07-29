# QRVMC

[![chat: on gitter][gitter badge]][Gitter]
[![readme style: standard][readme style standard badge]][standard readme]

> QRL Client-VM Connector API

The QRVMC is the low-level ABI between Quantum Resistant Virtual Machines (QRVMs) and QRL Clients. On the QRVM side it supports classic QRVM1.
On the Client-side it defines the interface for QRVM implementations
to access QRL environment and state.


## Usage

### Documentation

Please visit the [documentation].

### Languages support

| Language                      | Supported Versions   | Supported Compilers          | Feature Support   |
|-------------------------------|----------------------|------------------------------|-------------------|
| **C**                         | C99, C11             | GCC 8+, clang 9+, MSVC 2017+ | Host- and VM-side |
| **C++**                       | C++17                | GCC 8+, clang 9+, MSVC 2017+ | Host- and VM-side |

### Testing tools

* **qrvmc run** ([tools/qrvmc]) — executes bytecode in any QRVMC-compatible VM implementation.
* **qrvmc-vmtester** ([tools/vmtester]) — can test any QRVM implementation for compatibility with QRVMC.
* **qrvm-test** ([qrvmone → test/unittests]) — allows running the collection of [qrvmone]'s unit tests on any QRVMC-compatible QRVM implementation.
* **qrvmone-fuzzer** ([qrvmone → test/fuzzer]) — differential fuzzer for QRVMC-compatible QRVM implementations. 

## License

[![license badge]][Apache License, Version 2.0]

Licensed under the [Apache License, Version 2.0].

## Internal

### Making new release

1. Update [CHANGELOG.md](CHANGELOG.md), put the release date, update release link.
2. `git add CHANGELOG.md`.
3. Tag new release: `bumpversion --allow-dirty prerel`.
4. Prepare CHANGELOG for next release: add unreleased section and link.
5. `git add CHANGELOG.md`.
6. Start new release series: `bumpversion --allow-dirty --no-tag minor`.