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

#include "AudioFilePrompt.hpp"
#include "CircleBuffer.hpp"
#include "FiniteDifferenceMethod.hpp"
#include "TS808Components.hpp"
#include "Utility.hpp"

#include <fstream>
#include <functional>
#include <ranges>

using namespace std;
using namespace TRM;

constexpr int ExpectedSampleRate = 48'000;

int main ()
{
    const auto CorrectInputFile = AllOf | SampleRate(ExpectedSampleRate) | NonEmpty;

    auto vOP_P_d_wav = Prompt<ExistingAudioFile> ("Path to V(OP_P_d) (48 kHz): ", CorrectInputFile);
    vOP_P_d_wav.printSummary();

    auto vOP_N_d_wav = Prompt<ExistingAudioFile> ("Path to V(OP_N_d): ", CorrectInputFile);
    vOP_N_d_wav.printSummary();

    auto vOP_OUT_d_wav = Prompt<ExistingAudioFile> ("Path to V(OP_OUT_d): ", CorrectInputFile);
    vOP_OUT_d_wav.printSummary();

    auto vY_d_wav = Prompt<ExistingAudioFile> ("Path to V(Y_d): ", CorrectInputFile);
    vY_d_wav.printSummary();

    // Hack: LTspice can only output integer wav, which would be clipped where the values
    // are outside of [-1, 1] --> output 0.1 * value --> need to compensate here
    auto ScaleBy = [](const double s, vector<double>& v)
    {
        for (double& d : v) d *= s;
    };
    ScaleBy(10., vOP_P_d_wav.samples[0]);
    ScaleBy(10., vOP_N_d_wav.samples[0]);
    ScaleBy(10., vOP_OUT_d_wav.samples[0]);
    ScaleBy(10., vY_d_wav.samples[0]);

    const vector<double>& OP_P = vOP_P_d_wav.samples[0];
    const vector<double>& OP_N = vOP_N_d_wav.samples[0];
    const vector<double>& OP_OUT = vOP_OUT_d_wav.samples[0];
    const vector<double>& Y = vY_d_wav.samples[0];

    ofstream out{"verified.txt"};
    out << "I(Rg)\tI(Feedback Loop)\n";
    out << setprecision(11);

    for (int i = 0; i < 3; ++i)
    {
        out << 0.0 << '\t' << 0.0 << '\n';
    }

    auto zippedView = views::zip(
        OP_P | views::adjacent<7>,
        OP_N | views::adjacent<7>,
        OP_OUT | views::adjacent<7>,
        views::transform(Y, [](double d){ return d - OpAmpBias; }) | views::adjacent<7>,
        views::zip_transform(minus{}, OP_OUT, OP_P) | views::adjacent<7>
    );

    FiniteDiff<1. / ExpectedSampleRate> diff;

    for (const auto [op_p, op_n, op_out, y, delta] : zippedView)
    {
        // Current through ground resistor
        out << get<3>(y)/Rg << '\t';

        // Current through feedback components
        const double d = get<3>(delta);
        out << (diff.FirstDerivative(delta) * Cf + d/Rf + Diode_1N4148_Current(d) - Diode_1N4148_Current(-d)) << '\n';
    }
}
