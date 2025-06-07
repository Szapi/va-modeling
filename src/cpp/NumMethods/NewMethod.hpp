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

#include "NewMethod_ODE_Descriptor.hpp"
#include "Utility.hpp"

namespace TRM::NewMethod
{
    // "A new adaptive nonlinear numerical method for singular and stiff differential problems"
    // <https://doi.org/10.1016/j.aej.2023.05.055>
    // This is a non-adaptive (fixed time step) implementation of the method.
    template<ODE_Descriptor ODE>
    class Executor
    {
    public:
        Executor(double h, double y0, ODE&& ode)
            : h{h}
            , y{y0}
            , derivativeCalculator{std::move(ode)}
        {}

        double DoOneStep()
        {
            const auto [f,b,c,d,e] = derivativeCalculator(y);

            TRM_DEFINE_POW(b, 2);
            TRM_DEFINE_POW(b, 3);
            TRM_DEFINE_POW(b, 4);

            TRM_DEFINE_POW(c, 2);
            TRM_DEFINE_POW(c, 3);

            TRM_DEFINE_POW(d, 2);

            TRM_DEFINE_POW(f, 2);
            TRM_DEFINE_POW(f, 3);
            
            TRM_DEFINE_POW(y, 2);
            
            TRM_DEFINE_POW(h, 2);
            TRM_DEFINE_POW(h, 3);

            const double alpha = ((-12)*c*e) + (15*d2);
            const double P     = (alpha*h2) + ((36*b*e-60*c*d)*h) - (180*b*d) + (240*c2);
            const double beta  = ((-90)*b2*d + (36*f*e + 120*c2)*b - 60*f*c*d)*h2;
            const double gamma = ((-72)*f2*e) - (360*b2*c);
            const double delta = (360*f2*d) - (1440*f*b*c) + (1080*b3);

            const double num = (P*y2) + (2*beta + (gamma + 480*f*c2)*h + delta)*y - 72*h*((f3*e - 5*f2*b*d - (10./3.)*f2*c2 + 15*f*b2*c - (15./2.)*b4)*h - 5*f3*d + 20*f2*b*c - 15*f*b3);
            const double den = (P*y) + ((-18)*b2*e + 60*b*c*d - alpha*f - 40*c3)*h3 + beta + (gamma + 180*f*b*d + 240*f*c2)*h + delta;

            y = num / den;
            return y;
        }

        double GetValue() const { return y; }

    private:
        const double h; // time step
        double y;
        ODE derivativeCalculator;
    };

} // namespace TRM::NewMethod
