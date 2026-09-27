
#include "mini_audio.h"

using namespace godot;

static float latest_sample = 0.0f; 

inline void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    MiniAudio* self = (MiniAudio*)pDevice->pUserData;
    if (!pInput) return;

    const float* input = (const float*)pInput;
    ma_uint32 channels = pDevice->capture.channels;
    ma_uint32 total = frameCount * channels;

    if (self->fft_buffer.size() != self->fft_size * channels)
        self->fft_buffer.resize(self->fft_size * channels, 0.0f);

    if (total < self->fft_buffer.size()) {
        memmove(self->fft_buffer.data(), self->fft_buffer.data() + total,
                (self->fft_buffer.size() - total) * sizeof(float));
        memcpy(self->fft_buffer.data() + (self->fft_buffer.size() - total),
               input, total * sizeof(float));
    } else {
        memcpy(self->fft_buffer.data(), input + (total - self->fft_buffer.size()),
               self->fft_buffer.size() * sizeof(float));
    }

    if (self->channel_peaks.size() != channels)
        self->channel_peaks.resize(channels, 0.0f);

    for (ma_uint32 ch = 0; ch < channels; ch++)
        self->channel_peaks[ch] = 0.0f;

    for (ma_uint32 i = 0; i < total; i++) {
        float v = fabsf(input[i]);
        int ch = i % channels;
        if (v > self->channel_peaks[ch]) self->channel_peaks[ch] = v;
    }

    latest_sample = *std::max_element(self->channel_peaks.begin(), self->channel_peaks.end());

    (void)pOutput;
}

void MiniAudio::_bind_methods(){
	ClassDB::bind_method(D_METHOD("start_audio"), &MiniAudio::start_audio);
	ClassDB::bind_method(D_METHOD("stop_audio"), &MiniAudio::stop_audio);

	ClassDB::bind_method(D_METHOD("get_sample"), &MiniAudio::get_sample);
	ClassDB::bind_method(D_METHOD("get_sample_int"), &MiniAudio::get_sample_int);
	ClassDB::bind_method(D_METHOD("set_microphone", "index"), &MiniAudio::set_microphone);
	ClassDB::bind_method(D_METHOD("get_microphone"), &MiniAudio::get_microphone);
	ClassDB::bind_method(D_METHOD("init_microphone"), &MiniAudio::init_microphone);

	ClassDB::bind_method(D_METHOD("get_sample_rate"), &MiniAudio::get_sample_rate);
	ClassDB::bind_method(D_METHOD("get_channels"), &MiniAudio::get_channels);
    ClassDB::bind_method(D_METHOD("get_device_name"), &MiniAudio::get_device_name);
    ClassDB::bind_method(D_METHOD("get_device_names"), &MiniAudio::get_device_names);

    ClassDB::bind_method(D_METHOD("get_magnitude", "a", "b", "mode"), &MiniAudio::get_magnitude);

}

int MiniAudio::get_sample_rate(){

    return device.sampleRate;
}

int MiniAudio::get_channels(){

    return device.capture.channels;
}

MiniAudio::MiniAudio(){
	init_microphone();
}

MiniAudio::~MiniAudio(){
    ma_device_uninit(&device);
}

void MiniAudio::start_audio(){
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        ma_device_uninit(&device);
        printf("Failed to start device.\n");
		return;
    }
}

float MiniAudio::get_sample() {
    return latest_sample;
}

int MiniAudio::get_sample_int() {
    return int(latest_sample * 100);
}

