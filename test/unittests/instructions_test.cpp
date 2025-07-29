// EVMC: Ethereum Client-VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include <qrvmc/instructions.h>
#include <gtest/gtest.h>

inline bool operator==(const qrvmc_instruction_metrics& a,
                       const qrvmc_instruction_metrics& b) noexcept
{
    return a.gas_cost == b.gas_cost && a.stack_height_required == b.stack_height_required &&
           a.stack_height_change == b.stack_height_change;
}

TEST(instructions, name_gas_cost_equivalence)
{
    for (auto r = int{QRVMC_SHANGHAI}; r <= QRVMC_MAX_REVISION; ++r)
    {
        const auto rev = static_cast<qrvmc_revision>(r);
        const auto names = qrvmc_get_instruction_names_table(rev);
        const auto metrics = qrvmc_get_instruction_metrics_table(rev);

        for (int i = 0; i < 256; ++i)
        {
            auto name = names[i];
            auto gas_cost = metrics[i].gas_cost;

            if (name != nullptr)
                EXPECT_GE(gas_cost, 0);
            else
                EXPECT_EQ(gas_cost, 0);
        }
    }
}

TEST(instructions, shanghai_hard_fork)
{
    const auto s = qrvmc_get_instruction_metrics_table(QRVMC_SHANGHAI);
    // const auto p = qrvmc_get_instruction_metrics_table(QRVMC_PARIS);
    const auto sn = qrvmc_get_instruction_names_table(QRVMC_SHANGHAI);
    // const auto pn = qrvmc_get_instruction_names_table(QRVMC_PARIS);

    // NOTE(rgeraldes24): unused for now
    // for (int op = 0x00; op <= 0xff; ++op)
    // {
    //     if (op == OP_PUSH0)
    //         continue;
    //     EXPECT_EQ(s[op], p[op]) << op;
    //     EXPECT_STREQ(sn[op], pn[op]) << op;
    // }

    EXPECT_EQ(s[OP_EXP].gas_cost, 10);

    EXPECT_EQ(s[OP_REVERT].gas_cost, 0);
    EXPECT_EQ(s[OP_REVERT].stack_height_required, 2);
    EXPECT_EQ(s[OP_REVERT].stack_height_change, -2);
    EXPECT_EQ(sn[OP_REVERT], std::string{"REVERT"});

    EXPECT_EQ(s[OP_RETURNDATACOPY].gas_cost, 3);
    EXPECT_EQ(sn[OP_RETURNDATACOPY], std::string{"RETURNDATACOPY"});

    EXPECT_EQ(s[OP_RETURNDATASIZE].gas_cost, 2);
    EXPECT_EQ(sn[OP_RETURNDATASIZE], std::string{"RETURNDATASIZE"});

    for (auto op : {OP_SHL, OP_SHR, OP_SAR})
    {
        const auto m = s[op];
        EXPECT_EQ(m.gas_cost, 3);
        EXPECT_EQ(m.stack_height_required, 2);
        EXPECT_EQ(m.stack_height_change, -1);
    }

    EXPECT_EQ(s[OP_CREATE2].gas_cost, 32000);
    EXPECT_EQ(s[OP_CREATE2].stack_height_required, 4);
    EXPECT_EQ(s[OP_CREATE2].stack_height_change, -3);
    EXPECT_EQ(sn[OP_CREATE2], std::string{"CREATE2"});

    EXPECT_EQ(s[OP_CHAINID].gas_cost, 2);
    EXPECT_EQ(s[OP_CHAINID].stack_height_required, 0);
    EXPECT_EQ(s[OP_CHAINID].stack_height_change, 1);
    EXPECT_EQ(sn[OP_CHAINID], std::string{"CHAINID"});

    EXPECT_EQ(s[OP_SELFBALANCE].gas_cost, 5);
    EXPECT_EQ(s[OP_SELFBALANCE].stack_height_required, 0);
    EXPECT_EQ(s[OP_SELFBALANCE].stack_height_change, 1);
    EXPECT_EQ(sn[OP_SELFBALANCE], std::string{"SELFBALANCE"});

    // EIP-2929 WARM_STORAGE_READ_COST
    EXPECT_EQ(s[OP_EXTCODESIZE].gas_cost, 100);
    EXPECT_EQ(s[OP_EXTCODECOPY].gas_cost, 100);
    EXPECT_EQ(s[OP_EXTCODEHASH].gas_cost, 100);
    EXPECT_EQ(s[OP_BALANCE].gas_cost, 100);
    EXPECT_EQ(s[OP_CALL].gas_cost, 100);
    EXPECT_EQ(s[OP_DELEGATECALL].gas_cost, 100);
    EXPECT_EQ(s[OP_STATICCALL].gas_cost, 100);
    EXPECT_EQ(s[OP_SLOAD].gas_cost, 100);

    // EIP-3198: BASEFEE opcode
    EXPECT_EQ(s[OP_BASEFEE].gas_cost, 2);
    EXPECT_EQ(s[OP_BASEFEE].stack_height_required, 0);
    EXPECT_EQ(s[OP_BASEFEE].stack_height_change, 1);
    EXPECT_EQ(sn[OP_BASEFEE], std::string{"BASEFEE"});

    EXPECT_EQ(sn[OP_PREVRANDAO], std::string{"PREVRANDAO"});

    // EIP-3855: PUSH0 instruction
    EXPECT_EQ(s[OP_PUSH0].gas_cost, 2);
    EXPECT_EQ(s[OP_PUSH0].stack_height_required, 0);
    EXPECT_EQ(s[OP_PUSH0].stack_height_change, 1);
    EXPECT_EQ(sn[OP_PUSH0], std::string{"PUSH0"});
}
