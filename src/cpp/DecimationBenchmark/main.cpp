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
#include "Decimation.hpp"
#include "Prompt.hpp"
#include "Stopwatch.hpp"
#include "Utility.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <numeric>
#include <ranges>

using namespace std;
using namespace TRM;


int main ()
{
    const auto inputFile192 = Prompt<ExistingAudioFile> ("Enter input file path (192 kHz, > 10 samples)",
                                                         AllOf | SampleRate(192'000) | NonEmpty);
    inputFile192.printSummary();

    constexpr std::size_t LeftCh = 0u;

    const auto& in192 = inputFile192.samples[LeftCh];

    auto CompareRegularVsPolyphase = [&]<std::size_t ChunkSz>(std::integral_constant<std::size_t, ChunkSz>, const bool exportResult) -> void
    {
        static_assert(ChunkSz >= Decimation::D4x<ChunkSz>::Taps);
        static_assert(ChunkSz % 4 == 0);

        const auto Iterations = in192.size() / ChunkSz;
        const auto OutSz      = Iterations * ChunkSz / 4;

        vector<double> out48;
        out48.resize(OutSz);

        auto Flush = [&]
        {
            if (exportResult)
            {
                AudioFile<double> outputFile;
                outputFile.setSampleRate(48'000);
                outputFile.setBitDepth(24);
                outputFile.setAudioBuffer({move(out48)});
                auto outputFileName = Prompt<string>(" └ Enter output file name: ");
                if (!outputFile.save(outputFileName))
                {
                    cout << " ! Failed to write output file !\n";
                }
            }
            else
            {
                out48.clear();
            }
            out48.resize(OutSz);
        };

        auto RunBench = [&](auto method, std::string scenario)
        {
            auto src = in192.begin();
            auto dst = out48.begin();
            {
                Stopwatch sw{scenario};

                for (auto i = Iterations; i-->0;)
                {
                    src = method.Load(src);
                    dst = method.Apply(dst);
                }
            }
            Flush();
        };

        RunBench(Decimation::D4x<ChunkSz>{},      format("Naive approach (chunk size = {}):",     ChunkSz));
        RunBench(Decimation::D4x_Poly<ChunkSz>{}, format("Polyphase approach (chunk size = {}):", ChunkSz));
    };

    CompareRegularVsPolyphase(std::integral_constant<std::size_t, 128>{}, true);
}
