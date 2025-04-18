#define DR_WAV_IMPLEMENTATION
#define DR_WAV_NO_STDIO
#include "audio_stream_wav_ext.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

int32_t AudioStreamPlaybackWavExt::_mix_resampled(AudioFrame *p_buffer, int p_frames) {
	if (!active) {
		return 0;
	}

	size_t left = p_frames;
	size_t total_mixed = 0;
	drwav_uint16 channels = flac_stream->wav->channels;
	while (total_mixed < p_frames) {
		int mixed = drwav_read_pcm_frames_f32(flac_stream->wav, channels, (float*)(&p_buffer[total_mixed]));
		ERR_FAIL_COND_V_EDMSG(mixed < 0, 0, vformat("Failed to mix Wave file. %d", mixed));

		if (mixed == 0) {
			if (!flac_stream->loop) {
				_stop();
				break;
			}

			_seek(flac_stream->loop_offset);
			loops++;
			continue;
		}

		if (channels == 1) {
			p_buffer[total_mixed].right = p_buffer[total_mixed].left;
		}

		left -= mixed;
		total_mixed += mixed;
	}

	frames_mixed += total_mixed;
	return total_mixed;
}

double AudioStreamPlaybackWavExt::_get_stream_sampling_rate() const {
	return flac_stream->sample_rate;
}

void AudioStreamPlaybackWavExt::_start(double p_from_pos) {
	active = true;
	_seek(p_from_pos);
	loops = 0;
	begin_resample();
}

void AudioStreamPlaybackWavExt::_stop() {
	active = false;
}

bool AudioStreamPlaybackWavExt::_is_playing() const {
	return active;
}

int AudioStreamPlaybackWavExt::_get_loop_count() const {
	return loops;
}

double AudioStreamPlaybackWavExt::_get_playback_position() const {
	return double(frames_mixed) / double(flac_stream->sample_rate);
}

void AudioStreamPlaybackWavExt::_seek(double p_time) {
	if (!active) {
		return;
	}

	if (flac_stream->wav == nullptr) {
		return;
	}

	if (p_time >= flac_stream->get_length()) {
		p_time = 0;
	}
	
	frames_mixed = flac_stream->sample_rate * p_time;
	drwav_seek_to_pcm_frame(flac_stream->wav, frames_mixed);
}

void AudioStreamPlaybackWavExt::_tag_used_streams() {
	// flac_stream->tag_used(_get_playback_position());
}

AudioStreamPlaybackWavExt::~AudioStreamPlaybackWavExt() {
	
}

void AudioStreamPlaybackWavExt::_bind_methods() {

}

Ref<AudioStreamPlayback> AudioStreamWavExt::_instantiate_playback() const {
	Ref<AudioStreamPlaybackWavExt> flacs;

	ERR_FAIL_COND_V_MSG(data.is_empty(), flacs,
			"This AudioStreamWavExt does not have an audio file assigned "
			"to it. AudioStreamWavExt should not be created from the "
			"inspector or with `.new()`. Instead, load an audio file.");

	flacs.instantiate();
	flacs->flac_stream = Ref<AudioStreamWavExt>(this);

	if (flacs->flac_stream->wav != nullptr) {
		drwav_free(flacs->flac_stream->wav, NULL);
	}

	flacs->flac_stream->wav = (drwav*)malloc(sizeof(drwav));
	ERR_FAIL_COND_V_MSG(drwav_init_memory(flacs->flac_stream->wav, data.ptr(), data.size(), NULL) == DRWAV_FALSE, flacs,
			"So basically we failed to parse shit womp womp!");

	flacs->frames_mixed = 0;
	flacs->active = false;
	flacs->loops = 0;

	return flacs;
}

String AudioStreamWavExt::_get_stream_name() const {
	return ""; //return stream_name;
}

void AudioStreamWavExt::clear_data() {
	data.clear();
}

void AudioStreamWavExt::set_data(const PackedByteArray &p_data) {
	if (!data.is_empty()) {
		clear_data();
	}

	data_len = p_data.size();
	data.resize(data_len);//AudioServer::get_singleton()->audio_data_alloc(src_data_len, src_datar.ptr());
	memcpy(data.ptrw(), p_data.ptr(), data_len);

	if (wav != nullptr) {
		drwav_free(wav, NULL);
	}

	wav = (drwav*)malloc(sizeof(drwav));
	ERR_FAIL_COND_MSG(drwav_init_memory(wav, data.ptr(), data.size(), NULL) == DRWAV_FALSE, "Oh yeah that file doesn't really work sorry uwu");

	channels = wav->channels;
	sample_rate = wav->sampleRate;
	length = float(wav->totalPCMFrameCount) / float(sample_rate);
}

