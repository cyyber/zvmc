// EVMC: Ethereum Client-VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

package zvmc

/*
#cgo CFLAGS: -I${SRCDIR}/../../../include -Wall -Wextra
#cgo !windows LDFLAGS: -ldl

#include <zvmc/zvmc.h>
#include <zvmc/helpers.h>
#include <zvmc/loader.h>

#include <stdlib.h>
#include <string.h>

static inline enum zvmc_set_option_result set_option(struct zvmc_vm* vm, char* name, char* value)
{
	enum zvmc_set_option_result ret = zvmc_set_option(vm, name, value);
	free(name);
	free(value);
	return ret;
}

extern const struct zvmc_host_interface zvmc_go_host;

static struct zvmc_result execute_wrapper(struct zvmc_vm* vm,
	uintptr_t context_index, enum zvmc_revision rev,
	enum zvmc_call_kind kind, uint32_t flags, int32_t depth, int64_t gas,
	const zvmc_address* recipient, const zvmc_address* sender,
	const uint8_t* input_data, size_t input_size, const zvmc_uint256be* value,
	const uint8_t* code, size_t code_size)
{
	struct zvmc_message msg = {
		kind,
		flags,
		depth,
		gas,
		*recipient,
		*sender,
		input_data,
		input_size,
		*value,
		{{0}}, // create2_salt: not required for execution
		{{0}}, // code_address: not required for execution
	};

	struct zvmc_host_context* context = (struct zvmc_host_context*)context_index;
	return zvmc_execute(vm, &zvmc_go_host, context, rev, &msg, code, code_size);
}
*/
import "C"

import (
	"fmt"
	"sync"
	"unsafe"
)

// Hash represents the 32 bytes of arbitrary data (e.g. the result of Keccak256
// hash). It occasionally is used to represent 256-bit unsigned integer values
// stored in big-endian byte order.
type Hash [32]byte

// Address represents the 192-bit (24 bytes) address of a Zond account.
type Address [24]byte

// Static asserts.
const (
	// The size of zvmc_bytes32 equals the size of Hash.
	_ = uint(len(Hash{}) - C.sizeof_zvmc_bytes32)
	_ = uint(C.sizeof_zvmc_bytes32 - len(Hash{}))

	// The size of zvmc_address equals the size of Address.
	_ = uint(len(Address{}) - C.sizeof_zvmc_address)
	_ = uint(C.sizeof_zvmc_address - len(Address{}))
)

type Error int32

func (err Error) IsInternalError() bool {
	return err < 0
}

func (err Error) Error() string {
	return C.GoString(C.zvmc_status_code_to_string(C.enum_zvmc_status_code(err)))
}

const (
	Failure = Error(C.ZVMC_FAILURE)
	Revert  = Error(C.ZVMC_REVERT)
)

type Revision int32

const (
	Shanghai             Revision = C.ZVMC_SHANGHAI
	MaxRevision          Revision = C.ZVMC_MAX_REVISION
	LatestStableRevision Revision = C.ZVMC_LATEST_STABLE_REVISION
)

type VM struct {
	handle *C.struct_zvmc_vm
}

func Load(filename string) (vm *VM, err error) {
	cfilename := C.CString(filename)
	loaderErr := C.enum_zvmc_loader_error_code(C.ZVMC_LOADER_UNSPECIFIED_ERROR)
	handle := C.zvmc_load_and_create(cfilename, &loaderErr)
	C.free(unsafe.Pointer(cfilename))

	if loaderErr == C.ZVMC_LOADER_SUCCESS {
		vm = &VM{handle}
	} else {
		errMsg := C.zvmc_last_error_msg()
		if errMsg != nil {
			err = fmt.Errorf("ZVMC loading error: %s", C.GoString(errMsg))
		} else {
			err = fmt.Errorf("ZVMC loading error %d", int(loaderErr))
		}
	}

	return vm, err
}

func LoadAndConfigure(config string) (vm *VM, err error) {
	cconfig := C.CString(config)
	loaderErr := C.enum_zvmc_loader_error_code(C.ZVMC_LOADER_UNSPECIFIED_ERROR)
	handle := C.zvmc_load_and_configure(cconfig, &loaderErr)
	C.free(unsafe.Pointer(cconfig))

	if loaderErr == C.ZVMC_LOADER_SUCCESS {
		vm = &VM{handle}
	} else {
		errMsg := C.zvmc_last_error_msg()
		if errMsg != nil {
			err = fmt.Errorf("ZVMC loading error: %s", C.GoString(errMsg))
		} else {
			err = fmt.Errorf("ZVMC loading error %d", int(loaderErr))
		}
	}

	return vm, err
}

