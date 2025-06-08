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

#include "NewMethod.hpp"
#include "Prompt.hpp"

using namespace TRM;
using namespace std;

int main(int argC, char** argVal)
{
    // Recreation of Problem 3 from
    // "A new adaptive nonlinear numerical method for singular and stiff differential problems"
    // <https://doi.org/10.1016/j.aej.2023.05.055>
    auto derivatives = [](const double y) {
        const double q = (0.25 - y/40.);
        const double f0 = y * 0.25 * (1.0 - y/20.);
        const double f1 = f0 * q;
        const double f2 = f1*q + f0*f0 / 40.;
        const double f3 = f2*q + f1*f0 / 40.;
        const double f4 = f3*q + f1*f1 / 40.;
        return NewMethod::Derivatives{
            .first  = f0,
            .second = f1,
            .third  = f2,
            .fourth = f3,
            .fifth  = f4
        };
    };

    const auto Positive = [](auto x) -> bool { return x > static_cast<decltype(x)>(0); };

    const double h  = Prompt<double>("Time step (h > 0): "sv, Positive);
    const double y0 = Prompt<double>("Initial value: "sv);

    NewMethod::Executor x(h, y0, move(derivatives));

    const int totalSteps            = Prompt<int>("Total steps (N > 0): "sv, Positive);
    const int stepsBetweenPrintouts = Prompt<int>("Steps between printouts (P > 0): "sv, Positive);

    for(int t = 1; t <= totalSteps; ++t)
        if (const double y = x.DoOneStep(); t % stepsBetweenPrintouts == 0)
            cout << format(" t = {:5.2f}     y(t) ~ {:7.4f}\n", t*h, y);

    return 0;
}
