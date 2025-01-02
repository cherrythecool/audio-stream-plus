#!/usr/bin/env python
env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/", "thirdparty/"])
sources = Glob("src/*.cpp")

env.Append(CPPPATH=[
    "thirdparty/ogg/",
    "thirdparty/opusfile/",

    # Thank you opus
    "thirdparty/opus/",
    "thirdparty/opus/celt",
    "thirdparty/opus/celt/arm",
    "thirdparty/opus/silk",
    "thirdparty/opus/silk/float",
    "thirdparty/opus/silk/fixed",
])

# Note: for compiling arm and x86 platforms we probably want to do some specific shit
# for that with config headers;
# but if do that might also just switch to building with zig, not sure

env.Append(CPPDEFINES=["USE_ALLOCA", "OPUS_BUILD"])#, "HAVE_CONFIG_H"])
sources += Glob("thirdparty/ogg/*.c") + Glob("thirdparty/opusfile/*.c")

# we will need more in future when optimizing stuff like arm and x86 :p
sources += Glob("thirdparty/opus/*.c") + Glob("thirdparty/opus/celt/*.c") + Glob("thirdparty/opus/silk/*.c") + Glob("thirdparty/opus/silk/float/*.c")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "addons/audiostreamplus/libaudiostreamplus.{}.{}.framework/libaudiostreamplus.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "addons/audiostreamplus/libaudiostreamplus{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
