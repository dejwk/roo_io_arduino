cc_test(
    name = "arduino_fs_test",
    srcs = [
        "arduino_fs_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        "//lib/roo_io_arduino:testing",
        ":fakefs_arduino"
    ],
)

cc_test(
    name = "arduino_file_input_iterator_test",
    srcs = [
        "arduino_file_input_iterator_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        "//lib/roo_io/test:testing",
        ":fakefs_arduino"
    ],
)

cc_test(
    name = "arduino_file_output_iterator_test",
    srcs = [
        "arduino_file_output_iterator_test.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        "//lib/roo_io/test:testing",
        ":fakefs_arduino"
    ],
)

cc_library(
    name = "fakefs_arduino",
    srcs = [
        "fakefs_arduino.h",
        "fakefs_arduino.cpp",
    ],
    # includes = glob(["**/*.h"]),
    linkstatic = 1,
    deps = [
        "//lib/roo_io_arduino:testing",
        "//lib/roo_io/test/fs:fakefs"
    ],
)
