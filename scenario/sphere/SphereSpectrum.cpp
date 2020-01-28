#include "SphereSpectrum.h"

#include <cmath>

#include "scenario/RotatedSpectrumRange.h"

SphereSpectrum::SphereSpectrum(DisplayManager* display_manager, sdr::SpectrumSampler* sampler, uint32_t bin_coalesce_factor)
        : SimpleSpectrum(display_manager, sampler, bin_coalesce_factor)
{
    radius_ = 8;
    rings_ = 10;
    current_ring_ = 0;
}

SphereSpectrum::~SphereSpectrum()
{
    std::cout << "SphereSpectrum::~SphereSpectrum()" << std::endl;
}

void SphereSpectrum::run()
{
    resetState();

    FrameQueue* frame_queue = new FrameQueue(display_manager_, true);
    frame_queue->setFrameRate(1);

    frame_ = frame_queue->newFrame();

    current_sweep_ = samples_->getSweepCount();
    uint64_t raw_bin_count = samples_->getBinCount();
    uint64_t coalesced_bin_count = (raw_bin_count + bin_coalesce_factor_ - 1) / bin_coalesce_factor_; // integer ceiling

    std::cout << "Coalescing " << raw_bin_count << " frequency bins into " << coalesced_bin_count << " visual bins" << std::endl;

    double rad_per_bin = (2*M_PI) / (coalesced_bin_count * bin_width_);     // each full spectrum band wraps once around the sphere
    double rad_per_ring = ((170.0 / 180.0) * M_PI) / rings_;                // half of the sphere (a bit less than M_PI rad) can be used for history, otherwise bw/4 and 3bw/4 wrap into each other

    glm::vec3 start_coords = glm::vec3(0, 0, 0);                    // initial co-ordinates of the sphere's center

    for (uint16_t ring_id = 0; ring_id < rings_; ring_id++)
    {
        for (uint64_t bin_id = 0; bin_id < coalesced_bin_count; bin_id++)
        {
            // Coalesce the frequency bins
            std::vector<sdr::FrequencyBin const*> frequency_bins;
            uint64_t start_frequency_bin = bin_id * bin_coalesce_factor_;
            for (uint64_t j = start_frequency_bin; j < (start_frequency_bin + bin_coalesce_factor_) && j < raw_bin_count; j++)
            {
                frequency_bins.push_back(samples_->getFrequencyBin(j));
            }

            double theta = (rad_per_bin * bin_id * bin_width_);
            glm::vec3 world_coords = start_coords;

            float x = radius_ * cos(theta);
            float y = radius_ * sin(theta);
            float z = 0;

            if (ring_id != 0)
            {
                // Rotate subsequent rings around the y-axis
                z = x * sin(rad_per_ring * ring_id);
                x = x * cos(rad_per_ring * ring_id);    // note order (second) so we don't overwrite x used in z
            }

            world_coords.x += x;
            world_coords.y += y;
            world_coords.z += z;

//          bin = new RotatedSpectrumRange(display_manager_, Primitive::Type::CUBE, ring_id, bin_id, world_coords, theta, rad_per_ring, radius_, glm::vec3(1, 1, 1), frequency_bins);
            RotatedSpectrumRange* bin = new RotatedSpectrumRange(display_manager_, Primitive::Type::TRANSFORMING_RECTANGLE, ring_id, bin_id, world_coords, theta, rad_per_ring, radius_, glm::vec3(1, 1, 1), frequency_bins);
            bin->setScale(bin_width_, 1, 1);

            coalesced_bins_.push_back(bin);
            frame_->addObject(bin);
        }
    }

    frame_->addText("Spherical Perspective", 10, 10, 0, true, 1.0, glm::vec3(1.0, 1.0, 1.0));

    frame_queue->enqueueFrame(frame_);  // @todo we should use a shared pointer so we also retain ownership

    display_manager_->setUpdateSceneCallback(std::bind(&SphereSpectrum::updateSceneCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    frame_queue->setReady();
    frame_queue->setActive();    // transfer ownership to DisplayManager
}

void SphereSpectrum::updateSceneCallback(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame)
{
    frame_->updateObjects(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, reinterpret_cast<void*>(&current_ring_));

    // If the samplers have completed a new full sweep of the spectrum, move onto the next ring
    if (samples_->getSweepCount() != current_sweep_)
    {
        current_sweep_ = samples_->getSweepCount();

        if (++current_ring_ >= rings_)
        {
            current_ring_ = 0;
        }
    }

    float camera_z = 80.0 * cos(secs_since_rendering_started * 0.1 * M_PI);
    float camera_x = 80.0 * sin(secs_since_rendering_started * 0.1 * M_PI);

    display_manager_->setCameraCoords(glm::vec3(camera_x, 0, camera_z));
    display_manager_->setCameraPointingVector(glm::vec3(-camera_x, 0, -camera_z));
}