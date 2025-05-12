// EVMC: Ethereum Client-VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

// The vector is not used here, but including it was causing compilation issues
// previously related to using explicit template argument (SFINAE disabled).
#include <vector>

#include "../../examples/example_precompiles_vm/example_precompiles_vm.h"
#include "../../examples/example_vm/example_vm.h"

#include <zvmc/mocked_host.hpp>
#include <zvmc/zvmc.hpp>
#include <gtest/gtest.h>
#include <array>
#include <cctype>
#include <cstring>
#include <map>
#include <unordered_map>

using namespace zvmc::literals;

class NullHost : public zvmc::Host
{
public:
    bool account_exists(const zvmc::address& /*addr*/) const noexcept final { return false; }

    zvmc::bytes32 get_storage(const zvmc::address& /*addr*/,
                              const zvmc::bytes32& /*key*/) const noexcept final
    {
        return {};
    }

    zvmc_storage_status set_storage(const zvmc::address& /*addr*/,
                                    const zvmc::bytes32& /*key*/,
                                    const zvmc::bytes32& /*value*/) noexcept final
    {
        return {};
    }

    zvmc::uint256be get_balance(const zvmc::address& /*addr*/) const noexcept final { return {}; }

    size_t get_code_size(const zvmc::address& /*addr*/) const noexcept final { return 0; }

    zvmc::bytes32 get_code_hash(const zvmc::address& /*addr*/) const noexcept final { return {}; }

    size_t copy_code(const zvmc::address& /*addr*/,
                     size_t /*code_offset*/,
                     uint8_t* /*buffer_data*/,
                     size_t /*buffer_size*/) const noexcept final
    {
        return 0;
    }

    zvmc::Result call(const zvmc_message& /*msg*/) noexcept final { return zvmc::Result{}; }

    zvmc_tx_context get_tx_context() const noexcept final { return {}; }

    zvmc::bytes32 get_block_hash(int64_t /*block_number*/) const noexcept final { return {}; }

    void emit_log(const zvmc::address& /*addr*/,
                  const uint8_t* /*data*/,
                  size_t /*data_size*/,
                  const zvmc::bytes32 /*topics*/[],
                  size_t /*num_topics*/) noexcept final
    {}

    zvmc_access_status access_account(const zvmc::address& /*addr*/) noexcept final
    {
        return ZVMC_ACCESS_COLD;
    }

    zvmc_access_status access_storage(const zvmc::address& /*addr*/,
                                      const zvmc::bytes32& /*key*/) noexcept final
    {
        return ZVMC_ACCESS_COLD;
    }
};

TEST(cpp, address)
{
    zvmc::address a;
    EXPECT_EQ(std::count(std::begin(a.bytes), std::end(a.bytes), 0), int{sizeof(a)});
    EXPECT_EQ(a, zvmc::address{});
    EXPECT_TRUE(is_zero(a));
    EXPECT_FALSE(a);
    EXPECT_TRUE(!a);

    // TODO(rgeraldes24)
    auto other = zvmc_address{};
    other.bytes[19] = 0xfe;
    a = other;
    EXPECT_TRUE(std::equal(std::begin(a.bytes), std::end(a.bytes), std::begin(other.bytes)));

    a.bytes[0] = 1;
    other = a;
    EXPECT_TRUE(std::equal(std::begin(a.bytes), std::end(a.bytes), std::begin(other.bytes)));
    EXPECT_FALSE(is_zero(a));
    EXPECT_TRUE(a);
    EXPECT_FALSE(!a);
}

TEST(cpp, bytes32)
{
    zvmc::bytes32 b;
    EXPECT_EQ(std::count(std::begin(b.bytes), std::end(b.bytes), 0), int{sizeof(b)});
    EXPECT_EQ(b, zvmc::bytes32{});
    EXPECT_TRUE(is_zero(b));
    EXPECT_FALSE(b);
    EXPECT_TRUE(!b);

    auto other = zvmc_bytes32{};
    other.bytes[31] = 0xfe;
    b = other;
    EXPECT_TRUE(std::equal(std::begin(b.bytes), std::end(b.bytes), std::begin(other.bytes)));

    b.bytes[0] = 1;
    other = b;
    EXPECT_TRUE(std::equal(std::begin(b.bytes), std::end(b.bytes), std::begin(other.bytes)));
    EXPECT_FALSE(is_zero(b));
    EXPECT_TRUE(b);
    EXPECT_FALSE(!b);
}

