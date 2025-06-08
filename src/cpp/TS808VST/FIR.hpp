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

#include <numeric>
#include <array>
#include <type_traits>

namespace TRM
{
    template<class...D>
    consteval auto FIR(D&&...d)
    {
        static_assert(sizeof...(D) > 0, "FIR filter must have at least one coefficient");
        static_assert((std::is_same_v<std::remove_cvref_t<D>, double> && ...), "All coefficients must be doubles");

        return [coefs = std::array<double, sizeof...(D)>{{d...}}](auto begIt, const std::size_t size, auto dstIt) constexpr -> void
        {
            for (std::size_t i = 0; i < size; ++i)
            {
                *dstIt = std::inner_product(begin(coefs), end(coefs), begIt, 0.0);
                ++begIt;
                ++dstIt;
            }
        };
    }

    template<class...D>
    consteval auto Basic_FIR(D&&...d)
    {
        return [impl = FIR(d...)](auto begIt) constexpr -> double
        {
            double out = 0.0;
            impl(begIt, 1, &out);
            return out;
        };
    }

} // namespace TRM
