#include "audio_stream_opus.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

int32_t AudioStreamPlaybackOpus::_mix_resampled(AudioFrame *p_buffer, int p_frames) {
	if (!active) {
		return 0;
	}

	size_t left = p_frames;
	size_t total_mixed = 0;
	while (total_mixed < p_frames) {
		int mixed = op_read_float(opusFile, (float*)(&p_buffer[total_mixed]), left * 2, nullptr);
		ERR_FAIL_COND_V_EDMSG(mixed < 0, 0, vformat("Failed to mix Opus file. %d", mixed));

		if (mixed == 0) {
			if (!opus_stream->loop) {
				_stop();
				break;
			}

			_seek(opus_stream->loop_offset);
			loops++;
			continue;
		}

		left -= mixed;
		total_mixed += mixed;
	}

	frames_mixed += total_mixed;
	return total_mixed;
}

double AudioStreamPlaybackOpus::_get_stream_sampling_rate() const {
	return opus_stream->sample_rate;
}

void AudioStreamPlaybackOpus::_start(double p_from_pos) {
	active = true;
	_seek(p_from_pos);
	begin_resample();
}

void AudioStreamPlaybackOpus::_stop() {
	active = false;
}

bool AudioStreamPlaybackOpus::_is_playing() const {
	return active;
}

int AudioStreamPlaybackOpus::_get_loop_count() const {
	return loops;
}

double AudioStreamPlaybackOpus::_get_playback_position() const {
	return double(frames_mixed) / double(opus_stream->sample_rate);
}

void AudioStreamPlaybackOpus::_seek(double p_time) {
	if (!active) {
		return;
	}

	if (p_time >= opus_stream->get_length()) {
		p_time = 0;
	}

	frames_mixed = opus_stream->sample_rate * p_time;
	op_pcm_seek(opusFile, frames_mixed);
}

void AudioStreamPlaybackOpus::_tag_used_streams() {
	
}

AudioStreamPlaybackOpus::~AudioStreamPlaybackOpus() {
	if (opusFile) {
		op_free(opusFile);
		opusFile = nullptr;
	}
}

void AudioStreamPlaybackOpus::_bind_methods() {

}

Ref<AudioStreamPlayback> AudioStreamOpus::_instantiate_playback() const {
	Ref<AudioStreamPlaybackOpus> playback;
	ERR_FAIL_COND_V_MSG(data.is_empty(), playback,
			"This AudioStreamOpus does not have an audio file assigned "
			"to it. AudioStreamOpus should not be created from the "
			"inspector or with `.new()`. Instead, load an audio file.");

	playback.instantiate();
	playback->opus_stream = Ref<AudioStreamOpus>(this);

	playback->frames_mixed = 0;
	playback->active = false;

	int err = 0;
	playback->opusFile = op_open_memory(data.ptr(), data.size(), &err);
	ERR_FAIL_COND_V_EDMSG(err != 0, Ref<AudioStreamPlaybackOpus>(), vformat("Error loading Opus file! %d", err));
	ERR_FAIL_COND_V(playback->opusFile == nullptr, Ref<AudioStreamPlaybackOpus>());

	return playback;
}

String AudioStreamOpus::_get_stream_name() const {
	return ""; //return stream_name;
}

void AudioStreamOpus::clear_data() {
	data.clear();
}

void AudioStreamOpus::set_data(const PackedByteArray &p_data) {
	clear_data();

	data_len = p_data.size();
	data.resize(data_len);//AudioServer::get_singleton()->audio_data_alloc(src_data_len, src_datar.ptr());
	memcpy(data.ptrw(), p_data.ptr(), data_len);

	int err = 0;
	OggOpusFile *opusfile = op_open_memory(data.ptr(), data.size(), &err);
	ERR_FAIL_COND_EDMSG(err != 0, vformat("Error loading Opus file! %d", err));
	ERR_FAIL_COND(opusfile == nullptr);

	channels = op_channel_count(opusfile, -1);
	sample_rate = 48000;
	length = float(op_pcm_total(opusfile, -1)) / float(sample_rate);
}

PackedByteArray AudioStreamOpus::get_data() const {
	return data;
}

