#include "resource_format_loader_opus.h"
#include "audio_stream_opus.h"

using namespace godot;

ResourceFormatLoaderOpus::ResourceFormatLoaderOpus() {
}

ResourceFormatLoaderOpus::~ResourceFormatLoaderOpus() {
}

PackedStringArray ResourceFormatLoaderOpus::_get_recognized_extensions() const {
    PackedStringArray arr;
    arr.push_back("opus");
    return arr;
}

bool ResourceFormatLoaderOpus::_handles_type(const StringName &type) const {
    return type == StringName("AudioStreamOpus");
}

String ResourceFormatLoaderOpus::_get_resource_type(const String &path) const {
    if (path.get_extension() == "opus") {
        return "AudioStreamOpus";
    }

    return "";
}

bool ResourceFormatLoaderOpus::_exists(const String &path) const {
    return FileAccess::file_exists(path);
}

Variant ResourceFormatLoaderOpus::_load(const String &path, const String &original_path, bool use_sub_threads, int32_t cache_mode) const {
    Ref<AudioStreamOpus> stream;
    stream.instantiate();
    stream->set_data(FileAccess::get_file_as_bytes(path));
    return stream;
}

void ResourceFormatLoaderOpus::_bind_methods() {

}