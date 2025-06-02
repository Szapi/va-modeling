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

#include <chrono>
#include <format>
#include <iostream>
#include <string_view>

namespace TRM
{

    class Stopwatch
    {
        using clock = std::chrono::steady_clock;
    public:
        Stopwatch(std::string_view endMessage, std::ostream& out = std::cout)
            : startTime{clock::now()}
            , endMessage{endMessage}
            , out{out}
        {}

        ~Stopwatch()
        {
            using namespace std;
            using namespace std::chrono;
            const auto delta = clock::now() - startTime;
            auto print = [&](auto Ratio)
            {
                out << format("{} {:%Q %q}\n", endMessage, duration_cast<duration<double, decltype(Ratio)>>(delta));
            };
            if (delta < microseconds{1000})
                print(micro{});
            else if (delta < milliseconds{1000})
                print(milli{});
            else
                print(ratio<1>{});
        }

    private:
        const typename clock::time_point startTime;
        const std::string_view endMessage;
        std::ostream& out;
    };

} // namespace TRM
