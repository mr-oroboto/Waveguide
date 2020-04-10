#include "LinearSpectrum.h"

#include <iostream>

#include <scenario/SimpleSpectrum.h>

LinearSpectrum::LinearSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor)
        : SimpleSpectrum(window_manager, sampler, bin_coalesce_factor)
{
    start_picking_bin_ = nullptr;
    last_picked_bin_ = nullptr;

    max_freq_markers_ = 4;
}

void LinearSpectrum::run()
{
    /**
     * 1. Create a new repeating FrameQueue
     * 2. Create a single Frame that will be reused over and over
     * 3. Place initial SceneObjects in the frame
     * 4. Register a callback to be used to update SceneObjects
     * 5. Run the FrameQueue
     */
    resetState();

    window_manager_->setHandleMouseCallback(std::bind(&LinearSpectrum::handleMouse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    display_manager_->setPerspective(0.1f, 100.0f, 45.0f);

    FrameQueue* frame_queue = new FrameQueue(display_manager_, true);
    frame_queue->setFrameRate(1);

    frame_ = frame_queue->newFrame(false);

    uint64_t raw_bin_count = samples_->getBinCount();
    uint64_t coalesced_bin_count = (raw_bin_count + bin_coalesce_factor_ - 1) / bin_coalesce_factor_; // integer ceiling

    std::cout << "Coalescing " << raw_bin_count << " frequency bins into " << coalesced_bin_count << " visual bins" << std::endl;

    uint64_t marker_spacing = coalesced_bin_count / max_freq_markers_;
    glm::vec3 start_coords = glm::vec3(-1.0f * ((coalesced_bin_count * bin_width_) / 2.0f), 0, 0);
    if (marker_spacing == 0)
    {
        marker_spacing = 2;
    }

    for (uint64_t bin_id = 0; bin_id < coalesced_bin_count; bin_id++)
    {
        // Coalesce the frequency bins
        std::vector<sdr::FrequencyBin const*> frequency_bins;
        uint64_t start_frequency_bin = bin_id * bin_coalesce_factor_;
        for (uint64_t j = start_frequency_bin; j < (start_frequency_bin + bin_coalesce_factor_) && j < raw_bin_count; j++)
        {
            frequency_bins.push_back(samples_->getFrequencyBin(j));
        }

        glm::vec3 world_coords = start_coords;
        world_coords.x += (bin_id * bin_width_);

        SimpleSpectrumRange* bin = new SimpleSpectrumRange(display_manager_, Primitive::Type::RECTANGLE, 0, bin_id, world_coords, glm::vec3(1, 1, 1), frequency_bins);
        bin->setScale(bin_width_, 1.0, 1.0);

        coalesced_bins_.push_back(bin);
        frame_->addObject(bin);

        if (bin_id % marker_spacing == 0)
        {
            char msg[64];
            snprintf(msg, sizeof(msg), "%.3fMHz", const_cast<sdr::FrequencyBin*>(frequency_bins[0])->getFrequency() / 1000000.0f);
            frame_->addText(msg, world_coords.x, -2.0f, world_coords.z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));
        }
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "Linear Perspective (%.3fMhz - %.3fMhz)", sampler_->getStartFrequency() / 1000000.0f, sampler_->getEndFrequency() / 1000000.0f);
    frame_->addText(msg, 10, 10, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));

    frame_queue->enqueueFrame(frame_);  // @todo we should use a shared pointer so we also retain ownership

    display_manager_->setUpdateSceneCallback(std::bind(&LinearSpectrum::updateSceneCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    frame_queue->setReady();
    frame_queue->setActive();    // transfer ownership to DisplayManager
}

void LinearSpectrum::updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame)
{
    uint16_t current_slice = 0;

    if (samples_->getSweepCount() && current_interest_markers_ < max_interest_markers_)
    {
        markLocalMaxima();
    }

    frame_->updateObjects(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, static_cast<void*>(&current_slice));
}

void LinearSpectrum::clearInterestMarkers()
{
    if (frame_ == nullptr)
    {
        return;
    }

    for (unsigned long i : marked_bin_text_ids_)
    {
        frame_->deleteText(i);
    }

    marked_bin_text_ids_.clear();
    SimpleSpectrum::clearInterestMarkers();
}

void LinearSpectrum::addInterestMarkerToBin(SimpleSpectrumRange *bin)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "%.3fMHz", bin->getFrequency() / 1000000.0f);
    unsigned long text_id = frame_->addText(msg, bin->getPosition().x, bin->getAmplitude() + 0.2f, bin->getPosition().z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));

    marked_bin_text_ids_.push_back(text_id);

    SimpleSpectrum::addInterestMarkerToBin(bin);
}

