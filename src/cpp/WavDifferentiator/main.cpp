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
#include "Utility.hpp"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <numeric>
#include <ranges>
#include <utility>

using namespace std;
using namespace TRM;

int main ()
{
    const auto inputFile192 = Prompt<ExistingAudioFile>("Enter input guitar DI file (192 kHz, > 10 samples, stereo)",
                                                        AllOf | SampleRate(192'000) | NonEmpty | Stereo);
    inputFile192.printSummary();

    constexpr std::size_t LeftCh = 0u;

    constexpr double FullScaleSampleVoltage = 3.88;

    auto ToCorrectVoltage = [](double d){ return (d * FullScaleSampleVoltage); };

    auto in96 = inputFile192.samples[LeftCh] | views::stride(2) | views::transform(ToCorrectVoltage);

    cout << " ! Assuming 0 dBFS = " << FullScaleSampleVoltage << " V !\n";
    cout << " ! Assuming Left channel is raw guitar DI !\n";

    // -----------------

    FiniteDiff<1./96000.> diff;

    CircleBuffer<double, 7> in;
    auto LoadIn = CreateBufferLoader(in, in96);
    LoadIn(3);

    vector<double> out_96;
    out_96.reserve(inputFile192.samples[LeftCh].size() / 2);

    for ([[maybe_unused]] auto d : in96)
    {
        LoadIn(1);
        out_96.push_back(diff.FirstDerivative(in));
    }

    for(auto& d : out_96) d = d * 0.00033; // Scale by this magic number to avoid clipping

    AudioFile<double> outputFile;
    outputFile.setSampleRate(96'000);
    outputFile.setBitDepth(24);
    outputFile.setAudioBuffer({move(out_96)});
    auto outputFileName = Prompt<string>("Enter output file name: ", [](auto){ return true; });
    if (!outputFile.save(outputFileName))
    {
        cout << " ! Failed to write output file !\n";
    }
}
