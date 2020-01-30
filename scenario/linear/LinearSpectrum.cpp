#include "LinearSpectrum.h"

LinearSpectrum::LinearSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor)
        : SimpleSpectrum(window_manager, sampler, bin_coalesce_factor)
{
}

LinearSpectrum::~LinearSpectrum()
{
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

    display_manager_->setPerspective(0.1f, 100.0f, 45.0f);

    FrameQueue* frame_queue = new FrameQueue(display_manager_, true);
    frame_queue->setFrameRate(1);

    frame_ = frame_queue->newFrame();

    uint64_t raw_bin_count = samples_->getBinCount();
    uint64_t coalesced_bin_count = (raw_bin_count + bin_coalesce_factor_ - 1) / bin_coalesce_factor_; // integer ceiling

    std::cout << "Coalescing " << raw_bin_count << " frequency bins into " << coalesced_bin_count << " visual bins" << std::endl;

    uint64_t marker_spacing = coalesced_bin_count / 4;
    glm::vec3 start_coords = glm::vec3(-1.0f * ((coalesced_bin_count * bin_width_) / 2.0f), 0, 0);

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
            sprintf(msg, "%.3fMHz", const_cast<sdr::FrequencyBin*>(frequency_bins[0])->getFrequency() / 1000000.0f);
            frame_->addText(msg, world_coords.x, -2.0f, world_coords.z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));
        }
    }

    frame_->addText("Linear Perspective", 10, 10, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));

    frame_queue->enqueueFrame(frame_);  // @todo we should use a shared pointer so we also retain ownership

    display_manager_->setUpdateSceneCallback(std::bind(&LinearSpectrum::updateSceneCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    frame_queue->setReady();
    frame_queue->setActive();    // transfer ownership to DisplayManager
}

void LinearSpectrum::updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame)
{
    uint16_t current_slice = 0;

    if (samples_->getSweepCount() && current_markers_ < max_markers_)
    {
        markLocalMaxima();
    }

    frame_->updateObjects(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, static_cast<void*>(&current_slice));
}

void LinearSpectrum::markBin(SimpleSpectrumRange* bin)
{
    char msg[64];
    sprintf(msg, "%.3fMHz", bin->getFrequency() / 1000000.0f);
    frame_->addText(msg, bin->getPosition().x, bin->getAmplitude() + 0.2f, bin->getPosition().z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));

    SimpleSpectrum::markBin(bin);
}

