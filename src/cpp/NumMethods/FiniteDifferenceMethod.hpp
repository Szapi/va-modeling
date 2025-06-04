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

#include "Stencil.hpp"

namespace TRM
{

    // 7-point stencil central finite difference method
    template<double h>
    struct FiniteDiff
    {
        inline constexpr double FirstDerivative(const auto& s) noexcept
        {
            return StencilProduct<-1., 9., -45., 0., 45., -9., 1.>(s) /
                   (60 * h);
        }

        inline constexpr double SecondDerivative(const auto& s) noexcept
        {
            return StencilProduct<2., -27., 270., -490., 270., -27., 2.>(s) /
                   (180 * h * h);
        }

        inline constexpr double ThirdDerivative(const auto& s) noexcept
        {
            return StencilProduct<1., -8., 13., 0., -13., 8., -1.>(s) /
                   (8 * h * h * h);
        }

        inline constexpr double FourthDerivative(const auto& s) noexcept
        {
            return StencilProduct<-1., 12., -39., 56., -39., 12., -1.>(s) /
                   (6 * h * h * h * h);
        }
    };

} // namespace TRM