TEST(cpp, std_hash)
{
    using namespace zvmc::literals;

    static_assert(std::hash<zvmc::address>{}({}) == static_cast<size_t>(0xd94d12186c0f2fb7));
    static_assert(std::hash<zvmc::bytes32>{}({}) == static_cast<size_t>(0x4d25767f9dce13f5));

    EXPECT_EQ(std::hash<zvmc::address>{}({}), static_cast<size_t>(0xd94d12186c0f2fb7));
    EXPECT_EQ(std::hash<zvmc::bytes32>{}({}), static_cast<size_t>(0x4d25767f9dce13f5));

    auto ea = zvmc::address{};
    std::fill_n(ea.bytes, sizeof(ea), uint8_t{0xee});
    EXPECT_EQ(std::hash<zvmc::address>{}(ea), static_cast<size_t>(0x41dc0178e01b7cd9));

    auto eb = zvmc::bytes32{};
    std::fill_n(eb.bytes, sizeof(eb), uint8_t{0xee});
    EXPECT_EQ(std::hash<zvmc::bytes32>{}(eb), static_cast<size_t>(0xbb14e5c56b477375));

    const auto rand_address_1 = "Zaa00bb00cc00dd00ee00ff00110022003300440055006600"_address;
    EXPECT_EQ(std::hash<zvmc::address>{}(rand_address_1), static_cast<size_t>(0x30022347e325524e));

    const auto rand_address_2 = "Z00dd00cc00bb00aa0022001100ff00ee0044003300220011"_address;
    EXPECT_EQ(std::hash<zvmc::address>{}(rand_address_2), static_cast<size_t>(0x17f74b6894b0f6b7));

    const auto rand_bytes32_1 =
        0xbb01bb02bb03bb04bb05bb06bb07bb08bb09bb0abb0bbb0cbb0dbb0ebb0fbb00_bytes32;
    EXPECT_EQ(std::hash<zvmc::bytes32>{}(rand_bytes32_1), static_cast<size_t>(0x4f857586d70f2db9));

    const auto rand_bytes32_2 =
        0x04bb03bb02bb01bb08bb07bb06bb05bb0cbb0bbb0abb09bb00bb0fbb0ebb0dbb_bytes32;
    EXPECT_EQ(std::hash<zvmc::bytes32>{}(rand_bytes32_2), static_cast<size_t>(0x4efee0983bb6c4f5));
}

TEST(cpp, std_maps)
{
    std::map<zvmc::address, bool> addresses;
    addresses[{}] = true;
    ASSERT_EQ(addresses.size(), size_t{1});
    EXPECT_EQ(addresses.begin()->first, zvmc::address{});

    std::unordered_map<zvmc::address, bool> unordered_addresses;
    unordered_addresses.emplace(*addresses.begin());
    addresses.clear();
    ASSERT_EQ(unordered_addresses.size(), size_t{1});
    EXPECT_FALSE(unordered_addresses.begin()->first);

    std::map<zvmc::bytes32, bool> storage;
    storage[{}] = true;
    ASSERT_EQ(storage.size(), size_t{1});
    EXPECT_EQ(storage.begin()->first, zvmc::bytes32{});

    std::unordered_map<zvmc::bytes32, bool> unordered_storage;
    unordered_storage.emplace(*storage.begin());
    storage.clear();
    ASSERT_EQ(unordered_storage.size(), size_t{1});
    EXPECT_FALSE(unordered_storage.begin()->first);
}

enum relation
{
    equal,
    less,
    greater
};

namespace
{
/// Compares x and y using all comparison operators (also with reversed argument order)
/// and validates results against the expected relation: eq: x == y, less: x < y.
template <typename T>
void expect_cmp(const T& x, const T& y, relation expected)
{
    switch (expected)
    {
    case equal:
        EXPECT_TRUE(x == y);
        EXPECT_FALSE(x != y);
        EXPECT_FALSE(x < y);
        EXPECT_TRUE(x <= y);
        EXPECT_FALSE(x > y);
        EXPECT_TRUE(x >= y);

        EXPECT_TRUE(y == x);
        EXPECT_FALSE(y != x);
        EXPECT_FALSE(y < x);
        EXPECT_TRUE(y <= x);
        EXPECT_FALSE(y > x);
        EXPECT_TRUE(y >= x);
        break;
    case less:
        EXPECT_FALSE(x == y);
        EXPECT_TRUE(x != y);
        EXPECT_TRUE(x < y);
        EXPECT_TRUE(x <= y);
        EXPECT_FALSE(x > y);
        EXPECT_FALSE(x >= y);

        EXPECT_FALSE(y == x);
        EXPECT_TRUE(y != x);
        EXPECT_FALSE(y < x);
        EXPECT_FALSE(y <= x);
        EXPECT_TRUE(y > x);
        EXPECT_TRUE(y >= x);
        break;
    case greater:
        EXPECT_FALSE(x == y);
        EXPECT_TRUE(x != y);
        EXPECT_FALSE(x < y);
        EXPECT_FALSE(x <= y);
        EXPECT_TRUE(x > y);
        EXPECT_TRUE(x >= y);

        EXPECT_FALSE(y == x);
        EXPECT_TRUE(y != x);
        EXPECT_TRUE(y < x);
        EXPECT_TRUE(y <= x);
        EXPECT_FALSE(y > x);
        EXPECT_FALSE(y >= x);
        break;
    }
}
}  // namespace

