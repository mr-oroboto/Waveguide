#include "CircularSpectrum.h"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "scenario/RotatedSpectrumRange.h"

CircularSpectrum::CircularSpectrum(WindowManager* window_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor)
        : SimpleSpectrum(window_manager, sampler, bin_coalesce_factor)
{
    radius_ = 8;
    max_markers_ = 12;
    max_freq_markers_ = 4;
}

CircularSpectrum::~CircularSpectrum()
{
}

void CircularSpectrum::run()
{
    resetState();

    display_manager_->setPerspective(0.1, 130.0, 45);

    FrameQueue* frame_queue = new FrameQueue(display_manager_, true);
    frame_queue->setFrameRate(1);

    frame_ = frame_queue->newFrame();

    uint64_t raw_bin_count = samples_->getBinCount();
    uint64_t coalesced_bin_count = (raw_bin_count + bin_coalesce_factor_ - 1) / bin_coalesce_factor_; // integer ceiling

    std::cout << "Coalescing " << raw_bin_count << " frequency bins into " << coalesced_bin_count << " visual bins" << std::endl;

    double rad_per_bin = (2*M_PI) / coalesced_bin_count;            // each full spectrum band wraps once around the sphere

    uint64_t marker_spacing = coalesced_bin_count / max_freq_markers_;
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

        coalesced_bins_.push_back(bin);
        frame_->addObject(bin);

        if ((bin_id % marker_spacing) == 0 && bin_id < coalesced_bin_count - 2)
        {
            char msg[64];
            sprintf(msg, "%.3fMHz", const_cast<sdr::FrequencyBin*>(frequency_bins[0])->getFrequency() / 1000000.0f);
            float text_y = world_coords.y > 0 ? world_coords.y - 2.0f : world_coords.y + 2.0f;
            frame_->addText(msg, world_coords.x > 0 ? world_coords.x - 2.0f : world_coords.x + 2.0f, world_coords.y == 0 ? world_coords.y : text_y, world_coords.z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));
        }
    }

    frame_->addText("Circular Perspective", 10, 10, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));

    frame_queue->enqueueFrame(frame_);  // @todo we should use a shared pointer so we also retain ownership

    display_manager_->setUpdateSceneCallback(std::bind(&CircularSpectrum::updateSceneCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    frame_queue->setReady();
    frame_queue->setActive();    // transfer ownership to DisplayManager
}

void CircularSpectrum::updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame)
{
    uint16_t current_ring = 0;

    if (samples_->getSweepCount() && current_markers_ < max_markers_)
    {
        markLocalMaxima();
    }

    frame_->updateObjects(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, static_cast<void*>(&current_ring));
}

void CircularSpectrum::markBin(SimpleSpectrumRange* bin)
{
    char msg[64];
    sprintf(msg, "%.3fMHz", bin->getFrequency() / 1000000.0f);

    float x = (radius_ + bin->getAmplitude() + 1.0) * cos(dynamic_cast<RotatedSpectrumRange*>(bin)->getThetaOffset());
    float y = (radius_ + bin->getAmplitude() + 1.0) * sin(dynamic_cast<RotatedSpectrumRange*>(bin)->getThetaOffset());

    frame_->addText(msg, x, y, bin->getPosition().z, false, 0.02, glm::vec3(1.0, 1.0, 1.0));

    SimpleSpectrum::markBin(bin);
}
