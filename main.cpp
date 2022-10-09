#include <csignal>
#include <iostream>
#include <memory>
#include <string>

#include <unistd.h>

#include "Insight.h"
#include "sdr/SpectrumSampler.h"
#include "Config.h"
#include "scenario/ScenarioCollection.h"
#include "scenario/linear/LinearSpectrum.h"
#include "scenario/grid/GridSpectrum.h"
#include "scenario/sphere/SphereSpectrum.h"
#include "scenario/circular/CircularSpectrum.h"
#include "scenario/linear/LinearTimeSpectrum.h"
#include "scenario/cylindrical/CylindricalSpectrum.h"
#include "scenario/help/Help.h"

#define WINDOW_FULLSCREEN false
#define WINDOW_X_SIZE 950
#define WINDOW_Y_SIZE 600

ScenarioCollection scenarios;

static void signalHandler(int signal_number)
{
    switch (signal_number)
    {
        case SIGABRT:
        case SIGTERM:
        case SIGINT:
        case SIGHUP:
            exit(-1);
            break;
    }
}

void registerSignalHandlers()
{
    uint8_t signals[] = {
        SIGABRT,
        SIGTERM,
        SIGINT,
        SIGHUP
    };

    for (uint8_t i = 0; i < sizeof(signals) / sizeof(uint8_t); i++)
    {
        signal(signals[i], signalHandler);
    }
}

int main(int argc, char** argv)
{
    Config* config = nullptr;

    try
    {
        config = new Config(argc, argv);
    }
    catch (const char* error)
    {
        std::cerr << error << std::endl;
        return -1;
    }

    registerSignalHandlers();

    auto sampler =  new sdr::SpectrumSampler(config);
    sampler->start(config->getStartFrequency(), config->getEndFrequency());

    auto window_manager = new insight::WindowManager(WINDOW_X_SIZE, WINDOW_Y_SIZE, WINDOW_FULLSCREEN);
    if ( ! window_manager->initialise())
    {
        std::cerr << "Failed to initialise WindowManager" << std::endl;
        return -1;
    }

    insight::DisplayManager* display_manager = window_manager->getDisplayManager();
    std::string font_path = config->getFontPath().append("/Vera.ttf");
    if ( ! display_manager->registerFont(insight::Font::Type::FONT_DEFAULT, font_path))
    {
        std::cerr << "Failed to register font" << std::endl;
        return -1;
    }

    scenarios.initialise(window_manager);

    scenarios.addScenario(new Help(display_manager, WINDOW_X_SIZE, WINDOW_Y_SIZE));
    scenarios.addScenario(new LinearSpectrum(window_manager, sampler, 1000));
    scenarios.addScenario(new LinearTimeSpectrum(window_manager, sampler, 1000));
    scenarios.addScenario(new GridSpectrum(window_manager, sampler, 100));
    scenarios.addScenario(new SphereSpectrum(window_manager, sampler, 600));
    scenarios.addScenario(new CylindricalSpectrum(window_manager, sampler, 600));
    scenarios.addScenario(new CircularSpectrum(window_manager, sampler, 80));

    scenarios.nextScenario();

    window_manager->run();

    sampler->stop();

    return 0;
}