TEST(cpp, address_comparison)
{
    const auto zero = zvmc::address{};
    auto max = zvmc::address{};
    std::fill_n(max.bytes, sizeof(max), uint8_t{0xff});

    auto zero_max = zvmc::address{};
    std::fill_n(zero_max.bytes + 8, sizeof(zero_max) - 8, uint8_t{0xff});
    auto max_zero = zvmc::address{};
    std::fill_n(max_zero.bytes, sizeof(max_zero) - 8, uint8_t{0xff});

    expect_cmp(zero, zero, equal);
    expect_cmp(max, max, equal);
    expect_cmp(zero, max, less);
    expect_cmp(max, zero, greater);
    expect_cmp(zero_max, max_zero, less);
    expect_cmp(max_zero, zero_max, greater);

    for (size_t i = 0; i < sizeof(zvmc::address); ++i)
    {
        auto t = zvmc::address{};
        t.bytes[i] = 1;
        auto u = zvmc::address{};
        u.bytes[i] = 2;
        auto f = zvmc::address{};
        f.bytes[i] = 0xff;

        expect_cmp(zero, t, less);
        expect_cmp(zero, u, less);
        expect_cmp(zero, f, less);

        expect_cmp(t, max, less);
        expect_cmp(u, max, less);
        expect_cmp(f, max, less);

        expect_cmp(t, u, less);
        expect_cmp(t, f, less);
        expect_cmp(u, f, less);

        expect_cmp(t, t, equal);
        expect_cmp(u, u, equal);
        expect_cmp(f, f, equal);
    }
}

TEST(cpp, bytes32_comparison)
{
    const auto zero = zvmc::bytes32{};
    auto max = zvmc::bytes32{};
    std::fill_n(max.bytes, sizeof(max), uint8_t{0xff});
    auto z_max = zvmc::bytes32{};
    std::fill_n(z_max.bytes + 8, sizeof(max) - 8, uint8_t{0xff});
    auto max_z = zvmc::bytes32{};
    std::fill_n(max_z.bytes, sizeof(max) - 8, uint8_t{0xff});

    expect_cmp(zero, zero, equal);
    expect_cmp(max, max, equal);
    expect_cmp(zero, max, less);
    expect_cmp(max, zero, greater);
    expect_cmp(z_max, max_z, less);
    expect_cmp(max_z, z_max, greater);

    for (size_t i = 0; i < sizeof(zvmc::bytes32); ++i)
    {
        auto t = zvmc::bytes32{};
        t.bytes[i] = 1;
        auto u = zvmc::bytes32{};
        u.bytes[i] = 2;
        auto f = zvmc::bytes32{};
        f.bytes[i] = 0xff;

        expect_cmp(zero, t, less);
        expect_cmp(zero, u, less);
        expect_cmp(zero, f, less);

        expect_cmp(t, max, less);
        expect_cmp(u, max, less);
        expect_cmp(f, max, less);

        expect_cmp(t, u, less);
        expect_cmp(t, f, less);
        expect_cmp(u, f, less);

        expect_cmp(t, t, equal);
        expect_cmp(u, u, equal);
        expect_cmp(f, f, equal);
    }
}

TEST(cpp, literals)
{
    using namespace zvmc::literals;

    constexpr auto address1 = "Za0a1a2a3a4a5a6a7a8a9d0d1d2d3d4d5d6d7d8d9g0g1g2g3"_address;
    constexpr auto hash1 =
        0x01020304050607080910a1a2a3a4a5a6a7a8a9b0c1c2c3c4c5c6c7c8c9d0d1d2_bytes32;
    constexpr auto zero_address = "Z000000000000000000000000000000000000000000000000"_address;
    constexpr auto zero_hash =
        0x0000000000000000000000000000000000000000000000000000000000000000_bytes32;

    static_assert(address1.bytes[0] == 0xa0);
    static_assert(address1.bytes[9] == 0xa9);
    static_assert(address1.bytes[10] == 0xd0);
    static_assert(address1.bytes[19] == 0xd9);
    static_assert(hash1.bytes[0] == 0x01);
    static_assert(hash1.bytes[10] == 0xa1);
    static_assert(hash1.bytes[31] == 0xd2);
    static_assert(zero_address == zvmc::address{});
    static_assert(zero_hash == zvmc::bytes32{});

    static_assert("Z00"_address == "Z000000000000000000000000000000000000000000000000"_address);
    static_assert("Z01"_address == "Z000000000000000000000000000000000000000000000001"_address);
    static_assert("Zf101"_address == "Z00000000000000000000000000000000000000000000f101"_address);

    EXPECT_EQ("Z000000000000000000000000000000000000000000000000"_address, zvmc::address{});
    EXPECT_EQ(0x0000000000000000000000000000000000000000000000000000000000000000_bytes32,
              zvmc::bytes32{});

    auto a1 = "Za0a1a2a3a4a5a6a7a8a9d0d1d2d3d4d5d6d7d8d9g0g1g2g3"_address;
    const zvmc::address e1{{{0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
                             0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9}}};
    EXPECT_EQ(a1, e1);

    auto h1 = 0x01020304050607080910a1a2a3a4a5a6a7a8a9b0c1c2c3c4c5c6c7c8c9d0d1d2_bytes32;
    const zvmc::bytes32 f1{{{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0xa1,
                             0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xb0, 0xc1, 0xc2,
                             0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd0, 0xd1, 0xd2}}};
    EXPECT_EQ(h1, f1);
}

