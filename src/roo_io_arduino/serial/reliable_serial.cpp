#include "roo_io_arduino/serial/reliable_serial.h"

#include <thread>

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
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

}  // namespace roo_io