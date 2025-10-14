cc_library(
    name = "roo_io_arduino",
    srcs = glob(
        [
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@roo_io",
        "@roo_logging",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/FS",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD",
    ],
)
