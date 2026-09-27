#ifndef MINI_AUDIO_HPP
#define MINI_AUDIO_HPP

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/geometry2d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <mutex>
#include <algorithm>

#include <stdlib.h>
#include <stdio.h>
#include "miniaudio.h"
#include "kiss_fft.h"

using namespace godot;

class MiniAudio : public Node2D {
	GDCLASS(MiniAudio, Node2D);

protected:
	static void _bind_methods();


public:
    std::vector<float> fft_buffer;
    int fft_size = 512;
    std::vector<float> channel_peaks;
	MiniAudio();
	~MiniAudio();

    void start_audio();
    void stop_audio();
    float get_sample();
    Vector2 get_magnitude(float from_hz, float to_hz, int mode);
    int get_sample_int();
    void set_microphone(int index);
    int get_microphone();
    int get_sample_rate();
    int get_channels();
    String get_device_name();
    PackedStringArray get_device_names();

    void clear_buffer();



    void init_microphone();
private:
    bool has_device = false;
    ma_context context;
    ma_result result;
    ma_device_config deviceConfig;
    ma_device device;
    int selected_id = -1;
    ma_device_id selected_device_id;
};

#endif 
