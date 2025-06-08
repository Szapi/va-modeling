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

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace TRM {
//------------------------------------------------------------------------
static const Steinberg::FUID kTS808ClipperProcessorUID (0x7DB2003F, 0xED345C06, 0xABA65E16, 0x088B3127);
static const Steinberg::FUID kTS808ClipperControllerUID (0x551BA7FD, 0xDBFB5EC6, 0x80A384AB, 0x30B0F6E7);

#define TS808ClipperVST3Category "Fx"

//------------------------------------------------------------------------
} // namespace TRM