void AudioStreamOpus::set_loop(bool p_enable) {
	loop = p_enable;
}

bool AudioStreamOpus::has_loop() const {
	return loop;
}

void AudioStreamOpus::set_loop_offset(double p_seconds) {
	loop_offset = p_seconds;
}

double AudioStreamOpus::get_loop_offset() const {
	return loop_offset;
}

double AudioStreamOpus::_get_length() const {
	return length;
}

bool AudioStreamOpus::_is_monophonic() const {
	return false;
}

void AudioStreamOpus::set_bpm(double p_bpm) {
	ERR_FAIL_COND(p_bpm < 0);
	bpm = p_bpm;
	emit_changed();
}

double AudioStreamOpus::_get_bpm() const {
	return bpm;
}

void AudioStreamOpus::set_beat_count(int p_beat_count) {
	ERR_FAIL_COND(p_beat_count < 0);
	beat_count = p_beat_count;
	emit_changed();
}

int AudioStreamOpus::_get_beat_count() const {
	return beat_count;
}

// void AudioStreamOpus::set_bar_beats(int p_bar_beats) {
// 	ERR_FAIL_COND(p_bar_beats < 0);
// 	bar_beats = p_bar_beats;
// 	emit_changed();
// }

// int AudioStreamOpus::_get_bar_beats() const {
// 	return bar_beats;
// }

Ref<AudioStreamOpus> AudioStreamOpus::load_from_file(const String &path) {
	ERR_FAIL_COND_V_MSG(!FileAccess::file_exists(path), Ref<AudioStreamOpus>(), "File could not be found at path!");

	Ref<AudioStreamOpus> stream;
	stream.instantiate();
	stream->set_data(FileAccess::get_file_as_bytes(path));
	return stream;
}

void AudioStreamOpus::_bind_methods() {
	ClassDB::bind_static_method("AudioStreamOpus", D_METHOD("load_from_file", "path"), &AudioStreamOpus::load_from_file);

	ClassDB::bind_method(D_METHOD("set_data", "data"), &AudioStreamOpus::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &AudioStreamOpus::get_data);

	ClassDB::bind_method(D_METHOD("set_loop", "enable"), &AudioStreamOpus::set_loop);
	ClassDB::bind_method(D_METHOD("has_loop"), &AudioStreamOpus::has_loop);

	ClassDB::bind_method(D_METHOD("set_loop_offset", "seconds"), &AudioStreamOpus::set_loop_offset);
	ClassDB::bind_method(D_METHOD("get_loop_offset"), &AudioStreamOpus::get_loop_offset);

	ClassDB::bind_method(D_METHOD("set_bpm", "bpm"), &AudioStreamOpus::set_bpm);
	ClassDB::bind_method(D_METHOD("get_bpm"), &AudioStreamOpus::_get_bpm);

	ClassDB::bind_method(D_METHOD("set_beat_count", "count"), &AudioStreamOpus::set_beat_count);
	ClassDB::bind_method(D_METHOD("get_beat_count"), &AudioStreamOpus::_get_beat_count);

	// ClassDB::bind_method(D_METHOD("set_bar_beats", "count"), &AudioStreamOpus::set_bar_beats);
	// ClassDB::bind_method(D_METHOD("get_bar_beats"), &AudioStreamOpus::_get_bar_beats);

	// TODO: make these properties work in editor and save
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_data", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bpm", PROPERTY_HINT_RANGE, "0,400,0.01,or_greater", PROPERTY_USAGE_NO_EDITOR), "set_bpm", "get_bpm");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_count", PROPERTY_HINT_RANGE, "0,512,1,or_greater", PROPERTY_USAGE_NO_EDITOR), "set_beat_count", "get_beat_count");
	// ADD_PROPERTY(PropertyInfo(Variant::INT, "bar_beats", PROPERTY_HINT_RANGE, "2,32,1,or_greater"), "set_bar_beats", "get_bar_beats");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_loop", "has_loop");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "loop_offset", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_loop_offset", "get_loop_offset");
}

AudioStreamOpus::AudioStreamOpus() {
}

AudioStreamOpus::~AudioStreamOpus() {
	clear_data();
}
