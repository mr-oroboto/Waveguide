#ifndef WAVEGUIDE_SCENARIO_HELP_H
#define WAVEGUIDE_SCENARIO_HELP_H

#include "Insight.h"

class Help : public Scenario
{
public:
    Help(DisplayManager* display_manager, size_t screen_x_res, size_t screen_y_res);
    virtual ~Help();

    void run();

protected:
    Frame* frame_;

    size_t screen_x_res_;
    size_t screen_y_res_;
};

#endif //WAVEGUIDE_SCENARIO_HELP_H
