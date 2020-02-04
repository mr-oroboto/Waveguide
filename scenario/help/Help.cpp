#include "Help.h"

#define VERTICAL_OFFSET 30.0f

Help::Help(DisplayManager *display_manager, size_t screen_x_res, size_t screen_y_res) :
        Scenario(display_manager),
        screen_x_res_(screen_x_res), screen_y_res_(screen_y_res)
{
}

Help::~Help()
{
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

    float current_y_offset = 140.0f;
    for (size_t i = 0; i < Help::options_.size(); i++)
    {
        current_y_offset += VERTICAL_OFFSET;
        frame_->addText(Help::options_[i], start_x, start_y - current_y_offset, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));
    }

    current_y_offset += 120.0f;

//    char msg[1024];
//    frame_->addText("Debug Information", start_x, start_y - current_y_offset, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));
//
//    glm::vec3 camera_coords = display_manager_->getCameraCoords();
//    glm::vec3 camera_pointing = display_manager_->getCameraPointingVector();
//
//    snprintf(msg, sizeof(msg), "Camera: (%.2f, %.2f, %2.f)", camera_coords.x, camera_coords.y, camera_coords.z);
//    frame_->addText(msg, start_x, start_y - current_y_offset - (VERTICAL_OFFSET), 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));

    frame_queue->enqueueFrame(frame_);

    frame_queue->setReady();
    frame_queue->setActive();    // transfer ownership to DisplayManager
}

std::vector<std::string> Help::options_ = {
        "h: This help screen",
        "n: Cycle to next perspective",
        "[]: Reduce / increase FFT resolution",
        "c: Clear max amplitude markers",
        "mouse: Select frequency range for zooming (in linear view only)",
        "u: Undo last zoom",
        "wsaf: Move camera forward / backward / left / right",
        "arrows: Point camera in different direction (can also use mouse)",
        "q: Quit"
};
