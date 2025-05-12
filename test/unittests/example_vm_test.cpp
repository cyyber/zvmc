// EVMC: Ethereum Client-VM Connector API.
// Copyright 2020 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include "../../examples/example_vm/example_vm.h"
#include <zvmc/hex.hpp>
#include <zvmc/mocked_host.hpp>
#include <zvmc/zvmc.hpp>
#include <gtest/gtest.h>
#include <cstring>

using namespace zvmc::literals;

namespace
{
struct Output
{
    zvmc::bytes bytes;

    explicit Output(const char* output_hex) noexcept : bytes{zvmc::from_hex(output_hex).value()} {}

    friend bool operator==(const zvmc::Result& result, const Output& expected) noexcept
    {
        return expected.bytes.compare(0, zvmc::bytes::npos, result.output_data,
                                      result.output_size) == 0;
    }
};

auto vm = zvmc::VM{zvmc_create_example_vm()};

class example_vm : public testing::Test
{
protected:
    zvmc_revision rev = ZVMC_MAX_REVISION;
    zvmc::MockedHost host;
    zvmc_message msg{};

    example_vm() noexcept
    {
        msg.sender = "Z500000000000000000000000000000000000000000000005"_address;
        msg.recipient = "Zd0000000000000000000000000000000000000000000000d"_address;
    }

    zvmc::Result execute_in_example_vm(int64_t gas,
                                       const char* code_hex,
                                       const char* input_hex = "")
    {
        const auto code = zvmc::from_hex(code_hex).value();
        const auto input = zvmc::from_hex(input_hex).value();

        msg.gas = gas;
        msg.input_data = input.data();
        msg.input_size = input.size();

        return vm.execute(host, rev, msg, code.data(), code.size());
    }
};


}  // namespace

TEST_F(example_vm, empty_code)
{
    const auto r = execute_in_example_vm(999, "");
    EXPECT_EQ(r.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(r.gas_left, 999);
    EXPECT_EQ(r.output_size, size_t{0});
}

TEST_F(example_vm, push)
{
    // Yul:
    // mstore(0, 0xd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeef) return(0, 32)
    const auto r = execute_in_example_vm(
        10, "7fd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeef60005260206000f3");
    EXPECT_EQ(r.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(r.gas_left, 4);
    EXPECT_EQ(r, Output("d0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeef"));
}

TEST_F(example_vm, return_address)
{
    // Yul: mstore(0, address()) return(12, 20)
    const auto r = execute_in_example_vm(6, "306000526014600cf3");
    EXPECT_EQ(r.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(r.gas_left, 0);
    EXPECT_EQ(r, Output("d00000000000000000000000000000000000000d"));
}

TEST_F(example_vm, counter_in_storage)
{
    // Yul: sstore(0, add(sload(0), 1)) stop()
    auto& storage_value = host.accounts[msg.recipient].storage[{}].current;
    storage_value = 0x00000000000000000000000000000000000000000000000000000000000000bb_bytes32;
    const auto r = execute_in_example_vm(10, "60016000540160005500");
    EXPECT_EQ(r.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(r.gas_left, 3);
    EXPECT_EQ(r, Output(""));
    EXPECT_EQ(storage_value,
              0x00000000000000000000000000000000000000000000000000000000000000bc_bytes32);
}

TEST_F(example_vm, return_block_number)
{
    // Yul: mstore(0, number()) return(0, msize())
    host.tx_context.block_number = 0xb4;
    const auto r = execute_in_example_vm(7, "43600052596000f3");
    EXPECT_EQ(r.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(r.gas_left, 1);
    EXPECT_EQ(r, Output("00000000000000000000000000000000000000000000000000000000000000b4"));
}

TEST_F(example_vm, return_out_of_memory)
{
    // Yul: return(1024, 1)
    const auto r = execute_in_example_vm(10, "6001610400f3");
    EXPECT_EQ(r.status_code, ZVMC_FAILURE);
    EXPECT_EQ(r.gas_left, 0);
    EXPECT_EQ(r, Output(""));
}

TEST_F(example_vm, revert_out_of_memory)
{
    // Yul: revert(512, 513)
    const auto r = execute_in_example_vm(10, "610201610200fd");
    EXPECT_EQ(r.status_code, ZVMC_FAILURE);
    EXPECT_EQ(r.gas_left, 0);
    EXPECT_EQ(r, Output(""));
}

TEST_F(example_vm, revert_block_number)
{
    // Yul: mstore(0, number()) revert(0, 32)
    host.tx_context.block_number = 0xb4;
    const auto r = execute_in_example_vm(7, "4360005260206000fd");
    EXPECT_EQ(r.status_code, ZVMC_REVERT);
    EXPECT_EQ(r.gas_left, 1);
    EXPECT_EQ(r, Output("00000000000000000000000000000000000000000000000000000000000000b4"));
}

TEST_F(example_vm, call)
{
    // pseudo-Yul: call(3, 3, 3, 3, 3, 3, 3) return(0, msize())
    const auto expected_output = zvmc::from_hex("aabbcc").value();
    host.call_result.output_data = expected_output.data();
    host.call_result.output_size = expected_output.size();
    const auto r = execute_in_example_vm(100, "6003808080808080f1596000f3");
    EXPECT_EQ(r.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(r.gas_left, 89);
    EXPECT_EQ(r, Output("000000aabbcc"));
    ASSERT_EQ(host.recorded_calls.size(), size_t{1});
    EXPECT_EQ(host.recorded_calls[0].flags, uint32_t{0});
    EXPECT_EQ(host.recorded_calls[0].gas, 3);
    EXPECT_EQ(host.recorded_calls[0].value,
              0x0000000000000000000000000000000000000000000000000000000000000003_bytes32);
    EXPECT_EQ(host.recorded_calls[0].recipient,
              "Z000000000000000000000000000000000000000000000003"_address);
    EXPECT_EQ(host.recorded_calls[0].input_size, size_t{3});
}

TEST_F(example_vm, calldataload_full)
{
    // Yul: mstore(0, calldataload(2)) return(0, msize())
    const auto r = execute_in_example_vm(
        7, "600235600052596000f3",
        "4444000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");
    EXPECT_EQ(r.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(r.gas_left, 0);
    EXPECT_EQ(r, Output("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"));
}

TEST_F(example_vm, calldataload_partial)
{
    // Yul: mstore(0, calldataload(0)) return(0, msize())
    const auto r = execute_in_example_vm(7, "600035600052596000f3", "aabbccdd");
    EXPECT_EQ(r.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(r.gas_left, 0);
    EXPECT_EQ(r, Output("aabbccdd00000000000000000000000000000000000000000000000000000000"));
}

TEST_F(example_vm, calldataload_empty)
{
    // Yul: mstore(0, calldataload(4)) return(0, msize())
    const auto r = execute_in_example_vm(7, "600435600052596000f3", "aabbccdd");
    EXPECT_EQ(r.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(r.gas_left, 0);
    EXPECT_EQ(r, Output("0000000000000000000000000000000000000000000000000000000000000000"));
}

TEST_F(example_vm, mstore_out_of_memory)
{
    // Yul: mstore(1023, 0xffff)
    const auto r = execute_in_example_vm(9, "61ffff6103ff52");
    EXPECT_EQ(r.status_code, ZVMC_FAILURE);
    EXPECT_EQ(r.gas_left, 0);
    EXPECT_EQ(r, Output(""));
}