bool LinearSpectrum::handleMouse(WindowManager* window_manager, SDL_Event mouse_event, GLfloat secs_since_last_renderloop)
{
    if (mouse_event.type == SDL_MOUSEBUTTONDOWN && mouse_event.button.button == SDL_BUTTON_LEFT)
    {
        start_picking_bin_ = findFirstIntersectedBin(mouse_event.motion.x, mouse_event.motion.y);
        last_picked_bin_ = nullptr;

        if (start_picking_bin_)
        {
            highlightPickedBins(start_picking_bin_);
            return false;
        }
    }
    else if (start_picking_bin_ && mouse_event.type == SDL_MOUSEMOTION)
    {
        SimpleSpectrumRange* end_picking_bin = findFirstIntersectedBin(mouse_event.motion.x, mouse_event.motion.y);

        if (end_picking_bin)
        {
            highlightPickedBins(end_picking_bin);
            return false;
        }
    }
    else if (start_picking_bin_ && mouse_event.type == SDL_MOUSEBUTTONUP)
    {
        if (last_picked_bin_ && last_picked_bin_ != start_picking_bin_)
        {
            // Save the current range so we can return to it
            ZoomRange current_range = {
                    sampler_->getStartFrequency(),
                    sampler_->getEndFrequency()
            };
            previous_zoom_ranges_.push(current_range);

            uint64_t start_freq_hz = start_picking_bin_->getBinId() < last_picked_bin_->getBinId() ? start_picking_bin_->getFrequency() : last_picked_bin_->getFrequency();
            uint64_t end_freq_hz = start_picking_bin_->getBinId() < last_picked_bin_->getBinId() ? last_picked_bin_->getFrequency() : start_picking_bin_->getFrequency();

            max_freq_markers_ = 2;

            retune(start_freq_hz, end_freq_hz, true);
        }

        start_picking_bin_ = nullptr;
        last_picked_bin_ = nullptr;
    }

    return true;
}

SimpleSpectrumRange* LinearSpectrum::findFirstIntersectedBin(GLuint mouse_x, GLuint mouse_y)
{
    glm::vec3 ray_start_coords = window_manager_->getDisplayManager()->getCameraCoords();

    /**
     * NOTE: This is a terrible and naive intersection test, please improve it.
     */
    for (SimpleSpectrumRange* bin : coalesced_bins_)
    {
        glm::vec3 pos = bin->getPosition();
        glm::vec3 scale = bin->getScale();

        glm::vec3 top = pos, bottom = pos, left = pos, right = pos;
        top.y += scale.y / 2.0f;
        bottom.y -= scale.y / 2.0f;
        left -= scale.x / 2.0f;
        right += scale.x / 2.0f;

        // What is the length of the ray from the camera to the object's center?
        GLfloat len = glm::length(pos - display_manager_->getCameraCoords());

        glm::vec3 ray_end_coords = ray_start_coords + (window_manager_->getDisplayManager()->getRayFromCamera(mouse_x, mouse_y) * len);

        if (ray_end_coords.x >= left.x && ray_end_coords.x <= right.x && ray_end_coords.y <= top.y && ray_end_coords.y >= bottom.y)
        {
            return bin;
        }
    }

    return nullptr;
}

