#include "LinearTimeSpectrum.h"

#include <iostream>

LinearTimeSpectrum::LinearTimeSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor)
        : SimpleSpectrum(window_manager, sampler, bin_coalesce_factor)
{
    slices_ = 0;
    current_slice_ = 0;
    current_sweep_ = 0;
}

void LinearTimeSpectrum::run()
{
    resetState();

    display_manager_->setPerspective(0.1f, 100.0f, 45.0f);

    FrameQueue* frame_queue = new FrameQueue(display_manager_, true);
    frame_queue->setFrameRate(1);

    frame_ = frame_queue->newFrame();

    current_sweep_ = samples_->getSweepCount();
    addSpectrumRanges(0, 0);

    char msg[128];
    snprintf(msg, sizeof(msg), "Linear Time Sliced Perspective (%.3fMhz - %.3fMhz)", sampler_->getStartFrequency() / 1000000.0f, sampler_->getEndFrequency() / 1000000.0f);
    frame_->addText(msg, 10, 10, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));

    frame_queue->enqueueFrame(frame_);  // @todo we should use a shared pointer so we also retain ownership

    display_manager_->setUpdateSceneCallback(std::bind(&LinearTimeSpectrum::updateSceneCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    frame_queue->setReady();
    frame_queue->setActive();    // transfer ownership to DisplayManager
}

void LinearTimeSpectrum::updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame)
{
    if ( ! set_initial_camera_)
    {
        window_manager_->setCameraCoords(glm::vec3(-50, 10, 35));
        window_manager_->setCameraPointingVector(glm::vec3(1, 0, -1.0));
        set_initial_camera_ = true;
    }

    // If the samplers have completed a new full sweep of the spectrum, move onto the next time slice
    if (samples_->getSweepCount() != current_sweep_)
    {
        current_sweep_ = samples_->getSweepCount();
        current_slice_++;

        addSpectrumRanges(current_slice_, secs_since_framequeue_started);
    }

//    if (samples_->getSweepCount() && current_interest_markers_ < max_interest_markers_)
//    {
//        markLocalMaxima();
//    }

    frame_->updateObjects(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, static_cast<void*>(&current_slice_));
}

void LinearTimeSpectrum::addSpectrumRanges(uint16_t slice_id, GLfloat secs_since_framequeue_started)
{
    uint64_t raw_bin_count = samples_->getBinCount();
    uint64_t coalesced_bin_count = (raw_bin_count + bin_coalesce_factor_ - 1) / bin_coalesce_factor_; // integer ceiling

    std::cout << "Coalescing " << raw_bin_count << " frequency bins into " << coalesced_bin_count << " visual bins" << std::endl;

    uint64_t marker_spacing = coalesced_bin_count / 4;
    glm::vec3 start_coords = glm::vec3(-1.0f * ((coalesced_bin_count * bin_width_) / 2.0f), 0, -1.0 * slice_id);
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

        SimpleSpectrumRange* bin = new SimpleSpectrumRange(display_manager_, Primitive::Type::RECTANGLE, slice_id, bin_id, world_coords, glm::vec3(1, 1, 1), frequency_bins);
        bin->setScale(bin_width_, 1.0, 1.0);

        coalesced_bins_.push_back(bin);
        frame_->addObject(bin);

        if (bin_id % marker_spacing == 0)
        {
            char msg[64];

            if (slice_id == 0)
            {
                snprintf(msg, sizeof(msg), "%.3fMHz", const_cast<sdr::FrequencyBin*>(frequency_bins[0])->getFrequency() / 1000000.0f);
                frame_->addText(msg, world_coords.x, -2.0f, world_coords.z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));
            }

            if (bin_id == 0)
            {
                snprintf(msg, sizeof(msg), "t = %.2f sec", secs_since_framequeue_started);
                frame_->addText(msg, world_coords.x - 5.0f, -2.0f, world_coords.z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));
            }
        }
    }
}

void LinearTimeSpectrum::addInterestMarkerToBin(SimpleSpectrumRange *bin)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "%.3fMHz", bin->getFrequency() / 1000000.0f);
    frame_->addText(msg, bin->getPosition().x, bin->getAmplitude() + 0.2f, bin->getPosition().z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));

    SimpleSpectrum::addInterestMarkerToBin(bin);
}

