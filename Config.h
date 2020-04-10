#ifndef WAVEGUIDE_CONFIG_H
#define WAVEGUIDE_CONFIG_H

#include <cstdint>
#include <string>

#include <argp.h>

class Config {
public:
    Config(int argc, char** argv);

    std::string getDevicePrefix();
    uint8_t getDeviceCount();

    uint64_t getSampleRate();
    uint64_t getStartFrequency();
    uint64_t getEndFrequency();

    uint32_t getDwellTime();
    uint16_t getAveragingWindow();

    float getGain();
    bool getAgc();

    bool getDcSpikeRemoval();

    std::string getFontPath();

private:
    static error_t parse_argument(int key, char *arg, struct argp_state* state);
    error_t parse(int key, char *arg);

    void validateOptions();

    uint8_t device_count_;
    std::string device_prefix_;

    uint64_t sample_rate_;
    uint64_t start_frequency_;
    uint64_t end_frequency_;

    uint32_t dwell_time_;
    uint16_t averaging_window_;

    float gain_;
    bool enable_agc_;

    bool enable_dc_spike_removal_;

    std::string font_path_;

    static argp parser_;
    static argp_option options_[];
};


#endif //WAVEGUIDE_CONFIG_H
