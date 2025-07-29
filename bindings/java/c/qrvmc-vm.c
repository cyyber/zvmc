// EVMC: Ethereum Client-VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include "host.h"
#include "org_theqrl_qrvmc_QrvmcVm.h"
#include <qrvmc/helpers.h>
#include <qrvmc/loader.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

JNIEXPORT jobject JNICALL Java_org_theqrl_qrvmc_QrvmcVm_load_1and_1create(JNIEnv* jenv,
                                                                        jclass jcls,
                                                                        jstring jfilename)
{
    (void)jcls;
    struct qrvmc_vm* qrvm = NULL;
    jint rs = qrvmc_java_set_jvm(jenv);
    (void)rs;
    assert(rs == JNI_OK);
    // load the QRVM
    const char* filename = (*jenv)->GetStringUTFChars(jenv, jfilename, NULL);
    assert(filename != NULL);
    enum qrvmc_loader_error_code loader_error = QRVMC_LOADER_UNSPECIFIED_ERROR;
    qrvm = qrvmc_load_and_create(filename, &loader_error);
    (*jenv)->ReleaseStringUTFChars(jenv, jfilename, filename);
    if (loader_error != QRVMC_LOADER_SUCCESS)
    {
        const char* error_msg = qrvmc_last_error_msg();
        jclass exception_class = (*jenv)->FindClass(jenv, "org/theqrl/qrvmc/QrvmcLoaderException");
        assert(exception_class != NULL);
        (*jenv)->ThrowNew(jenv, exception_class, error_msg ? error_msg : "Loading QRVMC VM failed");
    }
    jobject jresult = (*jenv)->NewDirectByteBuffer(jenv, (void*)qrvm, sizeof(struct qrvmc_vm));
    assert(jresult != NULL);
    return jresult;
}

JNIEXPORT jint JNICALL Java_org_theqrl_qrvmc_QrvmcVm_abi_1version(JNIEnv* jenv, jclass jcls)
{
    (void)jenv;
    (void)jcls;
    return QRVMC_ABI_VERSION;
}

JNIEXPORT jstring JNICALL Java_org_theqrl_qrvmc_QrvmcVm_name(JNIEnv* jenv, jclass jcls, jobject jqrvm)
{
    (void)jcls;
    struct qrvmc_vm* qrvm = (struct qrvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jqrvm);
    assert(qrvm != NULL);
    const char* qrvm_name = qrvmc_vm_name(qrvm);
    return (*jenv)->NewStringUTF(jenv, qrvm_name);
}

JNIEXPORT jstring JNICALL Java_org_theqrl_qrvmc_QrvmcVm_version(JNIEnv* jenv,
                                                              jclass jcls,
                                                              jobject jqrvm)
{
    (void)jcls;
    struct qrvmc_vm* qrvm = (struct qrvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jqrvm);
    assert(qrvm != NULL);
    const char* qrvm_version = qrvmc_vm_version(qrvm);
    return (*jenv)->NewStringUTF(jenv, qrvm_version);
}

JNIEXPORT void JNICALL Java_org_theqrl_qrvmc_QrvmcVm_destroy(JNIEnv* jenv, jclass jcls, jobject jqrvm)
{
    (void)jcls;
    struct qrvmc_vm* qrvm = (struct qrvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jqrvm);
    assert(qrvm != NULL);
    qrvmc_destroy(qrvm);
}

static jobject AllocateDirect(JNIEnv* jenv, size_t capacity)
{
    const char java_class_name[] = "java/nio/ByteBuffer";
    const char java_method_name[] = "allocateDirect";
    const char java_method_signature[] = "(I)Ljava/nio/ByteBuffer;";

    jclass jcls = (*jenv)->FindClass(jenv, java_class_name);
    assert(jcls != NULL);
    jmethodID method =
        (*jenv)->GetStaticMethodID(jenv, jcls, java_method_name, java_method_signature);
    assert(method != NULL);
    return (*jenv)->CallStaticObjectMethod(jenv, jcls, method, capacity);
}

JNIEXPORT jobject JNICALL Java_org_theqrl_qrvmc_QrvmcVm_execute(JNIEnv* jenv,
                                                              jclass jcls,
                                                              jobject jqrvm,
                                                              jobject jcontext,
                                                              jint jrev,
                                                              jobject jmsg,
                                                              jobject jcode)
{
    (void)jcls;
    struct qrvmc_message* msg = (struct qrvmc_message*)(*jenv)->GetDirectBufferAddress(jenv, jmsg);
    assert(msg != NULL);
    size_t code_size;
    const uint8_t* code = GetDirectBuffer(jenv, jcode, &code_size);
    struct qrvmc_vm* qrvm = (struct qrvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jqrvm);
    assert(qrvm != NULL);
    const struct qrvmc_host_interface* host = qrvmc_java_get_host_interface();
    jobject jresult = AllocateDirect(jenv, sizeof(struct qrvmc_result));
    assert(jresult != NULL);
    struct qrvmc_result* result =
        (struct qrvmc_result*)(*jenv)->GetDirectBufferAddress(jenv, jresult);
    assert(result != NULL);
    *result = qrvmc_execute(qrvm, host, (struct qrvmc_host_context*)jcontext, (enum qrvmc_revision)jrev,
                           msg, code, code_size);
    return jresult;
}

JNIEXPORT jint JNICALL Java_org_theqrl_qrvmc_QrvmcVm_get_1capabilities(JNIEnv* jenv,
                                                                     jclass jcls,
                                                                     jobject jqrvm)
{
    (void)jcls;
    struct qrvmc_vm* qrvm = (struct qrvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jqrvm);
    assert(qrvm != NULL);
    return (jint)qrvm->get_capabilities(qrvm);
}

JNIEXPORT jint JNICALL Java_org_theqrl_qrvmc_QrvmcVm_set_1option(JNIEnv* jenv,
                                                               jclass jcls,
                                                               jobject jqrvm,
                                                               jstring jname,
                                                               jstring jval)
{
    (void)jcls;
    struct qrvmc_vm* qrvm = (struct qrvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jqrvm);
    assert(qrvm != NULL);
    const char* name = (*jenv)->GetStringUTFChars(jenv, jname, 0);
    const char* value = (*jenv)->GetStringUTFChars(jenv, jval, 0);
    assert(name != NULL);
    assert(value != NULL);
    enum qrvmc_set_option_result option_result = qrvmc_set_option(qrvm, name, value);
    (*jenv)->ReleaseStringUTFChars(jenv, jname, name);
    (*jenv)->ReleaseStringUTFChars(jenv, jval, value);
    return (jint)option_result;
}
