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

    // Returns a subarray of the compile-time array 'arr'
    template<std::size_t N, std::size_t Start>
    consteval auto EveryNth(const auto& arr)
    {
        std::array<double, (std::size(arr) - Start + N - 1) / N> result;
        auto i = Start;
        for(double& d : result)
        {
            d = arr[i];
            i += N;
        }
        return result;
    }

    namespace _UnitTests
    {
        inline constexpr std::array<double, 8> arr { 0., 1., 2., 3., 4., 5., 6., 7.};

        inline constexpr auto sub1 = EveryNth<4, 0>(arr);
        static_assert(sub1.size() == 2 && sub1[0] == 0. && sub1[1] == 4.);

        inline constexpr auto sub2 = EveryNth<4, 1>(arr);
        static_assert(sub2.size() == 2 && sub2[0] == 1. && sub2[1] == 5.);

        inline constexpr auto sub3 = EveryNth<4, 2>(arr);
        static_assert(sub3.size() == 2 && sub3[0] == 2. && sub3[1] == 6.);

        inline constexpr auto sub4 = EveryNth<4, 3>(arr);
        static_assert(sub4.size() == 2 && sub4[0] == 3. && sub4[1] == 7.);

        inline constexpr auto sub5 = EveryNth<4, 4>(arr);
        static_assert(sub5.size() == 1 && sub5[0] == 4.);

    } // namespace _UnitTest

} // namespace TRM
