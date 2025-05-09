// EVMC: Ethereum Client-VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include <zvmc/loader.h>

#include <zvmc/helpers.h>
#include <zvmc/zvmc.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(ZVMC_LOADER_MOCK)
#include "../../test/unittests/loader_mock.h"
#elif defined(_WIN32)
#include <Windows.h>
#define DLL_HANDLE HMODULE
#define DLL_OPEN(filename) LoadLibrary(filename)
#define DLL_CLOSE(handle) FreeLibrary(handle)
#define DLL_GET_CREATE_FN(handle, name) (zvmc_create_fn)(uintptr_t) GetProcAddress(handle, name)
#define DLL_GET_ERROR_MSG() NULL
#else
#include <dlfcn.h>
#define DLL_HANDLE void*
#define DLL_OPEN(filename) dlopen(filename, RTLD_LAZY)
#define DLL_CLOSE(handle) dlclose(handle)
// NOLINTNEXTLINE(performance-no-int-to-ptr)
#define DLL_GET_CREATE_FN(handle, name) (zvmc_create_fn)(uintptr_t) dlsym(handle, name)
#define DLL_GET_ERROR_MSG() dlerror()
#endif

#ifdef __has_attribute
#if __has_attribute(format)
#define ATTR_FORMAT(archetype, string_index, first_to_check) \
    __attribute__((format(archetype, string_index, first_to_check)))
#endif
#endif

#ifndef ATTR_FORMAT
#define ATTR_FORMAT(...)
#endif

/*
 * Limited variant of strcpy_s().
 */
#if !defined(ZVMC_LOADER_MOCK)
static
#endif
    int
    strcpy_sx(char* dest, size_t destsz, const char* src)
{
    size_t len = strlen(src);
    if (len >= destsz)
    {
        // The input src will not fit into the dest buffer.
        // Set the first byte of the dest to null to make it effectively empty string
        // and return error.
        dest[0] = 0;
        return 1;
    }
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    memcpy(dest, src, len);
    dest[len] = 0;
    return 0;
}

enum
{
    PATH_MAX_LENGTH = 4096,
    LAST_ERROR_MSG_BUFFER_SIZE = 511
};

static const char* last_error_msg = NULL;

// Buffer for formatted error messages.
// It has one null byte extra to avoid buffer read overflow during concurrent access.
static char last_error_msg_buffer[LAST_ERROR_MSG_BUFFER_SIZE + 1];

ATTR_FORMAT(printf, 2, 3)
static enum zvmc_loader_error_code set_error(enum zvmc_loader_error_code error_code,
                                             const char* format,
                                             ...)
{
    va_list args;
    va_start(args, format);
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    if (vsnprintf(last_error_msg_buffer, LAST_ERROR_MSG_BUFFER_SIZE, format, args) <
        LAST_ERROR_MSG_BUFFER_SIZE)
        last_error_msg = last_error_msg_buffer;
    va_end(args);
    return error_code;
}


zvmc_create_fn zvmc_load(const char* filename, enum zvmc_loader_error_code* error_code)
{
    last_error_msg = NULL;  // Reset last error.
    enum zvmc_loader_error_code ec = ZVMC_LOADER_SUCCESS;
    zvmc_create_fn create_fn = NULL;

    if (!filename)
    {
        ec = set_error(ZVMC_LOADER_INVALID_ARGUMENT, "invalid argument: file name cannot be null");
        goto exit;
    }

    const size_t length = strlen(filename);
    if (length == 0)
    {
        ec = set_error(ZVMC_LOADER_INVALID_ARGUMENT, "invalid argument: file name cannot be empty");
        goto exit;
    }
    else if (length > PATH_MAX_LENGTH)
    {
        ec = set_error(ZVMC_LOADER_INVALID_ARGUMENT,
                       "invalid argument: file name is too long (%d, maximum allowed length is %d)",
                       (int)length, PATH_MAX_LENGTH);
        goto exit;
    }

    DLL_HANDLE handle = DLL_OPEN(filename);
    if (!handle)
    {
        // Get error message if available.
        last_error_msg = DLL_GET_ERROR_MSG();
        if (last_error_msg)
            ec = ZVMC_LOADER_CANNOT_OPEN;
        else
            ec = set_error(ZVMC_LOADER_CANNOT_OPEN, "cannot open %s", filename);
        goto exit;
    }

    // Create name buffer with the prefix.
    const char prefix[] = "zvmc_create_";
    const size_t prefix_length = strlen(prefix);
    char prefixed_name[sizeof(prefix) + PATH_MAX_LENGTH];
    strcpy_sx(prefixed_name, sizeof(prefixed_name), prefix);

    // Find filename in the path.
    const char* sep_pos = strrchr(filename, '/');
#ifdef _WIN32
    // On Windows check also Windows classic path separator.
    const char* sep_pos_windows = strrchr(filename, '\\');
    sep_pos = sep_pos_windows > sep_pos ? sep_pos_windows : sep_pos;
#endif
    const char* name_pos = sep_pos ? sep_pos + 1 : filename;

    // Skip "lib" prefix if present.
    const char lib_prefix[] = "lib";
    const size_t lib_prefix_length = strlen(lib_prefix);
    if (strncmp(name_pos, lib_prefix, lib_prefix_length) == 0)
        name_pos += lib_prefix_length;

    char* base_name = prefixed_name + prefix_length;
    strcpy_sx(base_name, PATH_MAX_LENGTH, name_pos);

    // Trim all file extensions.
    char* ext_pos = strchr(prefixed_name, '.');
    if (ext_pos)
        *ext_pos = 0;

    // Replace all "-" with "_".
    char* dash_pos = base_name;
    while ((dash_pos = strchr(dash_pos, '-')) != NULL)
        *dash_pos++ = '_';

    // Search for the built function name.
    create_fn = DLL_GET_CREATE_FN(handle, prefixed_name);

    if (!create_fn)
        create_fn = DLL_GET_CREATE_FN(handle, "zvmc_create");

    if (!create_fn)
    {
        DLL_CLOSE(handle);
        ec = set_error(ZVMC_LOADER_SYMBOL_NOT_FOUND, "ZVMC create function not found in %s",
                       filename);
    }

exit:
    if (error_code)
        *error_code = ec;
    return create_fn;
}

