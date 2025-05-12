// EVMC: Ethereum Client-VM Connector API.
// Copyright 2019-2020 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.
package org.theqrl.zvmc;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.charset.StandardCharsets;

public class TestMessage {
  int kind;
  int flags;
  int depth;
  long gas;
  char[] recipient;
  char[] sender;
  char[] inputData;
  long inputSize;
  char[] value;
  byte[] createSalt;
  byte[] codeAddress;

  public TestMessage(
      int kind,
      char[] sender,
      char[] recipient,
      char[] value,
      char[] inputData,
      long gas,
      int depth) {
    this.kind = kind;
    this.flags = 0;
    this.depth = depth;
    this.gas = gas;
    this.recipient = recipient;
    this.sender = sender;
    this.inputData = inputData;
    this.inputSize = (long) inputData.length;
    this.value = value;
    this.createSalt = new byte[32];
    this.codeAddress = new byte[24];
  }

  public TestMessage(ByteBuffer msg) {
    this.kind = msg.getInt();
    this.flags = msg.getInt();
    this.depth = msg.getInt();
    msg.getInt(); // padding
    this.gas = msg.getLong();
    ByteBuffer tmpbuf = msg.get(new byte[24]);
    this.recipient = StandardCharsets.ISO_8859_1.decode(tmpbuf).array();
    tmpbuf = msg.get(new byte[24]);
    this.sender = StandardCharsets.ISO_8859_1.decode(tmpbuf).array();
    tmpbuf = msg.get(new byte[8]);
    this.inputData = StandardCharsets.ISO_8859_1.decode(tmpbuf).array();
    this.inputSize = msg.getLong();
    tmpbuf = msg.get(new byte[32]);
    this.value = StandardCharsets.ISO_8859_1.decode(tmpbuf).array();
    this.createSalt = msg.get(new byte[32]).array();
    this.codeAddress = msg.get(new byte[24]).array();
  }

  public ByteBuffer toByteBuffer() {

    return ByteBuffer.allocateDirect(184)
        .order(ByteOrder.nativeOrder())
        .putInt(kind) // 4
        .putInt(flags) // 4
        .putInt(depth) // 4
        .put(new byte[4]) // 4 (padding)
        .putLong(gas) // 8
        .put(StandardCharsets.ISO_8859_1.encode(CharBuffer.wrap(recipient))) // 24
        .put(StandardCharsets.ISO_8859_1.encode(CharBuffer.wrap(sender))) // 24
        .put(StandardCharsets.ISO_8859_1.encode(CharBuffer.wrap(inputData))) // 8
        .putLong(inputSize) // 8
        .put(StandardCharsets.ISO_8859_1.encode(CharBuffer.wrap(value))) // 32
        .put(createSalt) // 32
        .put(codeAddress); // 24
  }
}
