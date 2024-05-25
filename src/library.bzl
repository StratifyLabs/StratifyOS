"""
Library Helper
"""

def sos_library(name, srcs = [], deps = [], headers = []):
    native.cxx_library(
        name = name,
        target_compatible_with = ["toolchains//config:stratifyos"],
        srcs = srcs,
        compiler_flags = ["-DSOS_BOOTSTRAP_SOCKETS=1"],
        headers = headers,
        deps = deps + [
            "//deps/StratifyOS/src:config",
            "//deps/StratifyOS/include:sos_include",
            "//config:stratifyos_config",
        ],
        _cxx_toolchain = select({
            "toolchains//config:stratifyos": "toolchains//:arm-none-eabi",
        }),
    )
