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
#include "RungeKutta4.hpp"
#include "TS808Components.hpp"
#include "Utility.hpp"

#include <cmath>
#include <format>
#include <iostream>
#include <ranges>

using namespace std;
using namespace TRM;

int main ()
{
    const auto inputFile192 = Prompt<ExistingAudioFile> ("Enter input guitar DI file (192 kHz, > 10 samples, stereo)",
                                                         AllOf | NonEmpty | Stereo | SampleRate(192'000));
    inputFile192.printSummary();

    constexpr std::size_t LeftCh  = 0u;
    constexpr std::size_t RightCh = 1u;

    constexpr double FullScaleSampleVoltage = 3.88;

    auto ToCorrectVoltage = [](double d){ return (d * FullScaleSampleVoltage); };

    // RK4 will run at 48'000 Hz, therefore we need 96'000 Hz input data
    auto in96 = inputFile192.samples[LeftCh]  | views::stride(2) | views::transform(ToCorrectVoltage);
    auto y96  = inputFile192.samples[RightCh] | views::stride(2) | views::transform(ToCorrectVoltage);

    cout << " ! Assuming 0 dBFS = " << FullScaleSampleVoltage << " V !\n";
    cout << " ! Assuming Left channel is raw guitar DI, Right channel is 720 Hz high-passed version of Left channel !\n";

    const auto diffIn96File = Prompt<ExistingAudioFile>("Enter guitar DI 1st derivative wav file (96 kHz)",
                                                        AllOf | NonEmpty | SampleRate(96'000));
    diffIn96File.printSummary();
    constexpr double diffWavScale = 1. / 0.00033;
    auto diffIn96  = diffIn96File.samples[LeftCh] | views::transform([](double d){ return d * diffWavScale; });

    int cur = 0;
    auto diffEquationDescriptor = [&]<class T>(const T&, const double x)
    {
        if constexpr(std::is_same_v<T, RK4::TimeStep>)
        {
            cur += 2;
            return;
        }
        else
        {
            const int    idx   = cur + static_cast<int>(T::lookahead);
            const double in    = in96[idx];
            const double din   = diffIn96[idx];
            const double y     = y96[idx];
            const double delta = x - in;
            return din + (y/Rg - delta/Rf - AntiParallel_1N4148_Current(delta)) / Cf;
        }
    };

    RK4::Executor rk4{std::integral_constant<double, 1. / 48'000>{}, 0.0, std::move(diffEquationDescriptor)};

    vector<double> output48;
    output48.reserve(inputFile192.samples[LeftCh].size() / 4);
    output48.push_back(0.0);
    do
    {
        output48.push_back(rk4.DoOneStep());
    } while ((cur + 2) < static_cast<int>(in96.size()));

    while(output48.size() < (in96.size() / 2)) output48.push_back(0.0);

    for(double& d : output48) d = d / 10.;

    AudioFile<double> outputFile;
    outputFile.setSampleRate(48'000);
    outputFile.setBitDepth(24);
    outputFile.setAudioBuffer({move(output48)});
    auto outputFileName = Prompt<string>("Enter output file name (op amp output / 10): ");
    if (!outputFile.save(outputFileName))
    {
        cout << " ! Failed to write output file !\n";
    }
}
