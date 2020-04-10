#include "GridSpectrum.h"

#include <iostream>

GridSpectrum::GridSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor)
        : SimpleSpectrum(window_manager, sampler, bin_coalesce_factor)
{
}

void GridSpectrum::run()
{
    resetState();

    display_manager_->setPerspective(0.1, 100.0, 90);

    FrameQueue* frame_queue = new FrameQueue(display_manager_, true);
    frame_queue->setFrameRate(1);

    frame_ = frame_queue->newFrame();

    uint64_t raw_bin_count = samples_->getBinCount();
    uint64_t coalesced_bin_count = (raw_bin_count + bin_coalesce_factor_ - 1) / bin_coalesce_factor_; // integer ceiling

    std::cout << "Coalescing " << raw_bin_count << " frequency bins into " << coalesced_bin_count << " visual bins" << std::endl;

    uint32_t grid_width = 80;
    glm::vec3 start_coords = glm::vec3(-1.0f * (grid_width / 2.0f), 0, 0);

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
        world_coords.x += (bin_id % grid_width);

        SimpleSpectrumRange* bin = new SimpleSpectrumRange(display_manager_, Primitive::Type::RECTANGLE, 0, bin_id, world_coords, glm::vec3(1, 1, 1), frequency_bins);

        coalesced_bins_.push_back(bin);
        frame_->addObject(bin);

        if (bin_id != 0 && (bin_id % grid_width) == 0)
        {
            char msg[64];
            snprintf(msg, sizeof(msg), "%.2fMHz", const_cast<sdr::FrequencyBin*>(frequency_bins[0])->getFrequency() / 1000000.0f);
            frame_->addText(msg, world_coords.x - 5.0f, 0.0f, world_coords.z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));

            start_coords.z -= 1.0f;
        }
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "Grid Perspective (%.3fMhz - %.3fMhz)", sampler_->getStartFrequency() / 1000000.0f, sampler_->getEndFrequency() / 1000000.0f);
    frame_->addText(msg, 10, 10, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));

    frame_queue->enqueueFrame(frame_);  // @todo we should use a shared pointer so we also retain ownership

    display_manager_->setUpdateSceneCallback(std::bind(&GridSpectrum::updateSceneCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    frame_queue->setReady();
    frame_queue->setActive();    // transfer ownership to DisplayManager
}

void GridSpectrum::updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame)
{
    uint16_t current_slice = 0;

    if ( ! set_initial_camera_)
    {
        window_manager_->setCameraCoords(glm::vec3(-55, 20, 25));
        window_manager_->setCameraPointingVector(glm::vec3(1, 0, -1.0));
        set_initial_camera_ = true;
    }

    if (samples_->getSweepCount() && current_interest_markers_ < max_interest_markers_)
    {
        markLocalMaxima();
    }

    frame_->updateObjects(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, static_cast<void*>(&current_slice));
}

void GridSpectrum::addInterestMarkerToBin(SimpleSpectrumRange *bin)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "%.3fMHz", bin->getFrequency() / 1000000.0f);
    unsigned long text_id = frame_->addText(msg, bin->getPosition().x, bin->getAmplitude() + 0.2f, bin->getPosition().z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));

    marked_bin_text_ids_.push_back(text_id);

    SimpleSpectrum::addInterestMarkerToBin(bin);
}

void GridSpectrum::clearInterestMarkers()
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