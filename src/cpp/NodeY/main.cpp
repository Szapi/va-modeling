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

#include "Prompt.hpp"
#include "CircleBuffer.hpp"
#include "Stencil.hpp"
#include "FiniteDifferenceMethod.hpp"
#include "Utility.hpp"
#include "TS808Components.hpp"
#include "Decimation.hpp"

#include "AudioFile/AudioFile.h"

#include <iostream>
#include <cmath>
#include <format>
#include <filesystem>
#include <numeric>
#include <ranges>

using namespace std;
using namespace TRM;

struct ExistingAudioFile : AudioFile<double> { using AudioFile<double>::AudioFile; };

PROMPT_PART(ExistingWavPath, std::string, "Enter file path (.wav, must exist): ", [](const std::string& s){
    std::filesystem::path p {s};
    return std::filesystem::exists(p) && p.extension().compare(".wav") == 0;
});

PROMPT_PREFERENCES(ExistingAudioFile, ExistingWavPath);


class IIR_HighPass
{
public:
    IIR_HighPass(double a, double b) : a {a}, b {b} {}

    void Reset(double newA, double newB)
    {
        prevIn = prevOut = 0.0;
        a = newA;
        b = newB;
    }

    inline double operator()(double in)
    {
        const double out = std::fma(-a, prevOut, std::fma(-b, prevIn, b*in));
        prevIn  = in;
        prevOut = out;
        return out;
    }

private:
    double a, b;
    double prevIn  = 0.0;
    double prevOut = 0.0;
};

// Calculate the voltage in node 'Y' in the TS808 diode clipper circuit using both
// backwards Euler and IIR method.
int main ()
{
    auto inputFile48 = Prompt<ExistingAudioFile> ("Enter input guitar DI file (48 kHz, > 10 samples)",
                                                  [](const AudioFile<double>&) { return true; });
    inputFile48.printSummary();

    constexpr std::size_t LeftCh  = 0u;
    [[maybe_unused]] constexpr std::size_t RightCh = 1u;

    auto& in48 = inputFile48.samples[LeftCh];
    for(auto& x : in48) x = (10. * x - 4.5);

    constexpr double h = 1. / 48'000.;

    const auto din48 = [&, diff=FiniteDiff<h>{}]() mutable -> vector<double>
    {
        vector<double> result;
        result.reserve(in48.size());

        CircleBuffer<double, 7> buf;
        auto load = CreateBufferLoader(buf, in48);
        load(4);
        for ([[maybe_unused]]auto x : in48)
        {
            result.push_back(diff.FirstDerivative(buf));
            load(1);
        }
        return result;
    }();

    const auto len48 = in48.size();
    vector<double> out48;
    out48.reserve(len48);

    auto flush = [&](const string_view prompt)
    {
        AudioFile<double> outputFile;
        outputFile.setSampleRate(48'000);
        outputFile.setBitDepth(24);
        outputFile.setAudioBuffer({move(out48)});
        auto outputFileName = Prompt<string>(prompt, [](auto){ return true; });
        if (!outputFile.save(outputFileName))
        {
            cout << " ! Failed to write output file !\n";
        }
        out48.reserve(len48);
    };

    { // Backwards Euler method
        constexpr double K = 1. / (1. + h / (Cg * Rg));
        out48.push_back(0.0);
        for(unsigned i = 1; i < len48; ++i)
        {
            out48.push_back(out48[i-1]*K + din48[i]*K*h);
        }
    }
    flush("Backwards Euler method: ");

    { // IIR method
        IIR_HighPass iir {-0.909935877261752,  0.954967938630875};
        for(unsigned i = 0; i < len48; ++i)
        {
            out48.push_back(iir(in48[i]));
        }
    }
    flush("IIR method: ");
}