func (vm *VM) Destroy() {
	C.zvmc_destroy(vm.handle)
}

func (vm *VM) Name() string {
	// TODO: consider using C.zvmc_vm_name(vm.handle)
	return C.GoString(vm.handle.name)
}

func (vm *VM) Version() string {
	// TODO: consider using C.zvmc_vm_version(vm.handle)
	return C.GoString(vm.handle.version)
}

type Capability uint32

const (
	CapabilityZVM1  Capability = C.ZVMC_CAPABILITY_ZVM1
	CapabilityZWASM Capability = C.ZVMC_CAPABILITY_ZWASM
)

func (vm *VM) HasCapability(capability Capability) bool {
	return bool(C.zvmc_vm_has_capability(vm.handle, uint32(capability)))
}

func (vm *VM) SetOption(name string, value string) (err error) {

	r := C.set_option(vm.handle, C.CString(name), C.CString(value))
	switch r {
	case C.ZVMC_SET_OPTION_INVALID_NAME:
		err = fmt.Errorf("zvmc: option '%s' not accepted", name)
	case C.ZVMC_SET_OPTION_INVALID_VALUE:
		err = fmt.Errorf("zvmc: option '%s' has invalid value", name)
	case C.ZVMC_SET_OPTION_SUCCESS:
	}
	return err
}

func (vm *VM) Execute(ctx HostContext, rev Revision,
	kind CallKind, static bool, depth int, gas int64,
	recipient Address, sender Address, input []byte, value Hash,
	code []byte) (output []byte, gasLeft int64, err error) {

	flags := C.uint32_t(0)
	if static {
		flags |= C.ZVMC_STATIC
	}

	ctxId := addHostContext(ctx)
	// FIXME: Clarify passing by pointer vs passing by value.
	zvmcRecipient := zvmcAddress(recipient)
	zvmcSender := zvmcAddress(sender)
	zvmcValue := zvmcBytes32(value)
	result := C.execute_wrapper(vm.handle, C.uintptr_t(ctxId), uint32(rev),
		C.enum_zvmc_call_kind(kind), flags, C.int32_t(depth), C.int64_t(gas),
		&zvmcRecipient, &zvmcSender, bytesPtr(input), C.size_t(len(input)), &zvmcValue,
		bytesPtr(code), C.size_t(len(code)))
	removeHostContext(ctxId)

	output = C.GoBytes(unsafe.Pointer(result.output_data), C.int(result.output_size))
	gasLeft = int64(result.gas_left)
	if result.status_code != C.ZVMC_SUCCESS {
		err = Error(result.status_code)
	}

	if result.release != nil {
		C.zvmc_release_result(&result)
	}

	return output, gasLeft, err
}

var (
	hostContextCounter uintptr
	hostContextMap     = map[uintptr]HostContext{}
	hostContextMapMu   sync.Mutex
)

func addHostContext(ctx HostContext) uintptr {
	hostContextMapMu.Lock()
	id := hostContextCounter
	hostContextCounter++
	hostContextMap[id] = ctx
	hostContextMapMu.Unlock()
	return id
}

func removeHostContext(id uintptr) {
	hostContextMapMu.Lock()
	delete(hostContextMap, id)
	hostContextMapMu.Unlock()
}

func getHostContext(idx uintptr) HostContext {
	hostContextMapMu.Lock()
	ctx := hostContextMap[idx]
	hostContextMapMu.Unlock()
	return ctx
}

func zvmcBytes32(in Hash) C.zvmc_bytes32 {
	out := C.zvmc_bytes32{}
	for i := 0; i < len(in); i++ {
		out.bytes[i] = C.uint8_t(in[i])
	}
	return out
}

func zvmcAddress(address Address) C.zvmc_address {
	r := C.zvmc_address{}
	for i := 0; i < len(address); i++ {
		r.bytes[i] = C.uint8_t(address[i])
	}
	return r
}

func bytesPtr(bytes []byte) *C.uint8_t {
	if len(bytes) == 0 {
		return nil
	}
	return (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
}
