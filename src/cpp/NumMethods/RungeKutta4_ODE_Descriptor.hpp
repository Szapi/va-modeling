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

#include <concepts>

namespace TRM::RK4
{

    template<std::size_t N>
    struct Lookahead { inline static constexpr auto lookahead = N; };

    struct TimeStep {};

    template<class Type>
    concept ODE_Descriptor = requires(Type& obj, double y, TimeStep timeStep)
    {
        { obj(Lookahead<0>{}, y) } -> std::convertible_to<double>;
        { obj(Lookahead<1>{}, y) } -> std::convertible_to<double>;
        { obj(Lookahead<2>{}, y) } -> std::convertible_to<double>;
        obj(timeStep, y);
    };

} // namespace TRM::RK4
