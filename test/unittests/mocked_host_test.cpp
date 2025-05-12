// EVMC: Ethereum Client-VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include <zvmc/mocked_host.hpp>
#include <gtest/gtest.h>

using namespace zvmc::literals;

TEST(mocked_host, mocked_account)
{
    zvmc::MockedAccount account;
    EXPECT_EQ(account.nonce, 0);
    --account.nonce;
    account.set_balance(0x0102030405060708);

    EXPECT_EQ(account.balance,
              0x0000000000000000000000000000000000000000000000000102030405060708_bytes32);
    EXPECT_EQ(account.nonce, -1);
}

TEST(mocked_host, storage)
{
    const auto addr1 = zvmc::address{};
    const auto addr2 = "Z200000000000000000000000000000000000000000000000"_address;
    const auto val1 = zvmc::bytes32{};
    const auto val2 = 0x2000000000000000000000000000000000000000000000000102030405060708_bytes32;
    const auto val3 = 0x1000000000000000000000000000000000000000000000000000000000000000_bytes32;

    zvmc::MockedHost host;
    const auto& chost = host;

    // Null bytes returned for non-existing accounts.
    EXPECT_EQ(chost.get_storage(addr1, {}), zvmc::bytes32{});
    EXPECT_EQ(chost.get_storage(addr2, {}), zvmc::bytes32{});

    // Set storage on non-existing account creates the account.
    EXPECT_EQ(host.set_storage(addr1, val1, val2), ZVMC_STORAGE_ADDED);
    EXPECT_EQ(chost.accounts.count(addr1), 1u);
    EXPECT_EQ(host.accounts[addr1].storage.count(val1), 1u);
    EXPECT_EQ(host.accounts[addr1].storage[val1].current, val2);

    auto& acc2 = host.accounts[addr2];
    EXPECT_EQ(chost.get_storage(addr2, val1), zvmc::bytes32{});
    EXPECT_EQ(acc2.storage.size(), 0u);
    EXPECT_EQ(host.set_storage(addr2, val1, val2), ZVMC_STORAGE_ADDED);
    EXPECT_EQ(chost.get_storage(addr2, val1), val2);
    EXPECT_EQ(acc2.storage.count(val1), 1u);
    EXPECT_EQ(host.set_storage(addr2, val1, val2), ZVMC_STORAGE_ASSIGNED);
    EXPECT_EQ(chost.get_storage(addr2, val1), val2);
    EXPECT_EQ(acc2.storage.count(val1), 1u);
    EXPECT_EQ(host.set_storage(addr2, val1, val3), ZVMC_STORAGE_ASSIGNED);
    EXPECT_EQ(chost.get_storage(addr2, val1), val3);
    EXPECT_EQ(acc2.storage.count(val1), 1u);
    EXPECT_NE(acc2.storage[val1].current, acc2.storage[val1].original);
    EXPECT_EQ(host.set_storage(addr2, val1, val1), ZVMC_STORAGE_ADDED_DELETED);
    EXPECT_EQ(chost.get_storage(addr2, val1), val1);
    EXPECT_EQ(acc2.storage.count(val1), 1u);
    EXPECT_EQ(acc2.storage.size(), 1u);
    EXPECT_EQ(acc2.storage[val1].current, acc2.storage[val1].original);

    EXPECT_EQ(chost.get_storage(addr2, val3), zvmc::bytes32{});
    acc2.storage[val3] = val2;
    EXPECT_EQ(chost.get_storage(addr2, val3), val2);
    EXPECT_EQ(acc2.storage.find(val3)->second.current, acc2.storage.find(val3)->second.original);
    EXPECT_EQ(host.set_storage(addr2, val3, val2), ZVMC_STORAGE_ASSIGNED);
    EXPECT_EQ(chost.get_storage(addr2, val3), val2);
    EXPECT_EQ(host.set_storage(addr2, val3, val3), ZVMC_STORAGE_MODIFIED);
    EXPECT_EQ(chost.get_storage(addr2, val3), val3);
    acc2.storage[val3].original = acc2.storage[val3].current;
    EXPECT_EQ(host.set_storage(addr2, val3, val1), ZVMC_STORAGE_DELETED);
    EXPECT_EQ(chost.get_storage(addr2, val3), val1);
}

TEST(mocked_host, storage_update_scenarios)
{
    static constexpr auto addr = "Zff"_address;
    static constexpr auto key = 0xfe_bytes32;

    static constexpr auto execute_scenario = [](const zvmc::bytes32& original,
                                                const zvmc::bytes32& current,
                                                const zvmc::bytes32& value) {
        zvmc::MockedHost host;
        host.accounts[addr].storage[key] = {current, original};
        return host.set_storage(addr, key, value);
    };

    static constexpr auto O = 0x00_bytes32;
    static constexpr auto X = 0x01_bytes32;
    static constexpr auto Y = 0x02_bytes32;
    static constexpr auto Z = 0x03_bytes32;

    EXPECT_EQ(execute_scenario(O, O, O), ZVMC_STORAGE_ASSIGNED);
    EXPECT_EQ(execute_scenario(X, O, O), ZVMC_STORAGE_ASSIGNED);
    EXPECT_EQ(execute_scenario(O, Y, Y), ZVMC_STORAGE_ASSIGNED);
    EXPECT_EQ(execute_scenario(X, Y, Y), ZVMC_STORAGE_ASSIGNED);
    EXPECT_EQ(execute_scenario(Y, Y, Y), ZVMC_STORAGE_ASSIGNED);
    EXPECT_EQ(execute_scenario(O, Y, Z), ZVMC_STORAGE_ASSIGNED);
    EXPECT_EQ(execute_scenario(X, Y, Z), ZVMC_STORAGE_ASSIGNED);

    EXPECT_EQ(execute_scenario(O, O, Z), ZVMC_STORAGE_ADDED);
    EXPECT_EQ(execute_scenario(X, X, O), ZVMC_STORAGE_DELETED);
    EXPECT_EQ(execute_scenario(X, X, Z), ZVMC_STORAGE_MODIFIED);
    EXPECT_EQ(execute_scenario(X, O, Z), ZVMC_STORAGE_DELETED_ADDED);
    EXPECT_EQ(execute_scenario(X, Y, O), ZVMC_STORAGE_MODIFIED_DELETED);
    EXPECT_EQ(execute_scenario(X, O, X), ZVMC_STORAGE_DELETED_RESTORED);
    EXPECT_EQ(execute_scenario(O, Y, O), ZVMC_STORAGE_ADDED_DELETED);
    EXPECT_EQ(execute_scenario(X, Y, X), ZVMC_STORAGE_MODIFIED_RESTORED);
}
