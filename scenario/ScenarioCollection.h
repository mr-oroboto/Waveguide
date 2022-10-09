#ifndef WAVEGUIDE_SCENARIO_SCENARIOCOLLECTION_H
#define WAVEGUIDE_SCENARIO_SCENARIOCOLLECTION_H

#include "Insight.h"

class ScenarioCollection : public insight::scenario::ScenarioCollection {
public:
    // insight::InputHandler overrides
    void handleKeystroke(insight::WindowManager* window_manager, SDL_Event keystroke_event, GLfloat secs_since_last_renderloop) override;
    void handleMouse(insight::WindowManager* window_manager, SDL_Event mouse_event, GLfloat secs_since_last_renderloop) override;

private:
    void adjustCoalesceFactors(bool increase);
    void adjustMaxInterestMarkers(bool increase);
    void adjustMinInterestMarkingAmplitude(bool increase);
};


#endif //WAVEGUIDE_SCENARIO_SCENARIOCOLLECTION_H
