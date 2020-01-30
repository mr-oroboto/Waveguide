#include "SimpleSpectrumRange.h"

SimpleSpectrumRange::SimpleSpectrumRange(DisplayManager* display_manager, Primitive::Type type, uint16_t slice_id, uint64_t bin_id,
                                         const glm::vec3& world_coords, const glm::vec3& colour,
                                         const std::vector<sdr::FrequencyBin const*>& frequency_bins) :
        SceneObject(display_manager, type, world_coords, colour), slice_id_(slice_id), bin_id_(bin_id), frequency_bins_(frequency_bins)
{
    amplitude_ = 0.0f;
}

SimpleSpectrumRange::~SimpleSpectrumRange()
{
}

float SimpleSpectrumRange::getAmplitude(bool refresh)
{
    if ( ! refresh)
    {
        return amplitude_;
    }

    float average_amplitude = 0.0f;
    for (sdr::FrequencyBin const* bin : frequency_bins_)
    {
        average_amplitude += const_cast<sdr::FrequencyBin*>(bin)->getLatestAmplitude(true);
    }

    average_amplitude /= frequency_bins_.size();    // in dB
    amplitude_ = average_amplitude + 100;           // offset so -100dB == 0 (ie. 30)
    amplitude_ /= 2.0;                              // todo: remove me

    return amplitude_;
}

uint64_t SimpleSpectrumRange::getFrequency()
{
    return const_cast<sdr::FrequencyBin*>(frequency_bins_[0])->getFrequency();
}

uint64_t SimpleSpectrumRange::getBinId()
{
    return bin_id_;
}

void SimpleSpectrumRange::draw(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, bool use_colour)
{
    if ( ! const_cast<sdr::FrequencyBin*>(frequency_bins_[0])->getHasBeenSet(2))
    {
        return;
    }

    SceneObject::draw(secs_since_rendering_started, secs_since_framequeue_started, secs_since_last_renderloop, secs_since_last_frame, use_colour);
}

void SimpleSpectrumRange::update(GLfloat secs_since_rendering_started, GLfloat secs_since_framequeue_started, GLfloat secs_since_last_renderloop, GLfloat secs_since_last_frame, void* context)
{
    uint16_t current_slice_id = *(static_cast<uint16_t*>(context));

    // If this bin doesn't belong to the ring currently being rendered, exit early.
    if (current_slice_id != slice_id_)
    {
        return;
    }

    float amplitude = getAmplitude(true);

//  float amplitude_shade = (1.0f / 50.0f) * amplitude;    // todo: restore me once amp / 2 is done
    float amplitude_shade = (1.0f / 25.0f) * amplitude;    // todo: remove me once amp / 2 is done
    float r = amplitude_shade, g = amplitude_shade, b = 1;

    this->setScale(this->scale_x_, amplitude, this->scale_z_);
    this->setColour(glm::vec3(r, g, b));
}