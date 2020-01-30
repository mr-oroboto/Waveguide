#include "Config.h"

#include <cstdlib>

Config::Config(int argc, char** argv)
{
    device_prefix_ = "rtl";
    device_count_ = 1;

    start_frequency_ = 88000000;
    end_frequency_ = 108000000;
    sample_rate_ = 2400000;
    dwell_time_ = 500000;
    gain_ = 10.0f;

    argp_parse(&parser_, argc, argv, 0, 0, this);

    validateOptions();
}

error_t Config::parse_argument(int key, char *arg, struct argp_state *state)
{
    Config* config = reinterpret_cast<Config*>(state->input);

    return config->parse(key, arg);
}

error_t Config::parse(int key, char* arg)
{
    switch (key)
    {
        case 's':
            start_frequency_ = strtoull(arg, NULL, 10);
            break;
        case 'e':
            end_frequency_ = strtoull(arg, NULL, 10);
            break;
        case 'r':
            sample_rate_ = strtoull(arg, NULL, 10);
            break;
        case 'd':
            dwell_time_ = strtoul(arg, NULL, 10);
            break;
        case 'g':
            gain_ = atof(arg);
            break;
        case 'c':
            device_count_ = static_cast<uint8_t>(strtoul(arg, NULL, 10));
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

void Config::validateOptions()
{
    if (end_frequency_ <= start_frequency_)
    {
        throw "End frequency must be greater than start frequency";
    }

    if (sample_rate_ < 240000)
    {
        throw "Sample rate must greater than or equal to 240000Hz";
    }

    if (dwell_time_ < 100000)
    {
        throw "Dwell time must be greater than or equal to 100000 (0.1 sec)";
    }

    if (gain_ < 0)
    {
        throw "Gain must be greater than or equal to 0.0";
    }
}

std::string Config::getDevicePrefix()
{
    return device_prefix_;
}

uint8_t Config::getDeviceCount()
{
    return device_count_;
}

uint64_t Config::getSampleRate()
{
    return sample_rate_;
}

uint64_t Config::getStartFrequency()
{
    return start_frequency_;
}

uint64_t Config::getEndFrequency()
{
    return end_frequency_;
}

uint32_t Config::getDwellTime()
{
    return dwell_time_;
}

float Config::getGain()
{
    return gain_;
}

argp Config::parser_ = {
        options_,
        parse_argument,
        0,
        0
};

argp_option Config::options_[] = {
        {"start", 's', "FREQUENCY", 0, "Start scanning at this frequency in Hz (default 88000000 (88Mhz))", 0},
        {"end", 'e', "FREQUENCY", 0, "End scanning at this frequency in Hz (default 108000000 (108Mhz)", 0},
        {"sample_rate", 'r', "RATE", 0, "Hardware sample rate in Hz (default 2400000Hz (2.4Mhz))", 1},
        {"dwell", 'd', "USEC", 0, "Dwell time per sampling slice in usec (default 500000 (0.5 sec))", 1},
        {"gain", 'g', "DB", 0, "Hardware gain (default 10.0)", 1},
        {"device_count", 'c', "COUNT", 0, "Use this many hardware devices to scan range (default 1)", 1},
        0
};

