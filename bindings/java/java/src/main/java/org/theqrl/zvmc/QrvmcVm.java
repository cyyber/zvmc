// EVMC: Ethereum Client-VM Connector API.
// Copyright 2019-2020 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.
package org.theqrl.qrvmc;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;

/**
 * The Java interface to the qrvm instance.
 *
 * <p>Defines the Java methods capable of accessing the qrvm implementation.
 */
public final class QrvmcVm implements AutoCloseable {
  private static final Throwable qrvmcLoadingError;
  private ByteBuffer nativeVm;

  // Load the dynamic library containing the JNI bindings to QRVMC.
  static {
    Throwable error = null;

    // First try loading from global path.
    try {
      System.loadLibrary("libqrvmc-java");
    } catch (UnsatisfiedLinkError globalLoadingError) {
      String extension = null;
      String os = System.getProperty("os.name").toLowerCase();
      if (os.contains("win")) {
        extension = "dll";
      } else if (os.contains("nix") || os.contains("nux") || os.contains("aix")) {
        extension = "so";
      } else if (os.contains("mac") || os.contains("darwin")) {
        extension = "dylib";
      } else {
        // Give up, because we are unsure what system we are running on.
        error = globalLoadingError;
      }

      // Try loading the binding from the package.
      if (extension != null) {
        try {
          Path qrvmcLib = Files.createTempFile("libqrvmc-java", extension);
          Files.copy(
              QrvmcVm.class.getResourceAsStream("/libqrvmc-java." + extension),
              qrvmcLib,
              StandardCopyOption.REPLACE_EXISTING);
          qrvmcLib.toFile().deleteOnExit();
          // We are somewhat certain about the file, try loading it.
          try {
            System.load(qrvmcLib.toAbsolutePath().toString());
          } catch (UnsatisfiedLinkError packageLoadingError) {
            error = packageLoadingError;
          }
        } catch (IOException packageCreationError) {
          error = packageCreationError;
        }
      }
    }
    qrvmcLoadingError = error;
  }

  /**
   * Returns true if the native library was loaded successfully and QRVMC capabilities are available.
   *
   * @return true if the library is available
   */
  public static boolean isAvailable() {
    return qrvmcLoadingError == null;
  }

  /**
   * This method loads the specified qrvm shared library and loads/initializes the jni bindings.
   *
   * @param filename /path/filename of the qrvm shared object
   * @throws org.theqrl.qrvmc.QrvmcLoaderException
   */
  public static QrvmcVm create(String filename) throws QrvmcLoaderException {
    if (!isAvailable()) {
      throw new QrvmcLoaderException("QRVMC JNI binding library failed to load", qrvmcLoadingError);
    }
    return new QrvmcVm(filename);
  }

  private QrvmcVm(String filename) throws QrvmcLoaderException {
    nativeVm = load_and_create(filename);
  }

  /**
   * This method loads the specified QRVM implementation and returns its pointer.
   *
   * @param filename Path to the dynamic object representing the QRVM implementation
   * @return Internal object pointer.
   * @throws org.theqrl.qrvmc.QrvmcLoaderException
   */
  private static native ByteBuffer load_and_create(String filename) throws QrvmcLoaderException;

  /**
   * QRVMC ABI version implemented by the VM instance.
   *
   * <p>Can be used to detect ABI incompatibilities. The QRVMC ABI version represented by this file
   * is in ::QRVMC_ABI_VERSION.
   */
  public static native int abi_version();

  /**
   * The name of the QRVMC VM implementation.
   *
   * <p>It MUST be a NULL-terminated not empty string. The content MUST be UTF-8 encoded (this
   * implies ASCII encoding is also allowed).
   */
  private static native String name(ByteBuffer nativeVm);

  /** Function is a wrapper around native name(). */
  public String name() {
    return name(nativeVm);
  }

  /**
   * The version of the QRVMC VM implementation, e.g. "1.2.3b4".
   *
   * <p>It MUST be a NULL-terminated not empty string. The content MUST be UTF-8 encoded (this
   * implies ASCII encoding is also allowed).
   */
  private static native String version(ByteBuffer nativeVm);

  /** Function is a wrapper around native version(). */
  public String version() {
    return version(nativeVm);
  }

  /**
   * Function to destroy the VM instance.
   *
   * <p>This is a mandatory method and MUST NOT be set to NULL.
   */
  private static native void destroy(ByteBuffer nativeVm);

  /**
   * Function to execute a code by the VM instance.
   *
   * <p>This is a mandatory method and MUST NOT be set to NULL.
   */
  private static native ByteBuffer execute(
      ByteBuffer nativeVm, HostContext context, int rev, ByteBuffer msg, ByteBuffer code);

  /**
   * Function is a wrapper around native execute.
   *
   * <p>This allows the context to managed in one method
   */
  public synchronized ByteBuffer execute(
      HostContext context, int rev, ByteBuffer msg, ByteBuffer code) {
    return execute(nativeVm, context, rev, msg, code);
  }

  /**
   * A method returning capabilities supported by the VM instance.
   *
   * <p>The value returned MAY change when different options are set via the set_option() method.
   *
   * <p>A Client SHOULD only rely on the value returned if it has queried it after it has called the
   * set_option().
   *
   * <p>This is a mandatory method and MUST NOT be set to NULL.
   */
  private static native int get_capabilities(ByteBuffer nativeVm);

  /** Function is a wrapper around native get_capabilities(). */
  public int get_capabilities() {
    return get_capabilities(nativeVm);
  }

  /**
   * Function that modifies VM's options.
   *
   * <p>If the VM does not support this feature the pointer can be NULL.
   */
  private static native int set_option(ByteBuffer nativeVm, String name, String value);

  /** Function is a wrapper around native set_option(). */
  public int set_option(String name, String value) {
    return set_option(nativeVm, name, value);
  }

  /** This method cleans up resources. */
  @Override
  public void close() {
    destroy(nativeVm);
  }
}
