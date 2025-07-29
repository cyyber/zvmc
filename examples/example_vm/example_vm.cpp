// EVMC: Ethereum Client-VM Connector API.
// Copyright 2016 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

/// @file
/// Example implementation of the QRVMC VM interface.
///
/// This VM implements a subset of QRVM instructions in simplistic, incorrect and unsafe way:
/// - memory bounds are not checked,
/// - stack bounds are not checked,
/// - most of the operations are done with 32-bit precision (instead of QRVM 256-bit precision).
/// Yet, it is capable of coping with some example QRVM bytecode inputs, which is very useful
/// in integration testing. The implementation is done in simple C++ for readability and uses
/// pure C API and some C helpers.

#include "example_vm.h"
#include <qrvmc/helpers.h>
#include <qrvmc/instructions.h>
#include <qrvmc/qrvmc.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/// The Example VM methods, helper and types are contained in the anonymous namespace.
/// Technically, this limits the visibility of these elements (internal linkage).
/// This is not strictly required, but is good practice and promotes position independent code.
namespace
{
/// The example VM instance struct extending the qrvmc_vm.
struct ExampleVM : qrvmc_vm
{
    int verbose = 0;  ///< The verbosity level.
    ExampleVM();      ///< Constructor to initialize the qrvmc_vm struct.
};

/// The implementation of the qrvmc_vm::destroy() method.
void destroy(qrvmc_vm* instance)
{
    delete static_cast<ExampleVM*>(instance);
}

/// The example implementation of the qrvmc_vm::get_capabilities() method.
qrvmc_capabilities_flagset get_capabilities(qrvmc_vm* /*instance*/)
{
    return QRVMC_CAPABILITY_QRVM1;
}

/// Example VM options.
///
/// The implementation of the qrvmc_vm::set_option() method.
/// VMs are allowed to omit this method implementation.
enum qrvmc_set_option_result set_option(qrvmc_vm* instance, const char* name, const char* value)
{
    auto* vm = static_cast<ExampleVM*>(instance);
    if (std::strcmp(name, "verbose") == 0)
    {
        if (value == nullptr)
            return QRVMC_SET_OPTION_INVALID_VALUE;

        char* end = nullptr;
        auto v = std::strtol(value, &end, 0);
        if (end == value)  // Parsing the value failed.
            return QRVMC_SET_OPTION_INVALID_VALUE;
        if (v > 9 || v < -1)  // Not in the valid range.
            return QRVMC_SET_OPTION_INVALID_VALUE;
        vm->verbose = static_cast<int>(v);
        return QRVMC_SET_OPTION_SUCCESS;
    }

    return QRVMC_SET_OPTION_INVALID_NAME;
}

/// The Example VM stack representation.
struct Stack
{
    qrvmc_uint256be items[1024] = {};  ///< The array of stack items.
    qrvmc_uint256be* pointer = items;  ///< The pointer to the currently first empty stack slot.

    /// Pops an item from the top of the stack.
    qrvmc_uint256be pop() { return *--pointer; }

    /// Pushes an item to the top of the stack.
    void push(qrvmc_uint256be value) { *pointer++ = value; }
};

/// The Example VM memory representation.
struct Memory
{
    uint32_t size = 0;        ///< The current size of the memory.
    uint8_t data[1024] = {};  ///< The fixed-size memory buffer.

    /// Expands the "active" QRVM memory by the given memory region defined by
    /// @p offset and @p region_size. The region of size 0 also expands the memory
    /// (what is different behavior than QRVM specifies).
    /// Returns pointer to the beginning of the region in the memory,
    /// or nullptr if the memory cannot be expanded to the required size.
    uint8_t* expand(uint32_t offset, uint32_t region_size)
    {
        uint32_t new_size = offset + region_size;
        if (new_size > sizeof(data))
            return nullptr;  // Cannot expand more than fixed max memory size.

        if (new_size > size)
            size = new_size;  // Update current memory size.

        return &data[offset];
    }

