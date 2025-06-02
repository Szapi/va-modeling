/*
 * Copyright (C) 2025 Ték Róbert Máté <eppenpontaz@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Utility.hpp"

#include <array>

namespace TRM
{

    inline constexpr std::array<Measurement, 10'001> Diode_1N4148_IVTable
    {
        #include "1N4148_IVTableData.inl"
    };

    inline constexpr std::array<Measurement, 96> Diode_1N4148_AntiPar_IVTable_SparsePoint5
    {
        #include "1N4148_AntiPar_IVTable_SparsePoint5.inl"
    };

    inline double Diode_1N4148_Current(const double v)
    {
        if (v < (Diode_1N4148_IVTable.front().x + Eps12)) [[unlikely]]
            return Diode_1N4148_IVTable.front().y; // Reverse saturation

        if ((Diode_1N4148_IVTable.back().x - Eps12) < v)  [[unlikely]]
            return Diode_1N4148_IVTable.back().y + 1.7408961998 * (v - Diode_1N4148_IVTable.back().x); // Extrapolate

        const auto upper = std::upper_bound(begin(Diode_1N4148_IVTable),
                                            end(Diode_1N4148_IVTable),
                                            v,
                                            [](const double v, const Measurement& m){ return v < m.x; });
        const auto lower = upper-1;
        return LinearInterpolation(v, *lower, *upper);
    }

    inline double AntiParallel_1N4148_Current(const double v)
    {
        return Diode_1N4148_Current(v) - Diode_1N4148_Current(-v);
    }

} // namespace TRM