#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#define DR_FLAC_NO_OGG
#include "audio_stream_flac.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

int32_t AudioStreamPlaybackFLAC::_mix_resampled(AudioFrame *p_buffer, int p_frames) {
	if (!active) {
		return 0;
	}

	int todo = p_frames;
	int start_buffer = 0;

	int frames_mixed_this_step = p_frames;

	int beat_length_frames = -1;
	bool beat_loop = flac_stream->has_loop() && flac_stream->_get_bpm() > 0 && flac_stream->_get_beat_count() > 0;
	if (beat_loop) {
		beat_length_frames = flac_stream->_get_beat_count() * flac_stream->sample_rate * 60 / flac_stream->_get_bpm();
	}
	
	while (todo && active) {
		float *buffer = (float *)p_buffer;
		if (start_buffer > 0) {
			buffer = (buffer + start_buffer * 2);
		}
		int mixed = drflac_read_pcm_frames_f32(flac_stream->pFlac, todo, buffer);
		for(int i = 0; i < mixed; i++){
			if (loop_fade_remaining < FADE_SIZE) {
				p_buffer[p_frames - todo].left += loop_fade[loop_fade_remaining].left * (float(FADE_SIZE - loop_fade_remaining) / float(FADE_SIZE));
				p_buffer[p_frames - todo].right += loop_fade[loop_fade_remaining].right * (float(FADE_SIZE - loop_fade_remaining) / float(FADE_SIZE));
				loop_fade_remaining++;
			}

			--todo;
			++frames_mixed;

			if (beat_loop && (int)frames_mixed >= beat_length_frames) {
				for (int fade_i = 0; fade_i < FADE_SIZE; fade_i++) {
					p_buffer[fade_i].left = buffer[fade_i * flac_stream->channels];
					p_buffer[fade_i].right =buffer[fade_i * flac_stream->channels + flac_stream->channels - 1];
				}
				loop_fade_remaining = 0;
				_seek(flac_stream->loop_offset);
				loops++;
			}
		}

		if (todo) {
			//end of file!
			if (flac_stream->loop) {
				//loop
				_seek(flac_stream->loop_offset);
				loops++;
				// we still have buffer to fill, start from this element in the next iteration.
				start_buffer = p_frames - todo;
			} else {
				frames_mixed_this_step = p_frames - todo;

				for (int i = p_frames - todo; i < p_frames; i++) {
					p_buffer[i].left = 0;
					p_buffer[i].right = 0;
				}
				active = false;
				todo = 0;
			}
		}
	}

	return frames_mixed_this_step;
	
}

double AudioStreamPlaybackFLAC::_get_stream_sampling_rate() const {

	return flac_stream->sample_rate;
}

void AudioStreamPlaybackFLAC::_start(double p_from_pos) {

	active = true;
	_seek(p_from_pos);
	loops = 0;
	begin_resample();
}

void AudioStreamPlaybackFLAC::_stop() {
	active = false;
}

bool AudioStreamPlaybackFLAC::_is_playing() const {
	return active;
}

int AudioStreamPlaybackFLAC::_get_loop_count() const {
	return loops;
}

double AudioStreamPlaybackFLAC::_get_playback_position() const {
	return double(frames_mixed) / double(flac_stream->sample_rate);
}

void AudioStreamPlaybackFLAC::_seek(double p_time) {
	if (!active) {
		return;
	}

	if (p_time >= flac_stream->get_length()) {
		p_time = 0;
	}
	frames_mixed = flac_stream->sample_rate * p_time;
	drflac_seek_to_pcm_frame(flac_stream->pFlac, frames_mixed);
}

void AudioStreamPlaybackFLAC::_tag_used_streams() {
	// flac_stream->tag_used(_get_playback_position());
}

AudioStreamPlaybackFLAC::~AudioStreamPlaybackFLAC() {
	
}

void AudioStreamPlaybackFLAC::_bind_methods() {

}

Ref<AudioStreamPlayback> AudioStreamFLAC::_instantiate_playback() const {
	Ref<AudioStreamPlaybackFLAC> flacs;

	ERR_FAIL_COND_V_MSG(data.is_empty(), flacs,
			"This AudioStreamFLAC does not have an audio file assigned "
			"to it. AudioStreamFLAC should not be created from the "
			"inspector or with `.new()`. Instead, load an audio file.");

	flacs.instantiate();
	flacs->flac_stream = Ref<AudioStreamFLAC>(this);

	flacs->flac_stream->pFlac = drflac_open_memory(data.ptr(), data_len, NULL);

	flacs->frames_mixed = 0;
	flacs->active = false;
	flacs->loops = 0;
	
	if (!flacs->flac_stream->pFlac) {
		ERR_FAIL_COND_V(!flacs->flac_stream->pFlac, Ref<AudioStreamPlaybackFLAC>());
	}

	return flacs;
}

String AudioStreamFLAC::_get_stream_name() const {
	return ""; //return stream_name;
}

void AudioStreamFLAC::clear_data() {
	data.clear();
}

