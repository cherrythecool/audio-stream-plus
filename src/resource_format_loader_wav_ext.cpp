#include "resource_format_loader_wav_ext.h"
#include "audio_stream_wav_ext.h"

using namespace godot;

ResourceFormatLoaderWavExt::ResourceFormatLoaderWavExt() {
}

ResourceFormatLoaderWavExt::~ResourceFormatLoaderWavExt() {
}

PackedStringArray ResourceFormatLoaderWavExt::_get_recognized_extensions() const {
    PackedStringArray arr;
    arr.push_back("wavext");
    return arr;
}

bool ResourceFormatLoaderWavExt::_handles_type(const StringName &type) const {
    return type == StringName("AudioStreamWavExt");
}

String ResourceFormatLoaderWavExt::_get_resource_type(const String &path) const {
    if (path.get_extension() == "wavext") {
        return "AudioStreamWavExt";
    }

    return "";
}

bool ResourceFormatLoaderWavExt::_exists(const String &path) const {
    return FileAccess::file_exists(path);
}

Variant ResourceFormatLoaderWavExt::_load(const String &path, const String &original_path, bool use_sub_threads, int32_t cache_mode) const {
    return AudioStreamWavExt::load_from_file(path);
}

void ResourceFormatLoaderWavExt::_bind_methods() {

}