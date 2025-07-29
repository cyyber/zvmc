// EVMC: Ethereum Client-VM Connector API.
// Copyright 2016 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

/// @file
/// Example implementation of a QRVMC Host.

#include "example_host.h"

#include <qrvmc/qrvmc.hpp>

#include <algorithm>
#include <map>
#include <vector>

using namespace qrvmc::literals;

namespace qrvmc
{
struct account
{
    virtual ~account() = default;

    qrvmc::uint256be balance = {};
    std::vector<uint8_t> code;
    std::map<qrvmc::bytes32, qrvmc::bytes32> storage;

    virtual qrvmc::bytes32 code_hash() const
    {
        // Extremely dumb "hash" function.
        qrvmc::bytes32 ret{};
        for (const auto v : code)
            ret.bytes[v % sizeof(ret.bytes)] ^= v;
        return ret;
    }
};

using accounts = std::map<qrvmc::address, account>;

}  // namespace qrvmc

class ExampleHost : public qrvmc::Host
{
    qrvmc::accounts accounts;
    qrvmc_tx_context tx_context{};

public:
    ExampleHost() = default;
    explicit ExampleHost(qrvmc_tx_context& _tx_context) noexcept : tx_context{_tx_context} {}
    ExampleHost(qrvmc_tx_context& _tx_context, qrvmc::accounts& _accounts) noexcept
      : accounts{_accounts}, tx_context{_tx_context}
    {}

    bool account_exists(const qrvmc::address& addr) const noexcept final
    {
        return accounts.find(addr) != accounts.end();
    }

    qrvmc::bytes32 get_storage(const qrvmc::address& addr,
                              const qrvmc::bytes32& key) const noexcept final
    {
        const auto account_iter = accounts.find(addr);
        if (account_iter == accounts.end())
            return {};

        const auto storage_iter = account_iter->second.storage.find(key);
        if (storage_iter != account_iter->second.storage.end())
            return storage_iter->second;
        return {};
    }

    qrvmc_storage_status set_storage(const qrvmc::address& addr,
                                    const qrvmc::bytes32& key,
                                    const qrvmc::bytes32& value) noexcept final
    {
        auto& account = accounts[addr];
        auto prev_value = account.storage[key];
        account.storage[key] = value;

        return (prev_value == value) ? QRVMC_STORAGE_ASSIGNED : QRVMC_STORAGE_MODIFIED;
    }

    qrvmc::uint256be get_balance(const qrvmc::address& addr) const noexcept final
    {
        auto it = accounts.find(addr);
        if (it != accounts.end())
            return it->second.balance;
        return {};
    }

    size_t get_code_size(const qrvmc::address& addr) const noexcept final
    {
        auto it = accounts.find(addr);
        if (it != accounts.end())
            return it->second.code.size();
        return 0;
    }

    qrvmc::bytes32 get_code_hash(const qrvmc::address& addr) const noexcept final
    {
        auto it = accounts.find(addr);
        if (it != accounts.end())
            return it->second.code_hash();
        return {};
    }

    size_t copy_code(const qrvmc::address& addr,
                     size_t code_offset,
                     uint8_t* buffer_data,
                     size_t buffer_size) const noexcept final
    {
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return 0;

        const auto& code = it->second.code;

        if (code_offset >= code.size())
            return 0;

        const auto n = std::min(buffer_size, code.size() - code_offset);

        if (n > 0)
            std::copy_n(&code[code_offset], n, buffer_data);
        return n;
    }

    qrvmc::Result call(const qrvmc_message& msg) noexcept final
    {
        return qrvmc::Result{QRVMC_REVERT, msg.gas, 0, msg.input_data, msg.input_size};
    }

    qrvmc_tx_context get_tx_context() const noexcept final { return tx_context; }

    // NOLINTNEXTLINE(bugprone-exception-escape)
    qrvmc::bytes32 get_block_hash(int64_t number) const noexcept final
    {
        const int64_t current_block_number = get_tx_context().block_number;

        return (number < current_block_number && number >= current_block_number - 256) ?
                   0xb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5f_bytes32 :
                   0x0000000000000000000000000000000000000000000000000000000000000000_bytes32;
    }

    void emit_log(const qrvmc::address& addr,
                  const uint8_t* data,
                  size_t data_size,
                  const qrvmc::bytes32 topics[],
                  size_t topics_count) noexcept final
    {
        (void)addr;
        (void)data;
        (void)data_size;
        (void)topics;
        (void)topics_count;
    }

    qrvmc_access_status access_account(const qrvmc::address& addr) noexcept final
    {
        (void)addr;
        return QRVMC_ACCESS_COLD;
    }

    qrvmc_access_status access_storage(const qrvmc::address& addr,
                                      const qrvmc::bytes32& key) noexcept final
    {
        (void)addr;
        (void)key;
        return QRVMC_ACCESS_COLD;
    }
};


extern "C" {

const qrvmc_host_interface* example_host_get_interface()
{
    return &qrvmc::Host::get_interface();
}

qrvmc_host_context* example_host_create_context(qrvmc_tx_context tx_context)
{
    auto host = new ExampleHost{tx_context};
    return host->to_context();
}

void example_host_destroy_context(qrvmc_host_context* context)
{
    delete qrvmc::Host::from_context<ExampleHost>(context);
}
}
