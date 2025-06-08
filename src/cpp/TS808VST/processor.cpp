//------------------------------------------------------------------------
// Copyright (C) 2025 Ték Róbert Máté <eppenpontaz@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <fstream>

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace TRM {
//------------------------------------------------------------------------
// TS808ClipperProcessor
//------------------------------------------------------------------------
TS808ClipperProcessor::TS808ClipperProcessor ()
{
    setControllerClass (kTS808ClipperControllerUID);
}

//------------------------------------------------------------------------
TS808ClipperProcessor::~TS808ClipperProcessor ()
{
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperProcessor::initialize (FUnknown* context)
{
    tresult result = AudioEffect::initialize (context);
    if (result != kResultOk)
    {
        return result;
    }

    addAudioInput (STR16 ("Stereo In"), SpeakerArr::kStereo);
    addAudioOutput (STR16 ("Stereo Out"), SpeakerArr::kStereo);

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperProcessor::terminate ()
{
    stateTransfer.clear_ui ();
    return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperProcessor::setActive (TBool state)
{
    return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperProcessor::process (Vst::ProcessData& data)
{
    stateTransfer.accessTransferObject_rt ([this] (const auto& stateModel)
    {
        gainParameter.setValue (stateModel.gain);
        toneParameter.setValue(stateModel.tone);
        levelParameter.setValue(stateModel.level);
    });

    handleParameterChanges (data.inputParameterChanges);

    if (data.numSamples == 0)
    {
        gainParameter.endChanges ();
        toneParameter.endChanges ();
        levelParameter.endChanges ();
        return kResultOk;
    }

    if (processSetup.symbolicSampleSize == SymbolicSampleSizes::kSample32)
        process<SymbolicSampleSizes::kSample32> (data);
    else
        process<SymbolicSampleSizes::kSample64> (data);

    gainParameter.endChanges ();
    toneParameter.endChanges ();
    levelParameter.endChanges ();
    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
    return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
    return (symbolicSampleSize == SymbolicSampleSizes::kSample32 ||
            symbolicSampleSize == SymbolicSampleSizes::kSample64) ?
               kResultTrue :
               kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperProcessor::setState (IBStream* state)
{
    IBStreamer streamer (state, kLittleEndian);

    uint32 numParams;
    if (streamer.readInt32u (numParams) == false)
        return kResultFalse;

    auto model = std::make_unique<StateModel> ();

    {
        ParamValue gain;
        if (!streamer.readDouble (gain))
            return kResultFalse;
        model->gain = gain;
    }
    {
        ParamValue tone;
        if (!streamer.readDouble (tone))
            return kResultFalse;
        model->tone = tone;
    }
    {
        ParamValue level;
        if (!streamer.readDouble (level))
            return kResultFalse;
        model->level = level;
    }

    stateTransfer.transferObject_ui (std::move (model));
    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperProcessor::getState (IBStream* state)
{
    IBStreamer streamer (state, kLittleEndian);
    streamer.writeDouble (gainParameter.getValue ());
    streamer.writeDouble (toneParameter.getValue ());
    streamer.writeDouble (levelParameter.getValue ());
    return kResultOk;
}

//------------------------------------------------------------------------
void TS808ClipperProcessor::handleParameterChanges (IParameterChanges* changes)
{
    if (!changes)
        return;

    int32 changeCount = changes->getParameterCount ();
    for (auto i = 0; i < changeCount; ++i)
    {
        if (auto queue = changes->getParameterData (i))
        {
            auto paramID = queue->getParameterId ();
            if (paramID == ParameterID::Gain)
            {
                gainParameter.beginChanges (queue);
            } else
            if (paramID == ParameterID::Tone)
            {
                toneParameter.beginChanges (queue);
            } else
            if (paramID == ParameterID::Level)
            {
                levelParameter.beginChanges (queue);
            }
        }
    }
}

//------------------------------------------------------------------------
} // namespace TRM
