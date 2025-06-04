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

#include <array>

namespace TRM
{
    // Primitive circle buffer class.
    // Can be used as a stencil, thanks to the std::get specialization.
    // Not very performant though, so only used for prototyping.
    template<class T, std::size_t Sz>
    class CircleBuffer
    {
    public:
        constexpr CircleBuffer() = default;
    
        T& RotateLeft ()
        {
            const auto idxLastItemAfterRotate = idxBegin;
            idxBegin = (idxBegin + 1u) % Sz;
            return buf[idxLastItemAfterRotate];
        }

        template<std::size_t I> requires (I < Sz)
        T& Get() { return buf[(idxBegin + I) % Sz]; }

        template<std::size_t I> requires (I < Sz)
        const T& Get() const { return buf[(idxBegin + I) % Sz]; }

    private:
        std::array<T, Sz> buf {};
        std::size_t idxBegin = 0u;
    };

} // namespace TRM

namespace std
{

    template<std::size_t I, class T, std::size_t Sz>
    T& get(TRM::CircleBuffer<T, Sz>& cb)
    {
        return cb.template Get<I>();
    }

    template<std::size_t I, class T, std::size_t Sz>
    const T& get(const TRM::CircleBuffer<T, Sz>& cb)
    {
        return cb.template Get<I>();
    }

    template<class T, std::size_t Sz>
    consteval std::size_t size(const TRM::CircleBuffer<T, Sz>&)
    {
        return Sz;
    }
    
} // namespace std
