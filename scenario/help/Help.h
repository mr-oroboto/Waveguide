#ifndef WAVEGUIDE_SCENARIO_HELP_H
#define WAVEGUIDE_SCENARIO_HELP_H

#include "Insight.h"

class Help : public insight::scenario::Scenario {
public:
    Help(insight::DisplayManager* display_manager, size_t screen_x_res, size_t screen_y_res);
    ~Help() = default;

    void run();

protected:
    std::shared_ptr<insight::Frame> frame_;

    size_t screen_x_res_;
    size_t screen_y_res_;

    static std::vector<std::string> options_;
};

#endif //WAVEGUIDE_SCENARIO_HELP_H
