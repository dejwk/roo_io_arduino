#include <memory>

#include "roo_io/core/output_stream.h"
#include "roo_io/memory/load.h"
#include "roo_io/memory/store.h"
#include "roo_io/reliable/bidi_streaming/internal/thread_safe/channel.h"
#include "roo_io/reliable/packet_transport/over_stream/packet_receiver_over_stream.h"
#include "roo_io/reliable/packet_transport/over_stream/packet_sender_over_stream.h"
#include "roo_io_arduino/stream/arduino_stream_input_stream.h"
#include "roo_io_arduino/stream/arduino_stream_output_stream.h"

namespace roo_io {

class ReliableSerial {
 public:
  using ConnectionCb = Channel::ConnectionCb;

  class Connection : public Stream {
   public:
    Connection(Channel& channel, uint32_t my_stream_id);

    // Status status() const;

    int available() override;
    int read() override;
    int peek() override;

    size_t readBytes(char* buffer, size_t length) override;
    size_t readBytes(uint8_t* buffer, size_t length) override;

    size_t write(uint8_t) override;
    size_t write(const uint8_t* buffer, size_t size) override;
    int availableForWrite() override;
    void flush() override;

    // Obtains the input stream that can be used to read from the reliable
    // serial.
    roo_io::InputStream& in() { return in_; }

    // Obtains the output stream that can be used to write to the reliable
    // serial.
    roo_io::OutputStream& out() { return out_; }

   private:
    class ReliableOutputStream : public roo_io::OutputStream {
     public:
      ReliableOutputStream(Channel& channel, uint32_t my_stream_id)
          : channel_(channel), my_stream_id_(my_stream_id), status_(kOk) {}

      size_t write(const roo::byte* buf, size_t count) override;

      size_t tryWrite(const roo::byte* buf, size_t count) override;

      int availableForWrite();

      void flush() override;

      void close() override;

      roo_io::Status status() const override { return status_; }

     private:
      Channel& channel_;
      uint32_t my_stream_id_;
      roo_io::Status status_;
    };

    class ReliableInputStream : public roo_io::InputStream {
     public:
      ReliableInputStream(Channel& channel, uint32_t my_stream_id)
          : channel_(channel), my_stream_id_(my_stream_id), status_(kOk) {}

      void close() override;

      size_t read(roo::byte* buf, size_t count) override;

      size_t tryRead(roo::byte* buf, size_t count) override;

      int available();
      int read();
      int peek();

      size_t timedRead(roo::byte* buf, size_t count,
                       roo_time::Interval timeout);

      roo_io::Status status() const override { return status_; }

     private:
      Channel& channel_;
      uint32_t my_stream_id_;
      roo_io::Status status_;
    };

    Channel& channel_;
    ReliableInputStream in_;
    ReliableOutputStream out_;
  };

  ReliableSerial(decltype(Serial)& serial, unsigned int sendbuf_log2,
                 unsigned int recvbuf_log2, std::string label,
                 ConnectionCb connection_cb = nullptr);

  void begin() { channel_.begin(); }

  // void loop();

  // Stream overrides.

  std::shared_ptr<Connection> connect();

  uint32_t packets_sent() const { return channel_.packets_sent(); }
  uint32_t packets_delivered() const { return channel_.packets_delivered(); }
  uint32_t packets_received() const { return channel_.packets_received(); }

  size_t receiver_bytes_received() const { return receiver_.bytes_received(); }

  size_t receiver_bytes_accepted() const { return receiver_.bytes_accepted(); }

 private:
  roo_io::ArduinoStreamOutputStream output_;
  roo_io::ArduinoStreamInputStream input_;
  roo_io::PacketSenderOverStream sender_;
  roo_io::PacketReceiverOverStream receiver_;

  Channel channel_;
  std::shared_ptr<Connection> connection_;
};

}  // namespace roo_io