PackedByteArray AudioStreamWavExt::get_data() const {
	return data;
}

void AudioStreamWavExt::set_loop(bool p_enable) {
	loop = p_enable;
}

bool AudioStreamWavExt::has_loop() const {
	return loop;
}

void AudioStreamWavExt::set_loop_offset(double p_seconds) {
	loop_offset = p_seconds;
}

double AudioStreamWavExt::get_loop_offset() const {
	return loop_offset;
}

double AudioStreamWavExt::_get_length() const {
	return length;
}

bool AudioStreamWavExt::_is_monophonic() const {
	return false;
}

void AudioStreamWavExt::set_bpm(double p_bpm) {
	ERR_FAIL_COND(p_bpm < 0);
	bpm = p_bpm;
	emit_changed();
}

double AudioStreamWavExt::_get_bpm() const {
	return bpm;
}

void AudioStreamWavExt::set_beat_count(int p_beat_count) {
	ERR_FAIL_COND(p_beat_count < 0);
	beat_count = p_beat_count;
	emit_changed();
}

int AudioStreamWavExt::_get_beat_count() const {
	return beat_count;
}

// void AudioStreamWavExt::set_bar_beats(int p_bar_beats) {
// 	ERR_FAIL_COND(p_bar_beats < 0);
// 	bar_beats = p_bar_beats;
// 	emit_changed();
// }

// int AudioStreamWavExt::_get_bar_beats() const {
// 	return bar_beats;
// }

Ref<AudioStreamWavExt> AudioStreamWavExt::load_from_file(const String &path) {
	ERR_FAIL_COND_V_MSG(!FileAccess::file_exists(path), Ref<AudioStreamWavExt>(), "File could not be found at path!");

	Ref<AudioStreamWavExt> stream;
	stream.instantiate();
	stream->set_data(FileAccess::get_file_as_bytes(path));
	return stream;
}

void AudioStreamWavExt::_bind_methods() {
	ClassDB::bind_static_method("AudioStreamWavExt", D_METHOD("load_from_file", "path"), &AudioStreamWavExt::load_from_file);

	ClassDB::bind_method(D_METHOD("set_data", "data"), &AudioStreamWavExt::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &AudioStreamWavExt::get_data);

	ClassDB::bind_method(D_METHOD("set_loop", "enable"), &AudioStreamWavExt::set_loop);
	ClassDB::bind_method(D_METHOD("has_loop"), &AudioStreamWavExt::has_loop);

	ClassDB::bind_method(D_METHOD("set_loop_offset", "seconds"), &AudioStreamWavExt::set_loop_offset);
	ClassDB::bind_method(D_METHOD("get_loop_offset"), &AudioStreamWavExt::get_loop_offset);

	ClassDB::bind_method(D_METHOD("set_bpm", "bpm"), &AudioStreamWavExt::set_bpm);
	ClassDB::bind_method(D_METHOD("get_bpm"), &AudioStreamWavExt::_get_bpm);

	ClassDB::bind_method(D_METHOD("set_beat_count", "count"), &AudioStreamWavExt::set_beat_count);
	ClassDB::bind_method(D_METHOD("get_beat_count"), &AudioStreamWavExt::_get_beat_count);

	// ClassDB::bind_method(D_METHOD("set_bar_beats", "count"), &AudioStreamWavExt::set_bar_beats);
	// ClassDB::bind_method(D_METHOD("get_bar_beats"), &AudioStreamWavExt::_get_bar_beats);

	// TODO: make these properties work in editor and save
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_data", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bpm", PROPERTY_HINT_RANGE, "0,400,0.01,or_greater", PROPERTY_USAGE_NO_EDITOR), "set_bpm", "get_bpm");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_count", PROPERTY_HINT_RANGE, "0,512,1,or_greater", PROPERTY_USAGE_NO_EDITOR), "set_beat_count", "get_beat_count");
	// ADD_PROPERTY(PropertyInfo(Variant::INT, "bar_beats", PROPERTY_HINT_RANGE, "2,32,1,or_greater"), "set_bar_beats", "get_bar_beats");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_loop", "has_loop");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "loop_offset", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_loop_offset", "get_loop_offset");
}

AudioStreamWavExt::AudioStreamWavExt() {
}

AudioStreamWavExt::~AudioStreamWavExt() {
	clear_data();

	if (wav != nullptr) {
		drwav_free(wav, NULL);
		wav = nullptr;
	}
}
