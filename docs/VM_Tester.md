# QRVMC VM Tester {#vmtester}

The QRVMC project contains a QRVMC-compatibility testing tool for VM implementations.

The tool is called `qrvmc-vmtester` and to include it in the QRVMC build
add `-DQRVMC_TESTING=ON` CMake option to the project configuration step.

Usage is simple as

```sh
qrvmc-vmtester [vm]
```

where `[vm]` is a path to a shared library with VM implementation.

For more information check `qrvmc-vmtester --help`.
