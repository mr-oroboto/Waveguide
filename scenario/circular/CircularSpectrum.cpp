#include "CircularSpectrum.h"

#include <iostream>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "scenario/RotatedSpectrumRange.h"
#include "sdr/SpectrumSampler.h"
#include "sdr/SpectrumSamples.h"

CircularSpectrum::CircularSpectrum(DisplayManager *display_manager, sdr::SpectrumSampler *sampler, uint32_t bin_coalesce_factor)
        : Scenario(display_manager), sampler_(sampler), bin_coalesce_factor_(bin_coalesce_factor)
{
    samples_ = sampler_->getSamples();
    radius_ = 8;

    assert(bin_coalesce_factor_ % 2 == 0);
}

CircularSpectrum::~CircularSpectrum()
{
    std::cout << "CircularSpectrum::~CircularSpectrum()" << std::endl;
}

void CircularSpectrum::run()
{
    /**
     * 1. Create a new repeating FrameQueue
     * 2. Create a single Frame that will be reused over and over
     * 3. Place initial SceneObjects in the frame
     * 4. Register a callback to be used to update SceneObjects
     * 5. Run the FrameQueue
     */
    FrameQueue* frame_queue = new FrameQueue(display_manager_, true);
    frame_queue->setFrameRate(1);

    frame_ = frame_queue->newFrame();

    uint64_t raw_bin_count = samples_->getBinCount();
    uint64_t coalesced_bin_count = (raw_bin_count + bin_coalesce_factor_ - 1) / bin_coalesce_factor_; // integer ceiling

    std::cout << "Coalescing " << raw_bin_count << " frequency bins into " << coalesced_bin_count << " visual bins" << std::endl;

    double rad_per_bin = (2*M_PI) / coalesced_bin_count;            // each full spectrum band wraps once around the sphere

    glm::vec3 start_coords = glm::vec3(0, 0, 0);                    // initial co-ordinates of the sphere's center

    for (uint64_t bin_id = 0; bin_id < coalesced_bin_count; bin_id++)
    {
        // Coalesce the frequency bins into a spectrum range
        std::vector<sdr::FrequencyBin const*> frequency_bins;
        uint64_t start_frequency_bin = bin_id * bin_coalesce_factor_;
        for (uint64_t j = start_frequency_bin; j < (start_frequency_bin + bin_coalesce_factor_) && j < raw_bin_count; j++)
        {
            frequency_bins.push_back(samples_->getFrequencyBin(j));
        }

        double theta = (rad_per_bin * bin_id);

        glm::vec3 world_coords = start_coords;
        world_coords.x += radius_ * cos(theta);
        world_coords.y += radius_ * sin(theta);

        RotatedSpectrumRange* bin = new RotatedSpectrumRange(display_manager_, Primitive::Type::LINE, 0, bin_id, world_coords, theta, 0, radius_, glm::vec3(1, 1, 1), frequency_bins);

        frame_->addObject(bin);
    }

    frame_->addText("Circular Frequency Analysis", 10, 10, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));

    frame_queue->enqueueFrame(frame_);  // @todo we should use a shared pointer so we also retain ownership

    display_manager_->setUpdateSceneCallback(std::bind(&CircularSpectrum::updateSceneCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    frame_queue->setReady();
    frame_queue->setActive();    // transfer ownership to DisplayManager
}

void CircularSpectrum::updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame)
{
    uint16_t current_ring = 0;

    frame_->updateObjects(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, reinterpret_cast<void*>(&current_ring));
}