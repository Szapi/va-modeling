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
#include "NewMethod.hpp"
#include "TS808Components.hpp"
#include "Utility.hpp"
#include "CircleBuffer.hpp"
#include "FiniteDifferenceMethod.hpp"

#include <cmath>
#include <format>
#include <iostream>
#include <ranges>

using namespace std;
using namespace TRM;

int main()
{
    const auto inputFile192 = Prompt<ExistingAudioFile> ("Enter input guitar DI file (192 kHz, > 10 samples, stereo)",
                                                         AllOf | NonEmpty | Stereo | SampleRate(192'000));
    inputFile192.printSummary();

    constexpr std::size_t LeftCh  = 0u;
    constexpr std::size_t RightCh = 1u;

    constexpr double FullScaleSampleVoltage = 3.88;

    auto ToCorrectVoltage = [](double d){ return (d * FullScaleSampleVoltage); };

    constexpr double h = 1. / (48'000);

    auto in48 = inputFile192.samples[LeftCh]  | views::stride(4) | views::transform(ToCorrectVoltage);
    auto y48  = inputFile192.samples[RightCh] | views::stride(4) | views::transform(ToCorrectVoltage);

    cout << " ! Assuming 0 dBFS = " << FullScaleSampleVoltage << " V !\n";
    cout << " ! Assuming Left channel is raw guitar DI, Right channel is 720 Hz high-passed version of Left channel !\n";

    constexpr double Gain = 0.0; // From 0 to 1
    constexpr double VT = 0.02677; // Volts
    constexpr double n  = 1.92;
    constexpr double A = 1. / (Rg * Cf);
    constexpr double B = - (Rf + Gain*Rd)/Cf;
    constexpr double D = 1. / (VT * n);
    constexpr double C = (-2.) * D / Cf;

    CircleBuffer<double, 7> yBuf{};
    auto LoadY = CreateBufferLoader(yBuf, y48);
    LoadY(4);

    // Used equation (2.7) for this, therefore less accurate, but it doesn't matter, it is unstable anyway
    auto derivatives = [&, diff = FiniteDiff<h>{}](const double delta) mutable {
        const double S = std::sinh(D*delta);
        const double K = std::cosh(D*delta);
        const double f0 = A*yBuf.Get<3>() + B*delta + C*S;
        const double f1 = A*diff.FirstDerivative(yBuf) + f0*(B + C*D*K);
        const double f2 = A*diff.SecondDerivative(yBuf) + f1*(B + C*D*K) + C*D*D*f0*f0*S;
        const double f3 = A*diff.ThirdDerivative(yBuf) + f2*(B + C*D*K) + 3*C*D*D*f0*f1*S + C*D*D*D*f0*f0*f0*K;
        const double f4 = A*diff.FourthDerivative(yBuf) + f3*(B + C*D*K) + 4*C*D*D*f0*f2*S + 6*C*D*D*D*f0*f0*f1*K + 3*C*D*D*f1*f1*S + C*D*D*D*D*f0*f0*f0*f0*S;
        LoadY(1);
        return NewMethod::Derivatives{.first = f0, .second = f1, .third = f2, .fourth = f3, .fifth = f4};
    };
    NewMethod::Executor x(h, 0.0, move(derivatives));

    std::vector<double> out;
    out.reserve(in48.size());

    for (const double d : in48)
    {
        out.push_back((d + x.DoOneStep()) / FullScaleSampleVoltage);
    }

    AudioFile<double> outputFile;
    outputFile.setSampleRate(48'000);
    outputFile.setBitDepth(24);
    outputFile.setAudioBuffer({move(out)});
    auto outputFileName = Prompt<string>("Enter output file name: ");
    if (!outputFile.save(outputFileName))
    {
        cout << " ! Failed to write output file !\n";
    }
}