TEST(cpp, bytes32_from_uint)
{
    using zvmc::bytes32;
    using zvmc::operator""_bytes32;

    static_assert(bytes32{0} == bytes32{});
    static_assert(bytes32{3}.bytes[31] == 3);
    static_assert(bytes32{0xfe00000000000000}.bytes[24] == 0xfe);

    EXPECT_EQ(bytes32{0}, bytes32{});
    EXPECT_EQ(bytes32{0x01},
              0x0000000000000000000000000000000000000000000000000000000000000001_bytes32);
    EXPECT_EQ(bytes32{0xff},
              0x00000000000000000000000000000000000000000000000000000000000000ff_bytes32);
    EXPECT_EQ(bytes32{0x500},
              0x0000000000000000000000000000000000000000000000000000000000000500_bytes32);
    EXPECT_EQ(bytes32{0x8000000000000000},
              0x0000000000000000000000000000000000000000000000008000000000000000_bytes32);
    EXPECT_EQ(bytes32{0xc1c2c3c4c5c6c7c8},
              0x000000000000000000000000000000000000000000000000c1c2c3c4c5c6c7c8_bytes32);
}

TEST(cpp, address_from_uint)
{
    using zvmc::address;
    using zvmc::operator""_address;

    static_assert(address{0} == address{});
    static_assert(address{3}.bytes[19] == 3);
    static_assert(address{0xfe00000000000000}.bytes[12] == 0xfe);

    EXPECT_EQ(address{0}, address{});
    EXPECT_EQ(address{0x01}, "Z000000000000000000000000000000000000000000000001"_address);
    EXPECT_EQ(address{0xff}, "Z0000000000000000000000000000000000000000000000ff"_address);
    EXPECT_EQ(address{0x500}, "Z000000000000000000000000000000000000000000000500"_address);
    EXPECT_EQ(address{0x8000000000000000}, "Z000000000000000000000000000000008000000000000000"_address);
    EXPECT_EQ(address{0xc1c2c3c4c5c6c7c8}, "Z00000000000000000000000000000000c1c2c3c4c5c6c7c8"_address);
}

TEST(cpp, address_to_bytes_view)
{
    using zvmc::operator""_address;

    constexpr auto a = "Za0a1a2a3a4a5a6a7a8a9b0b1b2b3b4b5b6b7b8b9c0c1c2c3"_address;
    static_assert(static_cast<zvmc::bytes_view>(a).size() == 20);
    const zvmc::bytes_view v = a;
    EXPECT_EQ(v, (zvmc::bytes{0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
                              0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9}));
}

TEST(cpp, bytes32_to_bytes_view)
{
    using zvmc::operator""_bytes32;

    constexpr auto b = 0xa0a1a2a3a4a5a6a7a8a9b0b1b2b3b4b5b6b7b8b9c0c1c2c3c4c5c6c7c8c9d0d1_bytes32;
    static_assert(static_cast<zvmc::bytes_view>(b).size() == 32);
    const zvmc::bytes_view v = b;
    EXPECT_EQ(v, (zvmc::bytes{0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xb0,
                              0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xc0, 0xc1,
                              0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd0, 0xd1}));
}

TEST(cpp, result)
{
    static const uint8_t output = 0;
    int release_called = 0;
    {
        auto raw_result = zvmc_result{};
        zvmc_get_optional_storage(&raw_result)->pointer = &release_called;
        EXPECT_EQ(release_called, 0);

        raw_result.output_data = &output;
        raw_result.release = [](const zvmc_result* r) {
            EXPECT_EQ(r->output_data, &output);
            ++*static_cast<int*>(zvmc_get_const_optional_storage(r)->pointer);
        };
        EXPECT_EQ(release_called, 0);

        auto res1 = zvmc::Result{raw_result};
        auto res2 = std::move(res1);
        EXPECT_EQ(release_called, 0);

        auto f = [](zvmc::Result r) { EXPECT_EQ(r.output_data, &output); };
        f(std::move(res2));

        EXPECT_EQ(release_called, 1);
    }
    EXPECT_EQ(release_called, 1);
}

