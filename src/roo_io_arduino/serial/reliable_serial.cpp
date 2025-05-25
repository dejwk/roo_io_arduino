#include "roo_io_arduino/serial/reliable_serial.h"

#include "Arduino.h"

namespace roo_io {

ReliableSerial::Connection::Connection(Channel& channel, uint32_t my_stream_id)
    : channel_(channel),
      in_(channel_, my_stream_id),
      out_(channel_, my_stream_id) {}

ReliableSerial::ReliableSerial(decltype(Serial)& serial,
                               unsigned int sendbuf_log2,
                               unsigned int recvbuf_log2, std::string label,
                               ConnectionCb connection_cb)
    : output_(serial),
      input_(serial),
      sender_(output_),
      receiver_(input_),
      channel_(sender_, receiver_, sendbuf_log2, recvbuf_log2,
               std::move(connection_cb)),
      connection_(nullptr) {
#ifdef ESP32
  serial.onReceive([this]() { channel_.tryRecv(); });
#endif
}

// void ReliableSerial::loop() { channel_.loop(); }

size_t ReliableSerial::Connection::ReliableOutputStream::write(
    const roo::byte* buf, size_t count) {
  if (status_ != kOk) return 0;
  if (count == 0) return 0;
  return channel_.write(buf, count, my_stream_id_, status_);
  // while (true) {
  //   for (int i = 0; i < 100; ++i) {
  //     size_t result = processor_.tryWrite(buf, count);
  //     if (result > 0) {
  //       // if (count > result) {
  //       //   buf += result;
  //       //   count -= result;
  //       //   result += processor_.tryWrite(buf, count);
  //       // }
  //       return result;
  //     }
  //     processor_.loop();
  //   }
  //   delay(1);
  // }
}

size_t ReliableSerial::Connection::ReliableOutputStream::tryWrite(
    const roo::byte* buf, size_t count) {
  if (status_ != kOk) return 0;
  return channel_.tryWrite(buf, count, my_stream_id_, status_);
}

int ReliableSerial::Connection::ReliableOutputStream::availableForWrite() {
  if (status_ != kOk) return 0;
  return channel_.availableForWrite(my_stream_id_, status_);
}

void ReliableSerial::Connection::ReliableOutputStream::flush() {
  if (status_ != kOk) return;
  channel_.flush(my_stream_id_, status_);
}

void ReliableSerial::Connection::ReliableOutputStream::close() {
  if (status_ != kOk) return;
  channel_.close(my_stream_id_, status_);
  if (status_ == kOk) status_ = kClosed;
}

void ReliableSerial::Connection::ReliableInputStream::close() {
  if (status_ != kOk && status_ != kEndOfStream) return;
  channel_.closeInput(my_stream_id_, status_);
  if (status_ != kOk && status_ != kEndOfStream) return;
  status_ = kClosed;
}

size_t ReliableSerial::Connection::ReliableInputStream::read(roo::byte* buf,
                                                             size_t count) {
  if (count == 0 || status_ != kOk) return 0;
  return channel_.read(buf, count, my_stream_id_, status_);
  // // processor_.loop();
  // while (true) {
  //   for (int i = 0; i < 100; ++i) {
  //     size_t result = processor_.tryRead(buf, count);
  //     if (result > 0) {
  //       // if (count > result) {
  //       //   buf += result;
  //       //   count -= result;
  //       //   result += processor_.tryRead(buf, count);
  //       // }
  //       return result;
  //     }
  //     processor_.loop();
  //   }
  //   delay(1);
  // }
}

size_t ReliableSerial::Connection::ReliableInputStream::tryRead(roo::byte* buf,
                                                                size_t count) {
  if (count == 0 || status_ != kOk) return 0;
  return channel_.tryRead(buf, count, my_stream_id_, status_);
}

int ReliableSerial::Connection::ReliableInputStream::available() {
  if (status_ != kOk) return 0;
  return channel_.availableForRead(my_stream_id_, status_);
}

int ReliableSerial::Connection::ReliableInputStream::read() {
  if (status_ != kOk) return 0;
  roo::byte result;
  size_t count = channel_.tryRead(&result, 1, my_stream_id_, status_);
  if (count > 0) return (int)result;
  return -1;
}

int ReliableSerial::Connection::ReliableInputStream::peek() {
  if (status_ != kOk) return -1;
  int result = channel_.peek(my_stream_id_, status_);
  if (result >= 0) return result;
  return -1;
}

size_t ReliableSerial::Connection::ReliableInputStream::timedRead(
    roo::byte* buf, size_t count, roo_time::Interval timeout) {
  roo_time::Uptime start = roo_time::Uptime::Now();
  size_t total = 0;
  if (status_ != kOk) return -1;
  while (count > 0) {
    for (int i = 0; i < 100; ++i) {
      size_t result = tryRead(buf, count);
      if (result == 0) {
        if (status_ != kOk) return -1;
        channel_.loop();
      } else {
        total += result;
        count -= result;
      }
      if (count == 0) return total;
    }
    if (roo_time::Uptime::Now() - start > timeout) break;
    delay(1);
  }
  return total;
}

int ReliableSerial::Connection::available() { return in_.available(); }

int ReliableSerial::Connection::read() { return in_.read(); }

int ReliableSerial::Connection::peek() { return in_.peek(); }

size_t ReliableSerial::Connection::readBytes(char* buffer, size_t length) {
  return in_.timedRead((roo::byte*)buffer, length,
                       roo_time::Millis(getTimeout()));
}

size_t ReliableSerial::Connection::readBytes(uint8_t* buffer, size_t length) {
  return in_.timedRead((roo::byte*)buffer, length,
                       roo_time::Millis(getTimeout()));
}

size_t ReliableSerial::Connection::write(uint8_t val) {
  out().write((const roo::byte*)&val, 1);
  return 1;
}

size_t ReliableSerial::Connection::write(const uint8_t* buffer, size_t size) {
  return out().writeFully((const roo::byte*)buffer, size);
}

int ReliableSerial::Connection::availableForWrite() {
  return out_.availableForWrite();
}

void ReliableSerial::Connection::flush() { out().flush(); }

std::shared_ptr<ReliableSerial::Connection> ReliableSerial::connect() {
  uint32_t my_stream_id = channel_.connect();
  connection_.reset(new Connection(channel_, my_stream_id));
  return connection_;
}

}  // namespace roo_io
