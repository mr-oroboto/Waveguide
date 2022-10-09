#include "ScenarioCollection.h"

#include "SimpleSpectrum.h"

void ScenarioCollection::adjustCoalesceFactors(bool increase)
{
    SimpleSpectrum* scenario = dynamic_cast<SimpleSpectrum*>(getCurrentScenario());

    uint32_t coalesce_factor = scenario->getCoalesceFactor();
    scenario->setCoalesceFactor(increase ? coalesce_factor * 2 : coalesce_factor / 2);

    nextScenario();
    previousScenario();
}

void ScenarioCollection::adjustMaxInterestMarkers(bool increase)
{
    SimpleSpectrum* scenario = dynamic_cast<SimpleSpectrum*>(getCurrentScenario());

    uint64_t max_markers = scenario->getMaxInterestMarkers();
    scenario->setMaxInterestMarkers(increase ? ++max_markers : --max_markers);

    nextScenario();
    previousScenario();
}

void ScenarioCollection::adjustMinInterestMarkingAmplitude(bool increase)
{
    SimpleSpectrum* scenario = dynamic_cast<SimpleSpectrum*>(getCurrentScenario());

    float min_amplitude = scenario->getMinInterestMarkingAmplitude();
    scenario->setMinInterestMarkingAmplitude(increase ? min_amplitude + 1.0f : min_amplitude - 1.0f);

    nextScenario();
    previousScenario();
}

void ScenarioCollection::handleKeystroke(insight::WindowManager* window_manager, SDL_Event keystroke_event, GLfloat secs_since_last_renderloop)
{
    SimpleSpectrum* scenario = dynamic_cast<SimpleSpectrum*>(getCurrentScenario());

    if (keystroke_event.type == SDL_KEYDOWN)
    {
        switch (keystroke_event.key.keysym.sym)
        {
            case SDLK_h:
                selectScenario(0);
                break;

            case SDLK_RIGHTBRACKET:
                adjustCoalesceFactors(false);
                break;
            case SDLK_LEFTBRACKET:
                adjustCoalesceFactors(true);
                break;

            case SDLK_o:
                adjustMaxInterestMarkers(false);
                break;
            case SDLK_p:
                adjustMaxInterestMarkers(true);
                break;

            case SDLK_COMMA:
                adjustMinInterestMarkingAmplitude(false);
                break;
            case SDLK_PERIOD:
                adjustMinInterestMarkingAmplitude(true);
                break;

            case SDLK_u:
                scenario->undoLastZoom();
                break;

            case SDLK_c:
                scenario->clearInterestMarkers();
                break;
        }
    }

    // Call parent class implementation (which handles scenario cycling).
    insight::scenario::ScenarioCollection::handleKeystroke(window_manager, keystroke_event, secs_since_last_renderloop);
}

void ScenarioCollection::handleMouse(insight::WindowManager* window_manager, SDL_Event mouse_event, GLfloat secs_since_last_renderloop)
{
    // Call parent class implementation.
    insight::scenario::ScenarioCollection::handleMouse(window_manager, mouse_event, secs_since_last_renderloop);
}
