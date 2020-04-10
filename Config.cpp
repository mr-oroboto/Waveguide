#include "Config.h"

#include <cstdlib>

Config::Config(int argc, char** argv)
{
    device_prefix_ = "rtl";
    device_count_ = 1;

    start_frequency_ = 480000000;
    end_frequency_ = 600000000;

    sample_rate_ = 1920000;

    dwell_time_ = 5000000;

    averaging_window_ = 6;

    gain_ = 15.0f;
    enable_agc_ = true;

    enable_dc_spike_removal_ = true;

    font_path_ = "/usr/share/fonts/truetype/ttf-bitstream-vera";

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
        case 'a':
            enable_agc_ = strtoul(arg, NULL, 10);
            break;
        case 'x':
            enable_dc_spike_removal_ = strtoul(arg, NULL, 10);
            break;
        case 'p':
            device_prefix_ = std::string(arg);
            break;
        case 'c':
            device_count_ = static_cast<uint8_t>(strtoul(arg, NULL, 10));
            break;
        case 'w':
            averaging_window_ = static_cast<uint16_t>(strtoul(arg, NULL, 10));
            break;
        case 'f':
            font_path_ = std::string(arg);
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

bool Config::getAgc()
{
    return enable_agc_;
}

bool Config::getDcSpikeRemoval()
{
    return enable_dc_spike_removal_;
}

uint16_t Config::getAveragingWindow()
{
    return averaging_window_;
}

std::string Config::getFontPath()
{
    return font_path_;
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
        {"averaging_window", 'w', "COUNT", 0, "Number of samples to average FFT measurements over (default 4)", 1},
        {"dwell", 'd', "USEC", 0, "Dwell time per sampling slice in usec (default 500000 (0.5 sec))", 1},
        {"gain", 'g', "DB", 0, "Hardware gain (default 15.0)", 1},
        {"agc", 'a', "ON", 0, "Enable auto gain control (default 1 (on))", 1},
        {"despike", 'x', "ON", 0, "Enable DC spike removal (default 1 (on))", 1},
        {"device_prefix", 'p', "STRING", 0, "Device prefix as known by osmosdr (default 'rtl')", 1},
        {"device_count", 'c', "COUNT", 0, "Use this many hardware devices to scan range (default 1)", 1},
        {"font_path", 'f', "STRING", 0, "Full path (excluding trailing slash) to where TTF fonts are stored", 2},
        0
};

