#pragma once

#include <qrvmc/qrvmc.hpp>
#include <gtest/gtest.h>

class qrvmc_vm_test : public ::testing::Test
{
public:
    static void set_vm(qrvmc::VM _owned_vm) noexcept { owned_vm = std::move(_owned_vm); }

protected:
    /// The raw pointer to the loaded VM instance.
    /// The C API is used to allow more sophisticated unit tests.
    qrvmc_vm* vm = nullptr;

    /// The C++ RAII wrapper of the loaded VM instance.
    static qrvmc::VM owned_vm;

    qrvmc_vm_test() : vm{owned_vm.get_raw_pointer()} {}

    void SetUp() override { ASSERT_TRUE(vm != nullptr) << "VM instance not loaded"; }
};
