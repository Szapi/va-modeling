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

#include <cmath>

namespace TRM
{

    class IIR_HighPass
    {
    public:
        IIR_HighPass(double a, double b) : a {a}, b {b} {}

        inline double operator()(double in)
        {
            const double bin = b*in;
            const double out = std::fma(-a, prevOut, -prevBin + bin);
            prevBin  = bin;
            prevOut = out;
            return out;
        }

    private:
        const double a;
        const double b;
        double prevBin  = 0.0;
        double prevOut = 0.0;
    };

    struct IIR_3_2
    {
        double b0, b1, b2, a1, a2;
    };

    class IIR_3_2_Executor
    {
    public:
        IIR_3_2_Executor(IIR_3_2 coefs) : coefs {coefs} {}

        void UpdateCoefs(const IIR_3_2& coefs)
        {
            this->coefs = coefs;
        }

        inline double operator()(double in)
        {
            const double out = std::fma(coefs.b0, in, std::fma(coefs.b1, z1, std::fma(coefs.b2, z2, -std::fma(coefs.a1, prevOut, std::fma(coefs.a2, prevPrevOut, 0.0)))));
            z2 = z1;
            z1 = in;
            prevPrevOut = prevOut;
            prevOut = out;
            return out;
        }

    private:
        IIR_3_2 coefs;
        double z1 = 0.0;
        double z2 = 0.0;
        double prevOut = 0.0;
        double prevPrevOut = 0.0;
    };

} // namespace TRM
