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

#include "RungeKutta4_ODE_Descriptor.hpp"

namespace TRM::RK4
{

    template<double H, ODE_Descriptor ODE>
    class Executor
    {
    public:
        Executor(std::integral_constant<double, H>, double y0, ODE&& ode)
            : y{y0}
            , odeDescriptor{std::move(ode)}
        {}

        double DoOneStep()
        {
            const double k_1 = odeDescriptor(Lookahead<0>{}, y);
            const double k_2 = odeDescriptor(Lookahead<1>{}, y + .5 * H * k_1);
            const double k_3 = odeDescriptor(Lookahead<1>{}, y + .5 * H * k_2);
            const double k_4 = odeDescriptor(Lookahead<2>{}, y + H * k_3);

            y += (k_1 + 2.*k_2 + 2.*k_3 + k_4) * H / 6.;
            odeDescriptor(TimeStep{}, 0.0);
            return y;
        }

        double GetValue() const { return y; }

    private:
        double y;
        ODE odeDescriptor;
    };
    
} // namespace TRM::RK4
