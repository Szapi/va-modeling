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

#include <algorithm>
#include <array>

namespace TRM
{

    template<std::size_t HeadSz, std::size_t TailSz>
    struct CarryoverBuffer
    {
        static_assert(HeadSz >= TailSz);

        std::array<double, HeadSz + TailSz> buf{};

        auto Carry()
        {
            return std::copy_n(begin(buf) + HeadSz, TailSz, begin(buf));
        }

        friend auto begin(      CarryoverBuffer<HeadSz, TailSz>& cob) { return begin(cob.buf); }
        friend auto begin(const CarryoverBuffer<HeadSz, TailSz>& cob) { return begin(cob.buf); }
    };
    
} // namespace TRM
