#include "roo_io_arduino/serial/reliable_serial.h"

#include "Arduino.h"

namespace roo_io {

ReliableSerial::ReliableSerial(decltype(Serial)& serial,
                               unsigned int sendbuf_log2,
                               unsigned int recvbuf_log2)
    : output_(serial),
      input_(serial),
      sender_(output_),
      receiver_(input_),
      processor_(sender_, receiver_, sendbuf_log2, recvbuf_log2),
      in_(processor_),
      out_(processor_) {}

void ReliableSerial::loop() {
  processor_.recvLoop();
  processor_.sendLoop();
}

namespace {

size_t TimedWrite(StreamingRetransmitter& target, const roo::byte* buf,
                  size_t count, unsigned long timeout) {
  unsigned long start = millis();
  size_t total = 0;
  while (count > 0) {
    for (int i = 0; i < 1000; ++i) {
      size_t result = target.tryWrite(buf, count);
      if (result == 0) {
        target.recvLoop();
        target.sendLoop();
      } else {
        total += result;
        count -= result;
      }
      if (millis() - start > timeout) break;
      delay(1);
    }
    return total;
  }
}

size_t TimedRead(StreamingRetransmitter& source, roo::byte* buf, size_t count,
                 unsigned long timeout) {
  unsigned long start = millis();
  size_t total = 0;
  while (count > 0) {
    for (int i = 0; i < 1000; ++i) {
      size_t result = source.tryRead(buf, count);
      if (result == 0) {
        source.recvLoop();
        source.sendLoop();
      } else {
        total += result;
        count -= result;
      }
    }
    if (millis() - start > timeout) break;
    delay(1);
  }
  return total;
}

}  // namespace

size_t ReliableSerial::ReliableOutputStream::write(const roo::byte* buf,
                                                   size_t count) {
  if (status() != roo_io::kOk) return 0;
  if (count == 0) return 0;
  while (true) {
    for (int i = 0; i < 1000; ++i) {
      size_t result = processor_.tryWrite(buf, count);
      if (result > 0) {
        // if (count > result) {
        //   buf += result;
        //   count -= result;
        //   result += processor_.tryWrite(buf, count);
        // }
        return result;
      }
      processor_.recvLoop();
      processor_.sendLoop();
    }
    delay(1);
  }
}

size_t ReliableSerial::ReliableInputStream::read(roo::byte* buf, size_t count) {
  if (status() != roo_io::kOk) return 0;
  if (count == 0) return 0;
  while (true) {
    for (int i = 0; i < 1000; ++i) {
      size_t result = processor_.tryRead(buf, count);
      if (result > 0) {
        // if (count > result) {
        //   buf += result;
        //   count -= result;
        //   result += processor_.tryRead(buf, count);
        // }
        return result;
      }
      processor_.recvLoop();
      processor_.sendLoop();
    }
    delay(1);
  }
}

int ReliableSerial::available() { return 0; }

int ReliableSerial::read() {
  roo::byte result;
  size_t count = processor_.tryRead(&result, 1);
  return count > 0 ? (int)result : -1;
}

int ReliableSerial::peek() { return processor_.peek(); }

size_t ReliableSerial::readBytes(char* buffer, size_t length) {
  return TimedRead(processor_, (roo::byte*)buffer, length, getTimeout());
}

size_t ReliableSerial::readBytes(uint8_t* buffer, size_t length) {
  return TimedRead(processor_, buffer, length, getTimeout());
}

size_t ReliableSerial::write(uint8_t val) {
  out().write((const roo::byte*)&val, 1);
  return 1;
}

size_t ReliableSerial::write(const uint8_t* buffer, size_t size) {
  return out().writeFully((const roo::byte*)buffer, size);
}

int ReliableSerial::availableForWrite() {
  return processor_.availableForWrite();
}

void ReliableSerial::flush() { out().flush(); }

}  // namespace roo_io