    /// Stores the given value bytes in the memory at the given offset.
    /// The Memory::size is updated accordingly.
    /// Returns true if successful, false if the memory cannot be expanded to the required size.
    bool store(uint32_t offset, const uint8_t* value_data, uint32_t value_size)
    {
        uint8_t* p = expand(offset, value_size);
        if (p == nullptr)
            return false;

        std::memcpy(p, value_data, value_size);
        return true;
    }
};

/// Creates 256-bit value out of 32-bit input.
inline qrvmc_uint256be to_uint256(uint32_t x)
{
    qrvmc_uint256be value = {};
    value.bytes[31] = static_cast<uint8_t>(x);
    value.bytes[30] = static_cast<uint8_t>(x >> 8);
    value.bytes[29] = static_cast<uint8_t>(x >> 16);
    value.bytes[28] = static_cast<uint8_t>(x >> 24);
    return value;
}

/// Creates 256-bit value out of an 160-bit address.
inline qrvmc_uint256be to_uint256(qrvmc_address address)
{
    qrvmc_uint256be value = {};
    size_t offset = sizeof(value) - sizeof(address);
    std::memcpy(&value.bytes[offset], address.bytes, sizeof(address.bytes));
    return value;
}

/// Truncates 256-bit value to 32-bit value.
inline uint32_t to_uint32(qrvmc_uint256be value)
{
    return (uint32_t{value.bytes[28]} << 24) | (uint32_t{value.bytes[29]} << 16) |
           (uint32_t{value.bytes[30]} << 8) | (uint32_t{value.bytes[31]});
}

/// Truncates 256-bit value to 160-bit address.
inline qrvmc_address to_address(qrvmc_uint256be value)
{
    qrvmc_address address = {};
    size_t offset = sizeof(value) - sizeof(address);
    std::memcpy(address.bytes, &value.bytes[offset], sizeof(address.bytes));
    return address;
}


/// The example implementation of the qrvmc_vm::execute() method.
qrvmc_result execute(qrvmc_vm* instance,
                     const qrvmc_host_interface* host,
                     qrvmc_host_context* context,
                     enum qrvmc_revision /*rev*/,
                     const qrvmc_message* msg,
                     const uint8_t* code,
                     size_t code_size)
{
    auto* vm = static_cast<ExampleVM*>(instance);

    if (vm->verbose > 0)
        std::puts("execution started\n");

    int64_t gas_left = msg->gas;
    Stack stack;
    Memory memory;

    for (size_t pc = 0; pc < code_size; ++pc)
    {
        // Check remaining gas, assume each instruction costs 1.
        gas_left -= 1;
        if (gas_left < 0)
            return qrvmc_make_result(QRVMC_OUT_OF_GAS, 0, 0, nullptr, 0);

        switch (code[pc])
        {
        default:
            return qrvmc_make_result(QRVMC_UNDEFINED_INSTRUCTION, 0, 0, nullptr, 0);

        case OP_STOP:
            return qrvmc_make_result(QRVMC_SUCCESS, gas_left, 0, nullptr, 0);

        case OP_ADD:
        {
            uint32_t a = to_uint32(stack.pop());
            uint32_t b = to_uint32(stack.pop());
            uint32_t sum = a + b;
            stack.push(to_uint256(sum));
            break;
        }

        case OP_ADDRESS:
        {
            qrvmc_uint256be value = to_uint256(msg->recipient);
            stack.push(value);
            break;
        }

        case OP_CALLDATALOAD:
        {
            uint32_t offset = to_uint32(stack.pop());
            qrvmc_uint256be value = {};

            if (offset < msg->input_size)
            {
                size_t copy_size = std::min(msg->input_size - offset, sizeof(value));
                std::memcpy(value.bytes, &msg->input_data[offset], copy_size);
            }

            stack.push(value);
            break;
        }

        case OP_NUMBER:
        {
            qrvmc_uint256be value =
                to_uint256(static_cast<uint32_t>(host->get_tx_context(context).block_number));
            stack.push(value);
            break;
        }

        case OP_MSTORE:
        {
            uint32_t index = to_uint32(stack.pop());
            qrvmc_uint256be value = stack.pop();
            if (!memory.store(index, value.bytes, sizeof(value)))
                return qrvmc_make_result(QRVMC_FAILURE, 0, 0, nullptr, 0);
            break;
        }

        case OP_SLOAD:
        {
            qrvmc_uint256be index = stack.pop();
            qrvmc_uint256be value = host->get_storage(context, &msg->recipient, &index);
            stack.push(value);
            break;
        }

        case OP_SSTORE:
        {
            qrvmc_uint256be index = stack.pop();
            qrvmc_uint256be value = stack.pop();
            host->set_storage(context, &msg->recipient, &index, &value);
            break;
        }

        case OP_MSIZE:
        {
            qrvmc_uint256be value = to_uint256(memory.size);
            stack.push(value);
            break;
        }

        case OP_PUSH1:
        case OP_PUSH2:
        case OP_PUSH3:
        case OP_PUSH4:
        case OP_PUSH5:
        case OP_PUSH6:
        case OP_PUSH7:
        case OP_PUSH8:
        case OP_PUSH9:
        case OP_PUSH10:
        case OP_PUSH11:
        case OP_PUSH12:
        case OP_PUSH13:
        case OP_PUSH14:
        case OP_PUSH15:
        case OP_PUSH16:
        case OP_PUSH17:
        case OP_PUSH18:
        case OP_PUSH19:
        case OP_PUSH20:
        case OP_PUSH21:
        case OP_PUSH22:
        case OP_PUSH23:
        case OP_PUSH24:
        case OP_PUSH25:
        case OP_PUSH26:
        case OP_PUSH27:
        case OP_PUSH28:
        case OP_PUSH29:
        case OP_PUSH30:
        case OP_PUSH31:
        case OP_PUSH32:
        {
            qrvmc_uint256be value = {};
            size_t num_push_bytes = size_t{code[pc]} - OP_PUSH1 + 1;
            size_t offset = sizeof(value) - num_push_bytes;
            std::memcpy(&value.bytes[offset], &code[pc + 1], num_push_bytes);
            pc += num_push_bytes;
            stack.push(value);
            break;
        }

        case OP_DUP1:
        {
            qrvmc_uint256be value = stack.pop();
            stack.push(value);
            stack.push(value);
            break;
        }

        case OP_CALL:
        {
            qrvmc_message call_msg = {};
            call_msg.gas = to_uint32(stack.pop());
            call_msg.recipient = to_address(stack.pop());
            call_msg.value = stack.pop();

            uint32_t call_input_offset = to_uint32(stack.pop());
            uint32_t call_input_size = to_uint32(stack.pop());
            call_msg.input_data = memory.expand(call_input_offset, call_input_size);
            call_msg.input_size = call_input_size;

            uint32_t call_output_offset = to_uint32(stack.pop());
            uint32_t call_output_size = to_uint32(stack.pop());
            uint8_t* call_output_ptr = memory.expand(call_output_offset, call_output_size);

            if (call_msg.input_data == nullptr || call_output_ptr == nullptr)
                return qrvmc_make_result(QRVMC_FAILURE, 0, 0, nullptr, 0);

            qrvmc_result call_result = host->call(context, &call_msg);

            qrvmc_uint256be value = to_uint256(call_result.status_code == QRVMC_SUCCESS ? 1 : 0);
            stack.push(value);

            if (call_output_size > call_result.output_size)
                call_output_size = static_cast<uint32_t>(call_result.output_size);
            memory.store(call_output_offset, call_result.output_data, call_output_size);

            if (call_result.release != nullptr)
                call_result.release(&call_result);
            break;
        }

        case OP_RETURN:
        {
            uint32_t output_offset = to_uint32(stack.pop());
            uint32_t output_size = to_uint32(stack.pop());
            uint8_t* output_ptr = memory.expand(output_offset, output_size);
            if (output_ptr == nullptr)
                return qrvmc_make_result(QRVMC_FAILURE, 0, 0, nullptr, 0);

            return qrvmc_make_result(QRVMC_SUCCESS, gas_left, 0, output_ptr, output_size);
        }

        case OP_REVERT:
        {
            uint32_t output_offset = to_uint32(stack.pop());
            uint32_t output_size = to_uint32(stack.pop());
            uint8_t* output_ptr = memory.expand(output_offset, output_size);
            if (output_ptr == nullptr)
                return qrvmc_make_result(QRVMC_FAILURE, 0, 0, nullptr, 0);

            return qrvmc_make_result(QRVMC_REVERT, gas_left, 0, output_ptr, output_size);
        }
        }
    }

    return qrvmc_make_result(QRVMC_SUCCESS, gas_left, 0, nullptr, 0);
}


/// @cond internal
#if !defined(PROJECT_VERSION)
/// The dummy project version if not provided by the build system.
#define PROJECT_VERSION "0.0.0"
#endif
/// @endcond

ExampleVM::ExampleVM()
  : qrvmc_vm{QRVMC_ABI_VERSION, "example_vm",       PROJECT_VERSION, ::destroy,
             ::execute,         ::get_capabilities, ::set_option}
{}
}  // namespace

extern "C" qrvmc_vm* qrvmc_create_example_vm()
{
    return new ExampleVM;
}
