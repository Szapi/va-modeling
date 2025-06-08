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
#include "controller.h"
#include "cids.h"
#include "version.h"

#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "TS808Clipper"

using namespace Steinberg::Vst;
using namespace TRM;

//------------------------------------------------------------------------
//  VST Plug-in Entry
//------------------------------------------------------------------------

BEGIN_FACTORY_DEF ("Ték Róbert Máté", 
			       "https://github.com/Szapi/va-modeling", 
			       "mailto:eppenpontaz@gmail.com")

	DEF_CLASS2 (INLINE_UID_FROM_FUID(kTS808ClipperProcessorUID),
				PClassInfo::kManyInstances,
				kVstAudioEffectClass,
				stringPluginName,
				Vst::kDistributable,
				TS808ClipperVST3Category,
				FULL_VERSION_STR,
				kVstVersionString,
				TS808ClipperProcessor::createInstance)

	DEF_CLASS2 (INLINE_UID_FROM_FUID (kTS808ClipperControllerUID),
				PClassInfo::kManyInstances,
				kVstComponentControllerClass,
				stringPluginName "Controller",
				0,
				"",
				FULL_VERSION_STR,
				kVstVersionString,
				TS808ClipperController::createInstance)

END_FACTORY
