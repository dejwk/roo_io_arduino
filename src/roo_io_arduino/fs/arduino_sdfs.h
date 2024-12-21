#pragma once

#include <SD.h>

#include <memory>

#include "roo_io_arduino/fs/arduino_mount.h"

namespace roo_io {

class ArduinoSdFs : public Filesystem {
 public:
  ArduinoSdFs(uint8_t ss_pin = SS, SDFS& sd = ::SD, SPIClass& spi = SPI,
              uint32_t freq = 20000000);

  MediaPresence checkMediaPresence() override;

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  SDFS& sd_;
  decltype(SPI)& spi_;
  uint8_t ss_pin_;
  uint32_t frequency_;
};

extern ArduinoSdFs SD;

}  // namespace roo_io