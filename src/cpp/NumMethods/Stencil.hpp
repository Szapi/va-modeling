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

#include <tuple>
#include <utility>

namespace TRM
{
    namespace _Impl
    {
        template<class T>
        constexpr size_t StencilSizeImpl = std::size(T{});

        template<class... Ts>
        constexpr size_t StencilSizeImpl<std::tuple<Ts...>> = sizeof...(Ts);
    }

    template<class T>
    constexpr size_t StencilSize = _Impl::StencilSizeImpl<std::remove_cvref_t<T>>;

    // A 1-dimensional mathematical N-point stencil around a designated point consists of
    // the point itself as well as (N-1)/2 neighbouring points on both sides.
    template<std::size_t N>
    constexpr bool ValidStencilSize = (N > 2) && (N % 2 == 1);

    // Dot product, but using stencils, aligned by their central points. Example:
    // _ _ x x O x x _ _ <-- Coeffs...
    // x x x x O x x x x <-- stencil
    template<double...Coeffs, class S>
    constexpr double StencilProduct(const S& stencil) noexcept
    {
        constexpr auto CoeffsSz  = sizeof...(Coeffs);
        constexpr auto StencilSz = StencilSize<S>;

        static_assert(ValidStencilSize<CoeffsSz>);
        static_assert(ValidStencilSize<StencilSz>);
        static_assert(CoeffsSz <= StencilSz);

        constexpr std::size_t StencilOffset = (StencilSz - CoeffsSz) / 2;
        return [&]<std::size_t...I>(std::index_sequence<I...>)
        {
            return ((Coeffs * std::get<I + StencilOffset>(stencil)) + ... );
        } (std::make_index_sequence<CoeffsSz>());
    }

} // namespace TRM