void AudioStreamFLAC::set_data(const PackedByteArray &p_data) {
	clear_data();

	data_len = p_data.size();
	data.resize(data_len);//AudioServer::get_singleton()->audio_data_alloc(src_data_len, src_datar.ptr());
	memcpy(data.ptrw(), p_data.ptr(), data_len);

	if (pFlac != nullptr) {
		drflac_close(pFlac);
	}

	pFlac = drflac_open_memory(data.ptr(), data.size(), NULL);
	ERR_FAIL_COND(pFlac == nullptr);

	channels = pFlac->channels;
	sample_rate = pFlac->sampleRate;
	length = float(pFlac->totalPCMFrameCount) / float(sample_rate);
}

PackedByteArray AudioStreamFLAC::get_data() const {
	return data;
}

void AudioStreamFLAC::set_loop(bool p_enable) {
	loop = p_enable;
}

bool AudioStreamFLAC::has_loop() const {

	return loop;
}

void AudioStreamFLAC::set_loop_offset(double p_seconds) {
	loop_offset = p_seconds;
}

double AudioStreamFLAC::get_loop_offset() const {
	return loop_offset;
}

double AudioStreamFLAC::_get_length() const {
	return length;
}

bool AudioStreamFLAC::_is_monophonic() const {
	return false;
}

void AudioStreamFLAC::set_bpm(double p_bpm) {
	ERR_FAIL_COND(p_bpm < 0);
	bpm = p_bpm;
	emit_changed();
}

double AudioStreamFLAC::_get_bpm() const {
	return bpm;
}

void AudioStreamFLAC::set_beat_count(int p_beat_count) {
	ERR_FAIL_COND(p_beat_count < 0);
	beat_count = p_beat_count;
	emit_changed();
}

int AudioStreamFLAC::_get_beat_count() const {
	return beat_count;
}

// void AudioStreamFLAC::set_bar_beats(int p_bar_beats) {
// 	ERR_FAIL_COND(p_bar_beats < 0);
// 	bar_beats = p_bar_beats;
// 	emit_changed();
// }

// int AudioStreamFLAC::_get_bar_beats() const {
// 	return bar_beats;
// }

Ref<AudioStreamFLAC> AudioStreamFLAC::load_from_file(const String &path) {
	ERR_FAIL_COND_V_MSG(!FileAccess::file_exists(path), Ref<AudioStreamFLAC>(), "File could not be found at path!");

	Ref<AudioStreamFLAC> stream;
	stream.instantiate();
	stream->set_data(FileAccess::get_file_as_bytes(path));
	return stream;
}

void AudioStreamFLAC::_bind_methods() {
	ClassDB::bind_static_method("AudioStreamFLAC", D_METHOD("load_from_file", "path"), &AudioStreamFLAC::load_from_file);

	ClassDB::bind_method(D_METHOD("set_data", "data"), &AudioStreamFLAC::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &AudioStreamFLAC::get_data);

	ClassDB::bind_method(D_METHOD("set_loop", "enable"), &AudioStreamFLAC::set_loop);
	ClassDB::bind_method(D_METHOD("has_loop"), &AudioStreamFLAC::has_loop);

	ClassDB::bind_method(D_METHOD("set_loop_offset", "seconds"), &AudioStreamFLAC::set_loop_offset);
	ClassDB::bind_method(D_METHOD("get_loop_offset"), &AudioStreamFLAC::get_loop_offset);

	ClassDB::bind_method(D_METHOD("set_bpm", "bpm"), &AudioStreamFLAC::set_bpm);
	ClassDB::bind_method(D_METHOD("get_bpm"), &AudioStreamFLAC::_get_bpm);

	ClassDB::bind_method(D_METHOD("set_beat_count", "count"), &AudioStreamFLAC::set_beat_count);
	ClassDB::bind_method(D_METHOD("get_beat_count"), &AudioStreamFLAC::_get_beat_count);

	// ClassDB::bind_method(D_METHOD("set_bar_beats", "count"), &AudioStreamFLAC::set_bar_beats);
	// ClassDB::bind_method(D_METHOD("get_bar_beats"), &AudioStreamFLAC::_get_bar_beats);

	// TODO: make these properties work in editor and save
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_data", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bpm", PROPERTY_HINT_RANGE, "0,400,0.01,or_greater", PROPERTY_USAGE_NO_EDITOR), "set_bpm", "get_bpm");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_count", PROPERTY_HINT_RANGE, "0,512,1,or_greater", PROPERTY_USAGE_NO_EDITOR), "set_beat_count", "get_beat_count");
	// ADD_PROPERTY(PropertyInfo(Variant::INT, "bar_beats", PROPERTY_HINT_RANGE, "2,32,1,or_greater"), "set_bar_beats", "get_bar_beats");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_loop", "has_loop");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "loop_offset", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_loop_offset", "get_loop_offset");
}

AudioStreamFLAC::AudioStreamFLAC() {
}

AudioStreamFLAC::~AudioStreamFLAC() {
	clear_data();

	if (pFlac) {
		drflac_close(pFlac);
		pFlac = nullptr;
	}
}
