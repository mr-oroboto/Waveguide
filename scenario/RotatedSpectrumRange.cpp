#include "RotatedSpectrumRange.h"

RotatedSpectrumRange::RotatedSpectrumRange(DisplayManager* display_manager, Primitive::Type type, uint16_t ring_id, uint64_t bin_id,
                                       const glm::vec3& world_coords, double theta_offset, double rad_per_ring, double radius,
                                       const glm::vec3& colour,
                                       const std::vector<sdr::FrequencyBin const*>& frequency_bins) :
        SimpleSpectrumRange(display_manager, type, world_coords, colour, frequency_bins),
        ring_id_(ring_id), bin_id_(bin_id),
        theta_offset_(theta_offset), rad_per_ring_(rad_per_ring), radius_(radius)
{
}

RotatedSpectrumRange::~RotatedSpectrumRange()
{
}

void RotatedSpectrumRange::draw(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, bool use_colour)
{
    if ( ! const_cast<sdr::FrequencyBin*>(frequency_bins_[0])->getHasBeenSet())
    {
        return;
    }

    SceneObject::draw(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, use_colour);
}

void RotatedSpectrumRange::update(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, void* context)
{
    uint16_t current_ring_id = *(reinterpret_cast<uint16_t*>(context));

    // If this bin doesn't belong to the ring currently being rendered, exit early.
    if (current_ring_id != ring_id_)
    {
        return;
    }

    float average_amplitude = 0.0f;
    for (sdr::FrequencyBin const* bin : frequency_bins_)
    {
        average_amplitude += const_cast<sdr::FrequencyBin*>(bin)->getLatestAmplitude(true);
    }

    average_amplitude /= frequency_bins_.size();            // in dB (ie. -70)
    float adjusted_amplitude = average_amplitude + 100;     // offset so -100dB == 0 (ie. 30)
    adjusted_amplitude /= 2.0;                              // todo: remove me

//  std::cout << const_cast<sdr::FrequencyBin*>(frequency_bins_[0])->getFrequency() << "Hz: " << average_amplitude << "dB (" << adjusted_amplitude << " adjusted dB)" << std::endl;

    float x_to = (radius_ + adjusted_amplitude) * cos(theta_offset_);
    float y_to = (radius_ + adjusted_amplitude) * sin(theta_offset_);
    float z_to = 0;

    if (ring_id_ != 0)
    {
        // Rotate subsequent rings around the y-axis
        z_to = x_to * sin(rad_per_ring_ * ring_id_);
        x_to = x_to * cos(rad_per_ring_ * ring_id_);    // note order (second) so we don't overwrite x used in z
    }

    this->setAdditionalCoords(glm::vec3(x_to, y_to, z_to));

    // Anything over 50dB adjusted (-50dB upward) is full intensity
//    float amplitude_shade = (1.0f / 50.0f) * adjusted_amplitude;    // todo: restore me once amp / 2 is done
    float amplitude_shade = (1.0f / 25.0f) * adjusted_amplitude;    // todo: restore me once amp / 2 is done
    float r = amplitude_shade, g = amplitude_shade, b = 1;

    if (false /* debug ring positions */)
    {
        switch (ring_id_)
        {
            case 0:
                r = 1; g = b = 0;
                break;
            case 1:
                r = g = 0; b = 1;
                break;
            default:
                r = g = b = 1.0;
                break;
        }
    }
    else if (false /* debug bin positions */)
    {
        r = g = (1.0f / 28) * bin_id_;  // DRAGON: 28 assumes 28 total coalesced bins during debugging
    }

    this->setColour(glm::vec3(r, g, b));
}