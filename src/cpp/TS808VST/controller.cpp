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

#include "controller.h"
#include "cids.h"
#include "pids.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "base/source/fstreamer.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace TRM {

//------------------------------------------------------------------------
// TS808ClipperController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperController::initialize (FUnknown* context)
{
tresult result = EditControllerEx1::initialize (context);
if (result != kResultOk)
{
return result;
}

parameters.addParameter (STR ("Gain"), STR ("%"), 0, 0., ParameterInfo::kCanAutomate, ParameterID::Gain);
parameters.addParameter (STR ("Tone"), STR ("%"), 0, 0., ParameterInfo::kCanAutomate, ParameterID::Tone);
parameters.addParameter (STR ("Level"), STR ("%"), 0, 0., ParameterInfo::kCanAutomate, ParameterID::Level);

return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperController::terminate ()
{
return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperController::setComponentState (IBStream* state)
{
if (!state)
return kResultFalse;

IBStreamer streamer (state, kLittleEndian);
{
ParamValue gain;
if (!streamer.readDouble (gain))
return kResultFalse;

if (auto param = parameters.getParameter (ParameterID::Gain))
param->setNormalized (gain);
}
{
ParamValue tone;
if (!streamer.readDouble (tone))
return kResultFalse;

if (auto param = parameters.getParameter (ParameterID::Tone))
param->setNormalized (tone);
}
{
ParamValue level;
if (!streamer.readDouble (level))
return kResultFalse;

if (auto param = parameters.getParameter (ParameterID::Level))
param->setNormalized (level);
}
return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperController::setState (IBStream* state)
{
if (!state)
return kResultFalse;

IBStreamer streamer (state, kLittleEndian);
{
ParamValue gain;
if (!streamer.readDouble (gain))
return kResultFalse;

if (auto param = parameters.getParameter (ParameterID::Gain))
param->setNormalized (gain);
}
{
ParamValue tone;
if (!streamer.readDouble (tone))
return kResultFalse;

if (auto param = parameters.getParameter (ParameterID::Tone))
param->setNormalized (tone);
}
{
ParamValue level;
if (!streamer.readDouble (level))
return kResultFalse;

if (auto param = parameters.getParameter (ParameterID::Level))
param->setNormalized (level);
}

return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API TS808ClipperController::getState (IBStream* state)
{
IBStreamer streamer (state, kLittleEndian);

if (auto param = parameters.getParameter (ParameterID::Gain))
streamer.writeDouble (param->getNormalized ());

if (auto param = parameters.getParameter (ParameterID::Tone))
streamer.writeDouble (param->getNormalized ());

if (auto param = parameters.getParameter (ParameterID::Level))
streamer.writeDouble (param->getNormalized ());

return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API TS808ClipperController::createView (FIDString name)
{
if (FIDStringsEqual (name, Vst::ViewType::kEditor))
{
auto* view = new VSTGUI::VST3Editor (this, "view", "editor.uidesc");
return view;
}
return nullptr;
}

//------------------------------------------------------------------------
} // namespace TRM
