#include "roo_io_arduino/fs/arduino_file_input_iterator.h"

#include "fakefs.h"
#include "fakefs_arduino.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "input_iterator_p.h"
#include "multipass_input_iterator_p.h"
#include "roo_io_arduino/fs/arduino_sdfs.h"

namespace roo_io {

class ArduinoFileIteratorFixture {
 public:
  using Iterator = ArduinoFileInputIterator;

  ArduinoFileInputIterator createIterator(const byte* beg, size_t size) {
    fake_ = std::unique_ptr<fakefs::FakeFs>(new fakefs::FakeFs());
    sdfs_ = std::unique_ptr<fakefs::FakeArduinoSdFsImpl>(
        new fakefs::FakeArduinoSdFsImpl(*fake_));
    {
      fakefs::FileStream fs = fake_->open(
          "/foo", fakefs::FakeFs::kWrite | fakefs::FakeFs::kTruncate);
      CHECK_EQ(size, fs.write(beg, size));
    }
    return ArduinoFileInputIterator(sdfs_->open("/foo"));
  }

 private:
  std::unique_ptr<fakefs::FakeFs> fake_;
  std::unique_ptr<fakefs::FakeArduinoSdFsImpl> sdfs_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(ArduinoFileIteratorTest, InputIteratorTest,
                               ArduinoFileIteratorFixture);

INSTANTIATE_TYPED_TEST_SUITE_P(ArduinoFileIteratorTest,
                               MultipassInputIteratorTest,
                               ArduinoFileIteratorFixture);

}  // namespace roo_io
