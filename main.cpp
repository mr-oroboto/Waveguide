#include <csignal>
#include <iostream>
#include <memory>
#include <string>

#include <unistd.h>

#include "Insight.h"
#include "sdr/SpectrumSampler.h"
#include "Config.h"
#include "scenario/linear/LinearSpectrum.h"
#include "scenario/grid/GridSpectrum.h"
#include "scenario/sphere/SphereSpectrum.h"
#include "scenario/circular/CircularSpectrum.h"
#include "scenario/linear/LinearTimeSpectrum.h"
#include "scenario/cylindrical/CylindricalSpectrum.h"
#include "scenario/help/Help.h"

#define WINDOW_FULLSCREEN false
#define WINDOW_X_SIZE 2560
#define WINDOW_Y_SIZE 1440

ScenarioCollection scenarios;

void adjustCoalesceFactors(SimpleSpectrum* scenario, bool increase)
{
    uint32_t coalesce_factor = scenario->getCoalesceFactor();
    scenario->setCoalesceFactor(increase ? coalesce_factor * 2 : coalesce_factor / 2);

    scenarios.nextScenario();
    scenarios.previousScenario();
}

void adjustMaxInterestMarkers(SimpleSpectrum* scenario, bool increase)
{
    uint64_t max_markers = scenario->getMaxInterestMarkers();
    scenario->setMaxInterestMarkers(increase ? ++max_markers : --max_markers);

    scenarios.nextScenario();
    scenarios.previousScenario();
}

void adjustMinInterestMarkingAmplitude(SimpleSpectrum* scenario, bool increase)
{
    float min_amplitude = scenario->getMinInterestMarkingAmplitude();
    scenario->setMinInterestMarkingAmplitude(increase ? min_amplitude + 1.0f : min_amplitude - 1.0f);

    scenarios.nextScenario();
    scenarios.previousScenario();
}

bool handleKeystroke(WindowManager* window_manager, SDL_Event keystroke_event, GLfloat secs_since_last_renderloop)
{
    bool continue_processing_keystrokes = true;
    SimpleSpectrum* scenario = dynamic_cast<SimpleSpectrum*>(scenarios.getCurrentScenario());

    if (keystroke_event.type == SDL_KEYDOWN)
    {
        switch (keystroke_event.key.keysym.sym)
        {
            case SDLK_n:
                scenarios.nextScenario();
                break;

            case SDLK_h:
                scenarios.selectScenario(0);
                break;

            case SDLK_RIGHTBRACKET:
                adjustCoalesceFactors(scenario, false);
                break;
            case SDLK_LEFTBRACKET:
                adjustCoalesceFactors(scenario, true);
                break;

            case SDLK_o:
                adjustMaxInterestMarkers(scenario, false);
                break;
            case SDLK_p:
                adjustMaxInterestMarkers(scenario, true);
                break;

            case SDLK_COMMA:
                adjustMinInterestMarkingAmplitude(scenario, false);
                break;
            case SDLK_PERIOD:
                adjustMinInterestMarkingAmplitude(scenario, true);
                break;

            case SDLK_u:
                scenario->undoLastZoom();
                break;

            case SDLK_c:
                scenario->clearInterestMarkers();
                break;
        }
    }

    return continue_processing_keystrokes;
}

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

    sdr::SpectrumSampler* sampler =  new sdr::SpectrumSampler(config);
    sampler->start(config->getStartFrequency(), config->getEndFrequency());

    std::unique_ptr<WindowManager> window_manager1 = std::make_unique<WindowManager>(WINDOW_X_SIZE, WINDOW_Y_SIZE, WINDOW_FULLSCREEN, glm::vec3(0, 0, 80));
    WindowManager* window_manager = new WindowManager(WINDOW_X_SIZE, WINDOW_Y_SIZE, WINDOW_FULLSCREEN, glm::vec3(0, 0, 80));
    if ( ! window_manager->initialise())
    {
        std::cerr << "Failed to initialise WindowManager" << std::endl;
        return -1;
    }

    DisplayManager* display_manager = window_manager->getDisplayManager();
    std::string font_path = config->getFontPath().append("/Vera.ttf");
    if ( ! display_manager->registerFont(Font::Type::FONT_DEFAULT, font_path))
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

    window_manager->setHandleKeystrokeCallback(std::bind(&handleKeystroke, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    scenarios.nextScenario();

    window_manager->run();

    sampler->stop();

    return 0;
}