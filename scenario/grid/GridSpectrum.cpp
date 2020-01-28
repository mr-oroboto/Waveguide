#include "GridSpectrum.h"

#include <iostream>

#include "scenario/SimpleSpectrumRange.h"
#include "sdr/SpectrumSampler.h"
#include "sdr/SpectrumSamples.h"

GridSpectrum::GridSpectrum(DisplayManager *display_manager, sdr::SpectrumSampler *sampler, uint32_t bin_coalesce_factor)
        : Scenario(display_manager), sampler_(sampler), bin_coalesce_factor_(bin_coalesce_factor)
{
    samples_ = sampler_->getSamples();
}

GridSpectrum::~GridSpectrum()
{
    std::cout << "GridSpectrum::~GridSpectrum()" << std::endl;
}

void GridSpectrum::run()
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

    display_manager_->setCameraCoords(glm::vec3(0, 10, 100));

    uint64_t raw_bin_count = samples_->getBinCount();
    uint64_t coalesced_bin_count = (raw_bin_count + bin_coalesce_factor_ - 1) / bin_coalesce_factor_; // integer ceiling

    std::cout << "Coalescing " << raw_bin_count << " frequency bins into " << coalesced_bin_count << " visual bins" << std::endl;

    uint32_t grid_width = 200;
    uint32_t marker_spacing = 50;
    glm::vec3 start_coords = glm::vec3(-1.0f * (grid_width / 2.0f), 0, 0);

    for (uint64_t i = 0; i < coalesced_bin_count; i++)
    {
        // Coalesce the frequency bins
        std::vector<sdr::FrequencyBin const*> frequency_bins;

        uint64_t start_frequency_bin = i * bin_coalesce_factor_;
        for (uint64_t j = start_frequency_bin; j < (start_frequency_bin + bin_coalesce_factor_) && j < raw_bin_count; j++)
        {
            frequency_bins.push_back(samples_->getFrequencyBin(j));
        }

        glm::vec3 world_coords = start_coords;
        world_coords.x += (i % grid_width);

        SimpleSpectrumRange* bin = new SimpleSpectrumRange(display_manager_, Primitive::Type::RECTANGLE, world_coords, glm::vec3(1, 1, 1), frequency_bins);
        frame_->addObject(bin);

        if ((i % marker_spacing) == 0)
        {
            char msg[64];
            sprintf(msg, "%.2fMHz", const_cast<sdr::FrequencyBin*>(frequency_bins[0])->getFrequency() / 1000000.0f);
            frame_->addText(msg, world_coords.x, 5.0f, world_coords.z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));
        }

        if (i != 0 && (i % grid_width) == 0)
        {
            start_coords.z -= 1.0f;
        }
    }

    frame_->addText("Grid Frequency Analysis", 10, 10, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));

    frame_queue->enqueueFrame(frame_);  // @todo we should use a shared pointer so we also retain ownership

    display_manager_->setUpdateSceneCallback(std::bind(&GridSpectrum::updateSceneCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    frame_queue->setReady();
    frame_queue->setActive();    // transfer ownership to DisplayManager
}

void GridSpectrum::updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame)
{
    frame_->updateObjects(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, nullptr);
}