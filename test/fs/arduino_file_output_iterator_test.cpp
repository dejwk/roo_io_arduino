#include "roo_io_arduino/fs/arduino_file_output_iterator.h"

#include "fakefs.h"
#include "fakefs_arduino.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "output_iterator_p.h"
#include "roo_io_arduino/fs/arduino_sdfs.h"

namespace roo_io {

class ArduinoFileOutputIteratorFixture {
 public:
  ArduinoFileOutputIterator createIterator(size_t max_size) {
    fake_ = std::unique_ptr<fakefs::FakeFs>(new fakefs::FakeFs(max_size));
    sdfs_ = std::unique_ptr<fakefs::FakeArduinoSdFsImpl>(
        new fakefs::FakeArduinoSdFsImpl(*fake_));
    return ArduinoFileOutputIterator(sdfs_->open("/foo", "w"));
  }

  std::vector<byte> getResult() const {
    return fakefs::ReadFile(*fake_, "/foo");
  }

  std::string getResultAsString() const {
    return fakefs::ReadTextFile(*fake_, "/foo");
  }

  static constexpr bool strict = false;

 private:
  std::unique_ptr<fakefs::FakeFs> fake_;
  std::unique_ptr<fakefs::FakeArduinoSdFsImpl> sdfs_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(ArduinoFileOutputIteratorTest,
                               OutputIteratorTest,
                               ArduinoFileOutputIteratorFixture);

TEST(ArduinoFileOutputIterator, DefaultConstructible) {
  ArduinoFileOutputIterator itr;
  EXPECT_EQ(kClosed, itr.status());
  itr.write(byte{5});
  byte buf[] = {byte{1}, byte{2}};
  EXPECT_EQ(0, itr.write(buf, 2));
  EXPECT_EQ(kClosed, itr.status());
}

}  // namespace roo_io
