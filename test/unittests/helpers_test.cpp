// EVMC: Ethereum Client-VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include <zvmc/helpers.h>

#include <gtest/gtest.h>

// Compile time checks:

static_assert(sizeof(zvmc_bytes32) == 32, "zvmc_bytes32 is too big");
static_assert(sizeof(zvmc_address) == 24, "zvmc_address is too big");
static_assert(sizeof(zvmc_vm) <= 64, "zvmc_vm does not fit cache line");
static_assert(offsetof(zvmc_message, value) % sizeof(size_t) == 0,
              "zvmc_message.value not aligned");

// Check enums match int size.
// On GCC/clang the underlying type should be unsigned int, on MSVC int
static_assert(sizeof(zvmc_call_kind) == sizeof(int),
              "Enum `zvmc_call_kind` is not the size of int");
static_assert(sizeof(zvmc_revision) == sizeof(int), "Enum `zvmc_revision` is not the size of int");

static constexpr size_t optionalDataSize =
    sizeof(zvmc_result) - offsetof(zvmc_result, create_address);
static_assert(optionalDataSize >= sizeof(zvmc_result_optional_storage),
              "zvmc_result's optional data space is too small");

TEST(helpers, release_result)
{
    auto r1 = zvmc_result{};
    zvmc_release_result(&r1);

    static zvmc_result r2;
    static bool e;

    e = false;
    r2 = zvmc_result{};
    r2.release = [](const zvmc_result* r) { e = r == &r2; };
    EXPECT_FALSE(e);
    zvmc_release_result(&r2);
    EXPECT_TRUE(e);
}
