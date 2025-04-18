#include "register_types.h"

#include "audio_stream_flac.h"
#include "resource_format_loader_flac.h"

#include "audio_stream_opus.h"
#include "resource_format_loader_opus.h"

#include "audio_stream_wav_ext.h"
#include "resource_format_loader_wav_ext.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

using namespace godot;

static Ref<ResourceFormatLoaderFLAC> resource_loader_flac;
static Ref<ResourceFormatLoaderOpus> resource_loader_opus;
static Ref<ResourceFormatLoaderWavExt> resource_loader_wav_ext;

void initialize_audio_modules(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    // TODO: implement resourceFormatSaver/ ResourceFormatLoader, otherwise importer will not work
    ClassDB::register_class<AudioStreamFLAC>();
    ClassDB::register_class<AudioStreamPlaybackFLAC>();
    ClassDB::register_class<ResourceFormatLoaderFLAC>();
    resource_loader_flac.instantiate();
    ResourceLoader::get_singleton()->add_resource_format_loader(resource_loader_flac);

    // TODO: implement resourceFormatSaver/ ResourceFormatLoader, otherwise importer will not work
    ClassDB::register_class<AudioStreamOpus>();
    ClassDB::register_class<AudioStreamPlaybackOpus>();
    ClassDB::register_class<ResourceFormatLoaderOpus>();
    resource_loader_opus.instantiate();
    ResourceLoader::get_singleton()->add_resource_format_loader(resource_loader_opus);

    // TODO: implement resourceFormatSaver/ ResourceFormatLoader, otherwise importer will not work
    ClassDB::register_class<AudioStreamWavExt>();
    ClassDB::register_class<AudioStreamPlaybackWavExt>();
    ClassDB::register_class<ResourceFormatLoaderWavExt>();
    resource_loader_wav_ext.instantiate();
    ResourceLoader::get_singleton()->add_resource_format_loader(resource_loader_wav_ext);
}

void uninitialize_audio_modules(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ResourceLoader::get_singleton()->remove_resource_format_loader(resource_loader_flac);
    resource_loader_flac.unref();

    ResourceLoader::get_singleton()->remove_resource_format_loader(resource_loader_opus);
    resource_loader_opus.unref();

    ResourceLoader::get_singleton()->remove_resource_format_loader(resource_loader_wav_ext);
    resource_loader_wav_ext.unref();
}

extern "C" {

// Initialization.
GDExtensionBool GDE_EXPORT asp_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_audio_modules);
    init_obj.register_terminator(uninitialize_audio_modules);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_EDITOR);

    return init_obj.init();
}

}