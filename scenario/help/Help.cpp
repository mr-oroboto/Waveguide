#include "Help.h"

Help::Help(DisplayManager *display_manager, size_t screen_x_res, size_t screen_y_res) :
        Scenario(display_manager),
        screen_x_res_(screen_x_res), screen_y_res_(screen_y_res)
{
}

Help::~Help()
{
    std::cout << "Help::~Help()" << std::endl;
}

void Help::run()
{
    FrameQueue* frame_queue = new FrameQueue(display_manager_, true);
    frame_queue->setFrameRate(1);

    frame_ = frame_queue->newFrame();

    GLfloat start_x = (screen_x_res_ / 2.0f) - 400.0f;
    GLfloat start_y = (screen_y_res_ / 2.0f) + 200.0f;

    frame_->addText("Waveguide Spectrum Visualiser", start_x, start_y, 0, true, 2.0, glm::vec3(1.0, 1.0, 1.0));
    frame_->addText("Usage Guide", start_x, start_y - 60.0f, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));
    frame_->addText("h: This help screen", start_x, start_y - 140.0f, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));
    frame_->addText("n: Cycle to next perspective", start_x, start_y - 170.0f, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));

    frame_queue->enqueueFrame(frame_);  // @todo we should use a shared pointer so we also retain ownership

    frame_queue->setReady();
    frame_queue->setActive();    // transfer ownership to DisplayManager
}