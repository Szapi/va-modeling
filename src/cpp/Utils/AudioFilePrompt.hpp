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

#include "AudioFile/AudioFile.h"
#include "Prompt.hpp"

#include <filesystem>
#include <string>

namespace TRM
{

    struct ExistingAudioFile : AudioFile<double> { using AudioFile<double>::AudioFile; };

    PROMPT_PART(ExistingWavPath, std::string, "Enter file path (.wav, must exist): ", [](const std::string& s){
        const std::filesystem::path p {s};
        return std::filesystem::exists(p) && p.extension().compare(".wav") == 0;
    });

    // Common prompt predicates
    constexpr auto NonEmpty = [](const ExistingAudioFile& file) -> bool { return file.samples[0].size() >  10u; };
    constexpr auto Stereo   = [](const ExistingAudioFile& file) -> bool { return file.samples.size()    >= 2u ; };

    constexpr auto SampleRate(const auto expected)
    {
        auto e = static_cast<std::uint32_t>(expected);
        return [e](const ExistingAudioFile& file){ return file.getSampleRate() == e; };
    }

} // namespace TRM

PROMPT_PREFERENCES(TRM::ExistingAudioFile, TRM::ExistingWavPath);
