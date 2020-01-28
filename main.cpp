#include <iostream>
#include <csignal>

#include <unistd.h>

#include "Insight.h"
#include "sdr/SpectrumSampler.h"
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

bool run = false;
ScenarioCollection scenarios;

bool handleKeystroke(WindowManager* window_manager, SDL_Event keystroke_event, GLfloat secs_since_last_renderloop)
{
    bool continue_processing_keystrokes = true;

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
        }
    }

    return continue_processing_keystrokes;
}

static void signal_handler(int signal_number)
{
    switch (signal_number)
    {
        case SIGINT:
        case SIGTERM:
        case SIGHUP:
            run = false;
            break;

        default:
            std::cerr << "Received signal: " << signal_number << std::endl;
            break;
    }
}

int main()
{
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        std::cerr << "WARN: Can't register SIGINT handler" << std::endl;
    }

    if (signal(SIGTERM, signal_handler) == SIG_ERR)
    {
        std::cerr << "WARN: Can't register SIGINT handler" << std::endl;
    }

    if (signal(SIGHUP, signal_handler) == SIG_ERR)
    {
        std::cerr << "WARN: Can't register SIGINT handler" << std::endl;
    }

    sdr::SpectrumSampler* sampler =  new sdr::SpectrumSampler(1, 2400000);
    sampler->start(88000000, 108000000);

    WindowManager window_manager(WINDOW_X_SIZE, WINDOW_Y_SIZE, WINDOW_FULLSCREEN, glm::vec3(0, 0, 80));

    if ( ! window_manager.initialise())
    {
        std::cerr << "Failed to initialise WindowManager" << std::endl;
        return -1;
    }

    DisplayManager* display_manager = window_manager.getDisplayManager();

    if ( ! display_manager->getTextDrawer()->registerFont(Font::Type::FONT_DEFAULT, "/home/sysop/ClionProjects/Insight/font/Vera.ttf"))
    {
        std::cerr << "Failed to register font" << std::endl;
        return -1;
    }

    scenarios.initialise(&window_manager);

    scenarios.addScenario(new Help(display_manager, WINDOW_X_SIZE, WINDOW_Y_SIZE));
    scenarios.addScenario(new LinearSpectrum(display_manager, sampler, 1000));
    scenarios.addScenario(new LinearTimeSpectrum(display_manager, sampler, 1000));
    scenarios.addScenario(new GridSpectrum(display_manager, sampler, 100));
    scenarios.addScenario(new SphereSpectrum(display_manager, sampler, 600));
    scenarios.addScenario(new CylindricalSpectrum(display_manager, sampler, 600));
    scenarios.addScenario(new CircularSpectrum(display_manager, sampler, 80));

    window_manager.setHandleKeystrokeCallback(std::bind(&handleKeystroke, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    scenarios.nextScenario();

    window_manager.run();

    sampler->stop();

    return 0;
}