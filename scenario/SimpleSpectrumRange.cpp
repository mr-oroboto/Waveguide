#include "SimpleSpectrumRange.h"

SimpleSpectrumRange::SimpleSpectrumRange(DisplayManager* display_manager, Primitive::Type type, const glm::vec3& world_coords,
                           const glm::vec3& colour, const std::vector<sdr::FrequencyBin const*>& frequency_bins) :
        SceneObject(display_manager, type, world_coords, colour), frequency_bins_(frequency_bins)
{
}

SimpleSpectrumRange::~SimpleSpectrumRange()
{
}

void SimpleSpectrumRange::draw(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, bool use_colour)
{
    if ( ! const_cast<sdr::FrequencyBin*>(frequency_bins_[0])->getHasBeenSet())
    {
        return;
    }

    SceneObject::draw(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, use_colour);
}

void SimpleSpectrumRange::update(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, void* context)
{
    float average_amplitude = 0.0f;

    for (sdr::FrequencyBin const* bin : frequency_bins_)
    {
        average_amplitude += const_cast<sdr::FrequencyBin*>(bin)->getLatestAmplitude(true);
    }

    average_amplitude /= frequency_bins_.size();        // in dB
    float adjusted_amplitude = average_amplitude + 100; // offset so -100dB == 0 (ie. 30)
    adjusted_amplitude /= 2.0;                              // todo: remove me

//  float amplitude_shade = (1.0f / 50.0f) * adjusted_amplitude;    // todo: restore me once amp / 2 is done
    float amplitude_shade = (1.0f / 25.0f) * adjusted_amplitude;    // todo: restore me once amp / 2 is done
    float r = amplitude_shade, g = amplitude_shade, b = 1;

    this->setScale(this->scale_x_, adjusted_amplitude, this->scale_z_);
    this->setColour(glm::vec3(r, g, b));
}