Vector2 MiniAudio::get_magnitude(float from_hz, float to_hz, int mode) {
    int channels = device.capture.channels;
    if (fft_buffer.size() < fft_size * channels) return Vector2(0, 0);

    int N = fft_size;
    int sample_rate = device.sampleRate;

    std::vector<float> channel_sums(channels, 0.0f);
    std::vector<int> channel_counts(channels, 0);
    std::vector<float> channel_max(channels, 0.0f);

    kiss_fft_cfg cfg = kiss_fft_alloc(N, 0, NULL, NULL);
    if (!cfg) return Vector2(0, 0);

    std::vector<kiss_fft_cpx> in(N);
    std::vector<kiss_fft_cpx> out(N);

    for (int ch = 0; ch < channels; ch++) {
        for (int i = 0; i < N; i++) {
            in[i].r = fft_buffer[i * channels + ch];
            in[i].i = 0.0f;
        }

        kiss_fft(cfg, in.data(), out.data());

        for (int k = 0; k < N / 2; k++) {
            float freq = (float)k * sample_rate / N;
            if (freq < from_hz || freq > to_hz) continue;

            float mag = sqrt(out[k].r * out[k].r + out[k].i * out[k].i);

            // Weight by current mic peak
            mag *= channel_peaks[ch];

            if (mode == 0) { 
                channel_sums[ch] += mag;
                channel_counts[ch]++;
            } else { // max
                if (mag > channel_max[ch]) channel_max[ch] = mag;
            }
        }
    }

    Vector2 result;
    if (mode == 0) {
        for (int ch = 0; ch < channels; ch++) {
            if (channel_counts[ch] > 0)
                channel_sums[ch] /= channel_counts[ch];
        }
        result.x = channel_sums[0];
        result.y = channels > 1 ? channel_sums[1] : channel_sums[0];
    } else {
        result.x = channel_max[0];
        result.y = channels > 1 ? channel_max[1] : channel_max[0];
    }

    std::free(cfg);
    return result;
}

void MiniAudio::stop_audio(){
	ma_device_stop(&device);
}

void MiniAudio::init_microphone(){

    deviceConfig = ma_device_config_init(ma_device_type_capture);

    deviceConfig.capture.format   = ma_format_f32;
    deviceConfig.capture.channels = 0;
    deviceConfig.sampleRate       = 0;
    deviceConfig.dataCallback     = data_callback;
    deviceConfig.pUserData        = this;

    if (selected_id != -1) {
        deviceConfig.capture.pDeviceID = &selected_device_id;
    }

    result = ma_device_init(NULL, &deviceConfig, &device);

    if (result != MA_SUCCESS){
        godot::print_line("Failed");
    }
}

void MiniAudio::set_microphone(int index){
    ma_context_init(NULL, 0, NULL, &context);

    ma_device_info* captureInfos;
    ma_uint32 captureCount;

    ma_context_get_devices(&context, NULL, NULL, &captureInfos, &captureCount);

    selected_device_id = captureInfos[index].id;
    selected_id = index;

    ma_context_uninit(&context);

    ma_device_uninit(&device);

    init_microphone();
}

void MiniAudio::clear_buffer(){
    
}

String MiniAudio::get_device_name(){
    char dev_name[256];
    size_t name_len = 0;

    ma_result err = ma_device_get_name(&device, ma_device_type_capture, dev_name, sizeof(dev_name), &name_len);

    if (err == MA_SUCCESS){
        return String(dev_name);
    }

    return "";
}

PackedStringArray MiniAudio::get_device_names() {
    PackedStringArray names;

    ma_context temp_context;
    ma_result result = ma_context_init(NULL, 0, NULL, &temp_context);
    if (result != MA_SUCCESS) {
        godot::print_line("Failed to init temporary context");
        return names;
    }

    ma_device_info* captureInfos = nullptr;
    ma_uint32 captureCount = 0;

    result = ma_context_get_devices(&temp_context, NULL, NULL, &captureInfos, &captureCount);
    if (result != MA_SUCCESS) {
        godot::print_line("Couldn't get capture devices");
        ma_context_uninit(&temp_context);
        return names;
    }

    for (ma_uint32 i = 0; i < captureCount; i++) {
        names.append(String(captureInfos[i].name));
    }

    ma_context_uninit(&temp_context);
    return names;
}

int MiniAudio::get_microphone(){
	return selected_id;
}