TEST(cpp, vm)
{
    auto vm = zvmc::VM{zvmc_create_example_vm()};
    EXPECT_TRUE(vm.is_abi_compatible());

    auto r = vm.set_option("verbose", "3");
    EXPECT_EQ(r, ZVMC_SET_OPTION_SUCCESS);

    EXPECT_EQ(vm.name(), std::string{"example_vm"});
    EXPECT_NE(vm.version()[0], 0);

    const auto host = zvmc_host_interface{};
    auto msg = zvmc_message{};
    msg.gas = 1;
    auto res = vm.execute(host, nullptr, ZVMC_MAX_REVISION, msg, nullptr, 0);
    EXPECT_EQ(res.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(res.gas_left, 1);
}

TEST(cpp, vm_capabilities)
{
    const auto vm = zvmc::VM{zvmc_create_example_vm()};

    EXPECT_TRUE(vm.get_capabilities() & ZVMC_CAPABILITY_ZVM1);
    EXPECT_FALSE(vm.get_capabilities() & ZVMC_CAPABILITY_ZWASM);
    EXPECT_FALSE(vm.get_capabilities() & ZVMC_CAPABILITY_PRECOMPILES);
    EXPECT_TRUE(vm.has_capability(ZVMC_CAPABILITY_ZVM1));
    EXPECT_FALSE(vm.has_capability(ZVMC_CAPABILITY_ZWASM));
    EXPECT_FALSE(vm.has_capability(ZVMC_CAPABILITY_PRECOMPILES));
}

TEST(cpp, vm_set_option)
{
    zvmc_vm raw = {ZVMC_ABI_VERSION, "", "", nullptr, nullptr, nullptr, nullptr};
    raw.destroy = [](zvmc_vm*) {};

    auto vm = zvmc::VM{&raw};
    EXPECT_EQ(vm.get_raw_pointer(), &raw);
    EXPECT_EQ(vm.set_option("1", "2"), ZVMC_SET_OPTION_INVALID_NAME);
}

TEST(cpp, vm_set_option_in_constructor)
{
    static int num_calls = 0;
    const auto set_option_method = [](zvmc_vm*, const char* name, const char* value) {
        ++num_calls;
        EXPECT_STREQ(name, "o");
        EXPECT_EQ(value, std::to_string(num_calls));
        return ZVMC_SET_OPTION_INVALID_NAME;
    };

    zvmc_vm raw{ZVMC_ABI_VERSION, "", "", nullptr, nullptr, nullptr, set_option_method};
    raw.destroy = [](zvmc_vm*) {};

    const auto vm = zvmc::VM{&raw, {{"o", "1"}, {"o", "2"}}};
    EXPECT_EQ(num_calls, 2);
}

TEST(cpp, vm_null)
{
    const zvmc::VM vm;
    EXPECT_FALSE(vm);
    EXPECT_TRUE(!vm);
    EXPECT_EQ(vm.get_raw_pointer(), nullptr);
}

TEST(cpp, vm_move)
{
    static int destroy_counter = 0;
    const auto template_vm = zvmc_vm{
        ZVMC_ABI_VERSION, "", "", [](zvmc_vm*) { ++destroy_counter; }, nullptr, nullptr, nullptr};

    EXPECT_EQ(destroy_counter, 0);
    {
        auto v1 = template_vm;
        auto v2 = template_vm;

        auto vm1 = zvmc::VM{&v1};
        EXPECT_TRUE(vm1);
        vm1 = zvmc::VM{&v2};
        EXPECT_TRUE(vm1);
    }
    EXPECT_EQ(destroy_counter, 2);
    {
        auto v1 = template_vm;

        auto vm1 = zvmc::VM{&v1};
        EXPECT_TRUE(vm1);
        vm1 = zvmc::VM{};
        EXPECT_FALSE(vm1);
    }
    EXPECT_EQ(destroy_counter, 3);
    {
        auto v1 = template_vm;

        auto vm1 = zvmc::VM{&v1};
        EXPECT_TRUE(vm1);
        auto vm2 = std::move(vm1);
        EXPECT_TRUE(vm2);
        EXPECT_FALSE(vm1);                          // NOLINT
        EXPECT_EQ(vm1.get_raw_pointer(), nullptr);  // NOLINT
        auto vm3 = std::move(vm2);
        EXPECT_TRUE(vm3);
        EXPECT_FALSE(vm2);                          // NOLINT
        EXPECT_EQ(vm2.get_raw_pointer(), nullptr);  // NOLINT
        EXPECT_FALSE(vm1);
        EXPECT_EQ(vm1.get_raw_pointer(), nullptr);
    }
    EXPECT_EQ(destroy_counter, 4);
    {
        // Moving to itself will destroy the VM and reset the zvmc::vm.
        auto v1 = template_vm;

        auto vm1 = zvmc::VM{&v1};
        auto& vm1_ref = vm1;
        vm1 = std::move(vm1_ref);
        EXPECT_EQ(destroy_counter, 5);  // Already destroyed.
        EXPECT_FALSE(vm1);              // Null.
    }
    EXPECT_EQ(destroy_counter, 5);
}

TEST(cpp, vm_execute_precompiles)
{
    auto vm = zvmc::VM{zvmc_create_example_precompiles_vm()};
    EXPECT_EQ(vm.get_capabilities(), zvmc_capabilities_flagset{ZVMC_CAPABILITY_PRECOMPILES});

    constexpr std::array<uint8_t, 3> input{{1, 2, 3}};

    zvmc_message msg{};
    msg.code_address.bytes[19] = 4;  // Call Identify precompile at address 0x4.
    msg.input_data = input.data();
    msg.input_size = input.size();
    msg.gas = 18;

    auto res = vm.execute(ZVMC_MAX_REVISION, msg, nullptr, 0);
    EXPECT_EQ(res.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(res.gas_left, 0);
    ASSERT_EQ(res.output_size, input.size());
    EXPECT_TRUE(std::equal(input.begin(), input.end(), res.output_data));
}

TEST(cpp, vm_execute_with_null_host)
{
    // This tests only if the used VM::execute() overload is at least implemented.
    // We know that the example VM will not use the host context in this case.

    auto host = NullHost{};

    auto vm = zvmc::VM{zvmc_create_example_vm()};
    const zvmc_message msg{};
    auto res = vm.execute(host, ZVMC_SHANGHAI, msg, nullptr, 0);
    EXPECT_EQ(res.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(res.gas_left, 0);
}

TEST(cpp, host)
{
    // Use MockedHost to execute all methods from the C++ host wrapper.
    zvmc::MockedHost mockedHost;
    const auto& host_interface = zvmc::MockedHost::get_interface();
    auto* host_context = mockedHost.to_context();

    auto host = zvmc::HostContext{host_interface, host_context};

    const auto a = zvmc::address{{{1}}};
    const auto v = zvmc::bytes32{{{7, 7, 7}}};

    EXPECT_FALSE(host.account_exists(a));

    mockedHost.accounts[a].storage[{}] = {0x01_bytes32};
    EXPECT_TRUE(host.account_exists(a));

    EXPECT_EQ(host.set_storage(a, {}, v), ZVMC_STORAGE_MODIFIED);
    EXPECT_EQ(host.set_storage(a, {}, v), ZVMC_STORAGE_ASSIGNED);
    EXPECT_EQ(host.get_storage(a, {}), v);

    EXPECT_TRUE(zvmc::is_zero(host.get_balance(a)));

    EXPECT_EQ(host.get_code_size(a), size_t{0});
    EXPECT_EQ(host.get_code_hash(a), zvmc::bytes32{});
    EXPECT_EQ(host.copy_code(a, 0, nullptr, 0), size_t{0});

    auto tx = host.get_tx_context();
    EXPECT_EQ(host.get_tx_context().block_number, tx.block_number);

    EXPECT_EQ(host.get_block_hash(0), zvmc::bytes32{});

    host.emit_log(a, nullptr, 0, nullptr, 0);
}

TEST(cpp, host_call)
{
    // Use example host to test Host::call() method.
    zvmc::MockedHost mockedHost;
    const auto& host_interface = zvmc::MockedHost::get_interface();
    auto* host_context = mockedHost.to_context();

    auto host = zvmc::HostContext{};  // Use default constructor.
    host = zvmc::HostContext{host_interface, host_context};

    EXPECT_EQ(host.call({}).gas_left, 0);
    ASSERT_EQ(mockedHost.recorded_calls.size(), 1u);
    const auto& recorded_msg1 = mockedHost.recorded_calls.back();
    EXPECT_EQ(recorded_msg1.kind, ZVMC_CALL);
    EXPECT_EQ(recorded_msg1.gas, 0);
    EXPECT_EQ(recorded_msg1.flags, 0u);
    EXPECT_EQ(recorded_msg1.depth, 0);
    EXPECT_EQ(recorded_msg1.input_data, nullptr);
    EXPECT_EQ(recorded_msg1.input_size, 0u);

    auto msg = zvmc_message{};
    msg.gas = 1;
    zvmc::bytes input{0xa, 0xb, 0xc};
    msg.input_data = input.data();
    msg.input_size = input.size();

    mockedHost.call_result.status_code = ZVMC_REVERT;
    mockedHost.call_result.gas_left = 4321;
    mockedHost.call_result.output_data = &input[2];
    mockedHost.call_result.output_size = 1;

    auto res = host.call(msg);
    ASSERT_EQ(mockedHost.recorded_calls.size(), 2u);
    const auto& recorded_msg2 = mockedHost.recorded_calls.back();
    EXPECT_EQ(recorded_msg2.kind, ZVMC_CALL);
    EXPECT_EQ(recorded_msg2.gas, 1);
    EXPECT_EQ(recorded_msg2.flags, 0u);
    EXPECT_EQ(recorded_msg2.depth, 0);
    ASSERT_EQ(recorded_msg2.input_size, 3u);
    EXPECT_EQ(zvmc::bytes(recorded_msg2.input_data, recorded_msg2.input_size), input);

    EXPECT_EQ(res.status_code, ZVMC_REVERT);
    EXPECT_EQ(res.gas_left, 4321);
    ASSERT_EQ(res.output_size, 1u);
    EXPECT_EQ(*res.output_data, input[2]);
}

TEST(cpp, result_raii)
{
    static auto release_called = 0;
    release_called = 0;
    auto release_fn = [](const zvmc_result*) noexcept { ++release_called; };

    {
        auto raw_result = zvmc_result{};
        raw_result.status_code = ZVMC_INTERNAL_ERROR;
        raw_result.release = release_fn;

        auto raii_result = zvmc::Result{raw_result};
        EXPECT_EQ(raii_result.status_code, ZVMC_INTERNAL_ERROR);
        EXPECT_EQ(raii_result.gas_left, 0);
        raii_result.gas_left = -1;

        auto raw_result2 = raii_result.release_raw();
        EXPECT_EQ(raw_result2.status_code, ZVMC_INTERNAL_ERROR);
        EXPECT_EQ(raw_result.status_code, ZVMC_INTERNAL_ERROR);
        EXPECT_EQ(raw_result2.gas_left, -1);
        EXPECT_EQ(raw_result.gas_left, 0);
        EXPECT_EQ(raw_result2.release, release_fn);
        EXPECT_EQ(raw_result.release, release_fn);
    }
    EXPECT_EQ(release_called, 0);

    {
        auto raw_result = zvmc_result{};
        raw_result.status_code = ZVMC_INTERNAL_ERROR;
        raw_result.release = release_fn;

        auto raii_result = zvmc::Result{raw_result};
        EXPECT_EQ(raii_result.status_code, ZVMC_INTERNAL_ERROR);
    }
    EXPECT_EQ(release_called, 1);
}

TEST(cpp, result_move)
{
    static auto release_called = 0;
    auto release_fn = [](const zvmc_result*) noexcept { ++release_called; };

    release_called = 0;
    {
        auto raw = zvmc_result{};
        raw.gas_left = -1;
        raw.release = release_fn;

        auto r0 = zvmc::Result{raw};
        EXPECT_EQ(r0.gas_left, raw.gas_left);

        auto r1 = std::move(r0);
        EXPECT_EQ(r1.gas_left, raw.gas_left);
    }
    EXPECT_EQ(release_called, 1);

    release_called = 0;
    {
        auto raw1 = zvmc_result{};
        raw1.gas_left = 1;
        raw1.release = release_fn;

        auto raw2 = zvmc_result{};
        raw2.gas_left = 1;
        raw2.release = release_fn;

        auto r1 = zvmc::Result{raw1};
        auto r2 = zvmc::Result{raw2};

        r2 = std::move(r1);
    }
    EXPECT_EQ(release_called, 2);
}

TEST(cpp, result_create_no_output)
{
    auto r = zvmc::Result{ZVMC_REVERT, 1};
    EXPECT_EQ(r.status_code, ZVMC_REVERT);
    EXPECT_EQ(r.gas_left, 1);
    EXPECT_FALSE(r.output_data);
    EXPECT_EQ(r.output_size, size_t{0});
}

TEST(cpp, result_create)
{
    const uint8_t output[] = {1, 2};
    auto r = zvmc::Result{ZVMC_FAILURE, -1, -2, output, sizeof(output)};
    EXPECT_EQ(r.status_code, ZVMC_FAILURE);
    EXPECT_EQ(r.gas_left, -1);
    EXPECT_EQ(r.gas_refund, -2);
    ASSERT_TRUE(r.output_data);
    ASSERT_EQ(r.output_size, size_t{2});
    EXPECT_EQ(r.output_data[0], 1);
    EXPECT_EQ(r.output_data[1], 2);

    auto c =
        zvmc::make_result(r.status_code, r.gas_left, r.gas_refund, r.output_data, r.output_size);
    EXPECT_EQ(c.status_code, r.status_code);
    EXPECT_EQ(c.gas_left, r.gas_left);
    ASSERT_EQ(c.output_size, r.output_size);
    EXPECT_EQ(zvmc::address{c.create_address}, zvmc::address{r.create_address});
    ASSERT_TRUE(c.release);
    EXPECT_TRUE(std::memcmp(c.output_data, r.output_data, c.output_size) == 0);
    c.release(&c);
}

TEST(cpp, status_code_to_string)
{
    struct TestCase
    {
        zvmc_status_code status_code;
        std::string_view str;
    };

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TEST_CASE(NAME) \
    TestCase            \
    {                   \
        NAME, #NAME     \
    }
    constexpr TestCase test_cases[]{
        TEST_CASE(ZVMC_SUCCESS),
        TEST_CASE(ZVMC_FAILURE),
        TEST_CASE(ZVMC_REVERT),
        TEST_CASE(ZVMC_OUT_OF_GAS),
        TEST_CASE(ZVMC_INVALID_INSTRUCTION),
        TEST_CASE(ZVMC_UNDEFINED_INSTRUCTION),
        TEST_CASE(ZVMC_STACK_OVERFLOW),
        TEST_CASE(ZVMC_STACK_UNDERFLOW),
        TEST_CASE(ZVMC_BAD_JUMP_DESTINATION),
        TEST_CASE(ZVMC_INVALID_MEMORY_ACCESS),
        TEST_CASE(ZVMC_CALL_DEPTH_EXCEEDED),
        TEST_CASE(ZVMC_STATIC_MODE_VIOLATION),
        TEST_CASE(ZVMC_PRECOMPILE_FAILURE),
        TEST_CASE(ZVMC_CONTRACT_VALIDATION_FAILURE),
        TEST_CASE(ZVMC_ARGUMENT_OUT_OF_RANGE),
        TEST_CASE(ZVMC_WASM_UNREACHABLE_INSTRUCTION),
        TEST_CASE(ZVMC_WASM_TRAP),
        TEST_CASE(ZVMC_INSUFFICIENT_BALANCE),
        TEST_CASE(ZVMC_INTERNAL_ERROR),
        TEST_CASE(ZVMC_REJECTED),
        TEST_CASE(ZVMC_OUT_OF_MEMORY),
    };
#undef TEST_CASE

    std::ostringstream os;
    for (const auto& t : test_cases)
    {
        std::string expected;
        std::transform(std::cbegin(t.str) + std::strlen("ZVMC_"), std::cend(t.str),
                       std::back_inserter(expected), [](char c) -> char {
                           return (c == '_') ? ' ' : static_cast<char>(std::tolower(c));
                       });
        EXPECT_EQ(zvmc::to_string(t.status_code), expected);
        os << t.status_code;
        EXPECT_EQ(os.str(), expected);
        os.str({});
    }
}

TEST(cpp, revision_to_string)
{
    struct TestCase
    {
        zvmc_revision rev;
        std::string_view str;
    };

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TEST_CASE(NAME) \
    TestCase            \
    {                   \
        NAME, #NAME     \
    }
    constexpr TestCase test_cases[]{
        TEST_CASE(ZVMC_SHANGHAI),
    };
#undef TEST_CASE

    // ASSERT_EQ(std::size(test_cases), size_t{ZVMC_MAX_REVISION + 1});
    std::ostringstream os;
    // for (size_t i = 0; i < std::size(test_cases); ++i)
    for (const auto& t : test_cases)
    {
        // const auto& t = test_cases[i];
        // EXPECT_EQ(t.rev, static_cast<int>(i));
        std::string expected;
        std::transform(std::cbegin(t.str) + std::strlen("ZVMC_"), std::cend(t.str),
                       std::back_inserter(expected), [skip = true](char c) mutable -> char {
                           if (skip)
                           {
                               skip = false;
                               return c;
                           }
                           else if (c == '_')
                           {
                               skip = true;
                               return ' ';
                           }
                           else
                               return static_cast<char>(std::tolower(c));
                       });
        EXPECT_EQ(zvmc::to_string(t.rev), expected);
        os << t.rev;
        EXPECT_EQ(os.str(), expected);
        os.str({});
    }
}


#if defined(__GNUC__) && !defined(__APPLE__)
extern "C" [[gnu::weak]] void __ubsan_handle_builtin_unreachable(void*);  // NOLINT
#endif

static bool has_ubsan() noexcept  // NOLINT(misc-use-anonymous-namespace)
{
#if defined(__GNUC__) && !defined(__APPLE__)
    return (__ubsan_handle_builtin_unreachable != nullptr);
#else
    return false;
#endif
}

TEST(cpp, status_code_to_string_invalid)
{
    if (!has_ubsan())
    {
        std::ostringstream os;
        int value = 99;  // NOLINT(misc-const-correctness) Not const because GCC complains.
        const auto invalid = static_cast<zvmc_status_code>(value);
        EXPECT_STREQ(zvmc::to_string(invalid), "<unknown>");
        os << invalid;
        EXPECT_EQ(os.str(), "<unknown>");
    }
}

TEST(cpp, revision_to_string_invalid)
{
    if (!has_ubsan())
    {
        std::ostringstream os;
        int value = 99;  // NOLINT(misc-const-correctness) Not const because GCC complains.
        const auto invalid = static_cast<zvmc_revision>(value);
        EXPECT_STREQ(zvmc::to_string(invalid), "<unknown>");
        os << invalid;
        EXPECT_EQ(os.str(), "<unknown>");
    }
}

TEST(cpp, result_c_const_access)
{
    static constexpr auto get_status = [](const zvmc_result& c_result) noexcept {
        return c_result.status_code;
    };

    const zvmc::Result r{ZVMC_REVERT};
    EXPECT_EQ(get_status(r.raw()), ZVMC_REVERT);
}

TEST(cpp, result_c_nonconst_access)
{
    static constexpr auto set_status = [](zvmc_result& c_result) noexcept {
        c_result.status_code = ZVMC_SUCCESS;
    };

    zvmc::Result r;
    EXPECT_EQ(r.status_code, ZVMC_INTERNAL_ERROR);
    set_status(r.raw());
    EXPECT_EQ(r.status_code, ZVMC_SUCCESS);
}
