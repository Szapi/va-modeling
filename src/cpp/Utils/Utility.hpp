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

#include <algorithm>
#include <cmath>
#include <utility>

inline constexpr double Eps6  = 1.e-6;
inline constexpr double Eps9  = 1.e-9;
inline constexpr double Eps12 = 1.e-12;

#define TRM_CONSTEXPR inline static constexpr

#define TRM_DEFINE_POW(x,exp) const double x ## exp = std::pow(x, exp)

#ifdef TRM_ENABLE_DEBUG_MACROS
    #include <string>
    #include <iostream>
    #define TRM_DBG_PRINT(x) std::cout << " < DEBUG > " << #x << " = " << x
    #define TRM_DBG_PRINT_NL(x) std::cout << " < DEBUG > " << #x << " = " << x << '\n'
    #define TRM_DBG_BREAK std::cout << " < DEBUG BREAK >\n\n"; { std::string TRM_DBG_BREAK_local_string; std::cin >> TRM_DBG_BREAK_local_string; }
    #define TRM_DBG_PRINT_BREAK(x) TRM_DBG_PRINT_NL(x); TRM_DBG_BREAK
#else
    #define TRM_DBG_PRINT(x)
    #define TRM_DBG_PRINT_NL(x)
    #define TRM_DBG_BREAK
    #define TRM_DBG_PRINT_BREAK(x)
#endif

namespace TRM
{

    struct Measurement
    {
        double x, y;
    };

    inline double LinearInterpolation(const double xIn,
                                      const Measurement &lower,
                                      const Measurement &upper)
    {
        const double x = std::clamp(xIn, lower.x, upper.x);
        return std::lerp(lower.y, upper.y, (x - lower.x) / (upper.x - lower.x));
    }
    
    template<std::size_t N, std::size_t Start, std::size_t Sz>
    consteval auto EveryNth(const std::array<double, Sz>& arr)
    {
        std::array<double, (Sz - Start + N - 1) / N> result;
        for(std::size_t i = 0; i < result.size(); ++i)
        {
            result[i] = arr[Start + N*i];
        }
        return result;
    }

} // namespace TRM
