#include "roo_io/core/output_stream.h"
#include "roo_io/memory/load.h"
#include "roo_io/memory/store.h"
#include "roo_io/reliable/packet_receiver.h"
#include "roo_io/reliable/packet_sender.h"
#include "roo_io/reliable/streaming_retransmitter.h"
#include "roo_io_arduino/stream/arduino_stream_input_stream.h"
#include "roo_io_arduino/stream/arduino_stream_output_stream.h"

namespace roo_io {

class ReliableSerial : public Stream {
 public:
  ReliableSerial(decltype(Serial)& serial, unsigned int sendbuf_log2,
                 unsigned int recvbuf_log2);

  // Obtains the input stream that can be used to read from the reliable serial.
  roo_io::InputStream& in() { return in_; }

  // Obtains the output stream that can be used to write to the reliable serial.
  roo_io::OutputStream& out() { return out_; }

  void loop();

  // Stream overrides.

  int available() override;
  int read() override;
  int peek() override;

  size_t readBytes(char* buffer, size_t length) override;
  size_t readBytes(uint8_t* buffer, size_t length) override;

  size_t write(uint8_t) override;
  size_t write(const uint8_t* buffer, size_t size) override;
  int availableForWrite() override;
  void flush() override;

 private:
  class ReliableOutputStream : public roo_io::OutputStream {
   public:
    ReliableOutputStream(StreamingRetransmitter& processor)
        : processor_(processor) {}

    size_t write(const roo::byte* buf, size_t count) override;

    size_t tryWrite(const roo::byte* buf, size_t count) override {
      return processor_.tryWrite(buf, count);
    }

    void flush() override { processor_.flush(); }

    roo_io::Status status() const override { return roo_io::kOk; }

   private:
    StreamingRetransmitter& processor_;
  };

  class ReliableInputStream : public roo_io::InputStream {
   public:
    ReliableInputStream(StreamingRetransmitter& processor)
        : processor_(processor) {}

    size_t read(roo::byte* buf, size_t count) override;

    size_t tryRead(roo::byte* buf, size_t count) override {
      return processor_.tryRead(buf, count);
    }

    roo_io::Status status() const override { return roo_io::kOk; }

   private:
    StreamingRetransmitter& processor_;
  };

  roo_io::ArduinoStreamOutputStream output_;
  roo_io::ArduinoStreamInputStream input_;
  roo_io::PacketSender sender_;
  roo_io::PacketReceiver receiver_;

  StreamingRetransmitter processor_;
  ReliableInputStream in_;
  ReliableOutputStream out_;
};

}  // namespace roo_io