cc_library(
    name = "roo_io_arduino",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//lib/roo_io",
        "//lib/roo_logging",
        "//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/FS",
        "//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD",
    ],
)

cc_library(
    name = "testing",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    defines = ["ROO_IO_TESTING"],
    alwayslink = 1,
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_io_arduino",
        "//roo_testing:arduino_gtest_main",
    ],
)
