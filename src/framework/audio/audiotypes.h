#ifndef MU_AUDIO_AUDIOTYPES_H
#define MU_AUDIO_AUDIOTYPES_H

#include <variant>
#include <memory>
#include <string>

#include "midi/miditypes.h"
#include "io/path.h"

namespace mu::audio {
using msecs_t = uint64_t;
using samples_t = uint64_t;
using audioch_t = uint8_t;
using volume_db_t = float;
using volume_dbfs_t = float;
using gain_t = float;
using balance_t = float;

using TrackSequenceId = int32_t;
using TrackSequenceIdList = std::vector<TrackSequenceId>;

using TrackId = int32_t;
using TrackIdList = std::vector<TrackId>;
using TrackName = std::string;

using MixerChannelId = int32_t;

using AudioSourceName = std::string;

using FxProcessorId = std::string;
using FxProcessorIdList =  std::vector<std::string>;

struct AudioOutputParams {
    FxProcessorIdList fxProcessors;
    volume_db_t volume = 1.f;
    balance_t balance = 0.f;
    bool isMuted = false;

    bool operator ==(const AudioOutputParams& other)
    {
        return fxProcessors == other.fxProcessors
               && volume == other.volume
               && balance == other.balance
               && isMuted == other.isMuted;
    }
};

using AudioInputParams = std::variant<midi::MidiData, io::path>;

struct AudioParams {
    AudioInputParams in;
    AudioOutputParams out;
};

struct VolumePressureDbfsBoundaries {
    volume_dbfs_t max = 0;
    volume_dbfs_t min = -60;
};
}

#endif // MU_AUDIO_AUDIOTYPES_H
