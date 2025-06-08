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

#include "1N4148_IVTable.hpp"

namespace TRM
{

    inline constexpr double Rf = 51'000.;  // Feedback resistor   (51k)
    inline constexpr double Cf = 51.e-12;  // Feedback capacitor  (51pF)
    inline constexpr double Rg = 4700.;    // Ground resistor     (4k7)
    inline constexpr double Rd = 500'000.; // Drive potentiometer (500k)

    inline constexpr double OpAmpBias = 4.5; // Volts
    
} // namespace TRM
