// EVMC: Ethereum Client-VM Connector API.
// Copyright 2019-2020 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.
package org.theqrl.qrvmc;

/** Exception thrown when the QRVMC binding or VM fails to load. */
public class QrvmcLoaderException extends Exception {
  public QrvmcLoaderException(String message) {
    super(message);
  }

  public QrvmcLoaderException(String message, Throwable cause) {
    super(message, cause);
  }
}