const char* zvmc_last_error_msg(void)
{
    const char* m = last_error_msg;
    last_error_msg = NULL;
    return m;
}

struct zvmc_vm* zvmc_load_and_create(const char* filename, enum zvmc_loader_error_code* error_code)
{
    // First load the DLL. This also resets the last_error_msg;
    zvmc_create_fn create_fn = zvmc_load(filename, error_code);

    if (!create_fn)
        return NULL;

    enum zvmc_loader_error_code ec = ZVMC_LOADER_SUCCESS;

    struct zvmc_vm* vm = create_fn();
    if (!vm)
    {
        ec = set_error(ZVMC_LOADER_VM_CREATION_FAILURE, "creating ZVMC VM of %s has failed",
                       filename);
        goto exit;
    }

    if (!zvmc_is_abi_compatible(vm))
    {
        ec = set_error(ZVMC_LOADER_ABI_VERSION_MISMATCH,
                       "ZVMC ABI version %d of %s mismatches the expected version %d",
                       vm->abi_version, filename, ZVMC_ABI_VERSION);
        zvmc_destroy(vm);
        vm = NULL;
        goto exit;
    }

exit:
    if (error_code)
        *error_code = ec;

    return vm;
}

/// Gets the token delimited by @p delim character of the string pointed by the @p str_ptr.
/// If the delimiter is not found, the whole string is returned.
/// The @p str_ptr is also slided after the delimiter or to the string end
/// if the delimiter is not found (in this case the @p str_ptr points to an empty string).
static char* get_token(char** str_ptr, char delim)
{
    char* str = *str_ptr;
    char* delim_pos = strchr(str, delim);
    if (delim_pos)
    {
        // If the delimiter is found, null it to get null-terminated prefix
        // and slide the str_ptr after the delimiter.
        *delim_pos = '\0';
        *str_ptr = delim_pos + 1;
    }
    else
    {
        // Otherwise, slide the str_ptr to the end and return the whole string as the prefix.
        *str_ptr += strlen(str);
    }
    return str;
}

struct zvmc_vm* zvmc_load_and_configure(const char* config, enum zvmc_loader_error_code* error_code)
{
    enum zvmc_loader_error_code ec = ZVMC_LOADER_SUCCESS;
    struct zvmc_vm* vm = NULL;

    char config_copy_buffer[PATH_MAX_LENGTH];
    if (strcpy_sx(config_copy_buffer, sizeof(config_copy_buffer), config) != 0)
    {
        ec = set_error(ZVMC_LOADER_INVALID_ARGUMENT,
                       "invalid argument: configuration is too long (maximum allowed length is %d)",
                       (int)sizeof(config_copy_buffer));
        goto exit;
    }

    char* options = config_copy_buffer;
    const char* path = get_token(&options, ',');

    vm = zvmc_load_and_create(path, error_code);
    if (!vm)
        return NULL;

    while (strlen(options) != 0)
    {
        if (vm->set_option == NULL)
        {
            ec = set_error(ZVMC_LOADER_INVALID_OPTION_NAME, "%s (%s) does not support any options",
                           vm->name, path);
            goto exit;
        }

        char* option = get_token(&options, ',');

        // Slit option into name and value by taking the name token.
        // The option variable will have the value, can be empty.
        const char* name = get_token(&option, '=');

        enum zvmc_set_option_result r = vm->set_option(vm, name, option);
        switch (r)
        {
        case ZVMC_SET_OPTION_SUCCESS:
            break;
        case ZVMC_SET_OPTION_INVALID_NAME:
            ec = set_error(ZVMC_LOADER_INVALID_OPTION_NAME, "%s (%s): unknown option '%s'",
                           vm->name, path, name);
            goto exit;
        case ZVMC_SET_OPTION_INVALID_VALUE:
            ec = set_error(ZVMC_LOADER_INVALID_OPTION_VALUE,
                           "%s (%s): unsupported value '%s' for option '%s'", vm->name, path,
                           option, name);
            goto exit;

        default:
            ec = set_error(ZVMC_LOADER_INVALID_OPTION_VALUE,
                           "%s (%s): unknown error when setting value '%s' for option '%s'",
                           vm->name, path, option, name);
            goto exit;
        }
    }

exit:
    if (error_code)
        *error_code = ec;

    if (ec == ZVMC_LOADER_SUCCESS)
        return vm;

    if (vm)
        zvmc_destroy(vm);
    return NULL;
}
