#include "externalaudiosource.h"
using namespace mu;
using namespace mu::audio;

ExternalAudioSource::ExternalAudioSource(const TrackId trackId, const io::Device *playbackData)
    : m_trackId(trackId), m_playbackData(playbackData)
{
    ONLY_AUDIO_WORKER_THREAD;
    ((QIODevice*)playbackData)->open(QFile::ReadOnly);
    b = ((QIODevice*)playbackData)->readAll();

    drwav_init_memory(&m_wav ,b.constData(),b.size(),nullptr);
    m_sampleRate = m_wav.sampleRate;
    m_channels = m_wav.channels;

}
bool ExternalAudioSource::isActive() const
{
    return m_active;
}
void ExternalAudioSource::setIsActive(const bool active)
{
     LOGI() << active;
     m_active = active;
}
void ExternalAudioSource::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;
}
unsigned int ExternalAudioSource::audioChannelsCount() const
{
    ONLY_AUDIO_WORKER_THREAD;


    return m_channels;
}
async::Channel<unsigned int> ExternalAudioSource::audioChannelsCountChanged() const
{

}
samples_t ExternalAudioSource::process(float* buffer, samples_t samplesPerChannel)
{
    ONLY_AUDIO_WORKER_THREAD;



    drwav_read_pcm_frames_f32(&m_wav, samplesPerChannel,buffer);

    return samplesPerChannel;

}

void ExternalAudioSource::seek(const msecs_t newPositionMsecs)  {
    drwav_init_memory(&m_wav, b.constData(),b.size(),NULL);
    drwav_read_pcm_frames_f32(&m_wav, newPositionMsecs/1000 * m_wav.sampleRate, nullptr);




}

void ExternalAudioSource::applyInputParams(const AudioInputParams& requiredParams)
{
        if(m_params == requiredParams) {
            return;
        }
    m_params = requiredParams;
    m_paramsChanges.send(m_params);

}

async::Channel<AudioInputParams> ExternalAudioSource::inputParamsChanged() const
{

return m_paramsChanges;
}
const AudioInputParams& ExternalAudioSource::inputParams() const
{

    return m_params;
}
