#ifndef AUDIO_STREAM_WAV_EXT_H
#define AUDIO_STREAM_WAV_EXT_H

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/texture.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream_playback.hpp>
#include <godot_cpp/classes/audio_stream_playback_resampled.hpp>
#include "dr_wav.h"

namespace godot {

class AudioStreamWavExt;

class AudioStreamPlaybackWavExt : public AudioStreamPlaybackResampled {
	GDCLASS(AudioStreamPlaybackWavExt, AudioStreamPlaybackResampled);

	enum {
		FADE_SIZE = 256
	};
	AudioFrame loop_fade[FADE_SIZE];
	int loop_fade_remaining = FADE_SIZE;

	uint32_t frames_mixed = 0;
	bool active = false;
	int loops = 0;

	friend class AudioStreamWavExt;

	Ref<AudioStreamWavExt> flac_stream;

protected:
	static void _bind_methods();

public:

	virtual int32_t _mix_resampled(AudioFrame *p_buffer, int p_frames) override;
	virtual double _get_stream_sampling_rate() const override;

	virtual void _start(double p_from_pos = 0.0) override;
	virtual void _stop() override;
	virtual bool _is_playing() const override;

	virtual int _get_loop_count() const override; //times it looped

	virtual double _get_playback_position() const override;
	virtual void _seek(double p_time) override;

	virtual void _tag_used_streams() override;

	AudioStreamPlaybackWavExt() {}
	~AudioStreamPlaybackWavExt();
};

class AudioStreamWavExt : public AudioStream {

	GDCLASS(AudioStreamWavExt, AudioStream);

	friend class AudioStreamPlaybackWavExt;

	PackedByteArray data;
	uint64_t data_len = 0;

	double sample_rate = 1.0;
	int channels = 1;
	float length = 0.0;
	bool loop = false;
	float loop_offset = 0.0;
	void clear_data();

	double bpm = 0;
	int beat_count = 0;
	int bar_beats = 4;
	drwav *wav = nullptr;

protected:
	static void _bind_methods();

public:
	static Ref<AudioStreamWavExt> load_from_file(const String &path);
	static Ref<AudioStreamWavExt> load_from_buffer(const PackedByteArray &p_data);

	void set_loop(bool p_enable);
	bool has_loop() const;

	void set_loop_offset(double p_seconds);
	double get_loop_offset() const;

	void set_bpm(double p_bpm);
	virtual double _get_bpm() const override;

	void set_beat_count(int p_beat_count);
	virtual int _get_beat_count() const override;

	// void set_bar_beats(int p_bar_beats);
	// virtual int _get_bar_beats() const override;

	virtual Ref<AudioStreamPlayback> _instantiate_playback() const override;
	virtual String _get_stream_name() const override;

	void set_data(const PackedByteArray &p_data);
	PackedByteArray get_data() const;

	virtual double _get_length() const override; //if supported, otherwise return 0

	virtual bool _is_monophonic() const override;

	AudioStreamWavExt();
	virtual ~AudioStreamWavExt();
};

};

#endif
