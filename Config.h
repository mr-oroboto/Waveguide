#ifndef WAVEGUIDE_CONFIG_H
#define WAVEGUIDE_CONFIG_H

#include <string>

#include <cstdint>

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
    float getGain();

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
    float gain_;

    static argp parser_;
    static argp_option options_[];
};


#endif //WAVEGUIDE_CONFIG_H
