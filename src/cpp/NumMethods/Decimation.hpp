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

#include "Utility.hpp"
#include "CarryoverBuffer.hpp"

#include <numeric>

namespace TRM
{

    template<std::size_t ChunkSz, std::size_t SourceSkip, auto Coeffs, auto OutputOp>
    struct FIR_Base
    {
        TRM_CONSTEXPR std::size_t Taps   = std::size(Coeffs);
        TRM_CONSTEXPR std::size_t TailSz = ((Taps - 1) / SourceSkip) * SourceSkip;

        CarryoverBuffer<ChunkSz, TailSz> buf{};

        auto Load(auto src) -> decltype(src)
        {
            std::copy_n(src, ChunkSz, buf.Carry());
            return src + ChunkSz;
        }

        auto Apply(auto dst) -> decltype(dst)
        {
            static_assert(ChunkSz % SourceSkip == 0);
            auto it = begin(buf);
            for (auto n = ChunkSz/SourceSkip; n-->0;)
            {
                OutputOp(*dst, std::inner_product(begin(Coeffs), end(Coeffs), it, 0.0));
                it  += SourceSkip;
                dst += 1;
            }
            return dst;
        }
    };

    namespace Decimation
    {
        constexpr auto D4x_Coeffs = std::array{
                0.0007589325224462893,
                0.002286074267442946,
                0.0033826242490856027,
                0.0048663705514655695,
                0.005309021855755256,
                0.00480516805522579,
                0.0029933866153971974,
                0.0004634417759766627,
                -0.0020636899377087215,
                -0.003590275407087308,
                -0.0034938957224134558,
                -0.0017276480817566841,
                0.0009935560457884059,
                0.003466331188040143,
                0.004471536520649566,
                0.0033689539110971158,
                0.0004660048489271437,
                -0.0030110407041343684,
                -0.005406480263527812,
                -0.005396222175403833,
                -0.002686721399518115,
                0.001708495464902097,
                0.005806100580951971,
                0.007516884003490526,
                0.0056769985779755675,
                0.0007471146309467735,
                -0.005203118903517638,
                -0.009320427016922076,
                -0.00929097522393258,
                -0.004569758720076178,
                0.003107956468700087,
                0.010284621234052094,
                0.013269364763137278,
                0.009948374668119533,
                0.001064889484072712,
                -0.009732798061335835,
                -0.017272825634327416,
                -0.01719349669995676,
                -0.008234942864250052,
                0.006627674819267409,
                0.020921774672168714,
                0.02721485692271923,
                0.020545410339426248,
                0.0012975926528935129,
                -0.023847659531530205,
                -0.043581629745100506,
                -0.04588362565279871,
                -0.022805717156306925,
                0.025740681918154637,
                0.09083359250658753,
                0.15669681177618142,
                0.20557214728101578,
                0.22360433681744618,
                0.20557214728101578,
                0.15669681177618142,
                0.09083359250658753,
                0.025740681918154637,
                -0.022805717156306925,
                -0.04588362565279871,
                -0.043581629745100506,
                -0.023847659531530205,
                0.0012975926528935129,
                0.020545410339426248,
                0.02721485692271923,
                0.020921774672168714,
                0.006627674819267409,
                -0.008234942864250052,
                -0.01719349669995676,
                -0.017272825634327416,
                -0.009732798061335835,
                0.001064889484072712,
                0.009948374668119533,
                0.013269364763137278,
                0.010284621234052094,
                0.003107956468700087,
                -0.004569758720076178,
                -0.00929097522393258,
                -0.009320427016922076,
                -0.005203118903517638,
                0.0007471146309467735,
                0.0056769985779755675,
                0.007516884003490526,
                0.005806100580951971,
                0.001708495464902097,
                -0.002686721399518115,
                -0.005396222175403833,
                -0.005406480263527812,
                -0.0030110407041343684,
                0.0004660048489271437,
                0.0033689539110971158,
                0.004471536520649566,
                0.003466331188040143,
                0.0009935560457884059,
                -0.0017276480817566841,
                -0.0034938957224134558,
                -0.003590275407087308,
                -0.0020636899377087215,
                0.0004634417759766627,
                0.0029933866153971974,
                0.00480516805522579,
                0.005309021855755256,
                0.0048663705514655695,
                0.0033826242490856027,
                0.002286074267442946,
                0.0007589325224462893
            };

        template<std::size_t ChunkSz>
        using D4x = FIR_Base<ChunkSz, 4, D4x_Coeffs, [](double& d, double v){ d = v; }>;

        template<std::size_t ChunkSz>
        class D4x_Poly
        {
            static_assert(ChunkSz % 4 == 0);
            TRM_CONSTEXPR std::size_t SubChunkSz = ChunkSz / 4;

            using D4x_Poly_1 = FIR_Base<SubChunkSz, 1, EveryNth<4, 0>(D4x_Coeffs), [](double& d, double v){ d  = v; }>;
            using D4x_Poly_2 = FIR_Base<SubChunkSz, 1, EveryNth<4, 1>(D4x_Coeffs), [](double& d, double v){ d += v; }>;
            using D4x_Poly_3 = FIR_Base<SubChunkSz, 1, EveryNth<4, 2>(D4x_Coeffs), [](double& d, double v){ d += v; }>;
            using D4x_Poly_4 = FIR_Base<SubChunkSz, 1, EveryNth<4, 3>(D4x_Coeffs), [](double& d, double v){ d += v; }>;

            D4x_Poly_1 p1;
            D4x_Poly_2 p2;
            D4x_Poly_3 p3;
            D4x_Poly_4 p4;

        public:
            auto Load(auto src) -> decltype(src)
            {   
                auto ReadTo = [&src](auto& it)
                {
                    *it = *src;
                    ++src;
                    ++it;
                };
                // Manually load them into the buffers
                auto it1 = p1.buf.Carry();
                auto it2 = p2.buf.Carry();
                auto it3 = p3.buf.Carry();
                auto it4 = p4.buf.Carry();
                for(auto n = SubChunkSz; n-->0;)
                {
                    ReadTo(it1);
                    ReadTo(it2);
                    ReadTo(it3);
                    ReadTo(it4);
                }
                return src;
            }

            auto Apply(auto dst) -> decltype(dst)
            {
                // Assemble the output on the stack rather than directly in dst
                std::array<double, SubChunkSz> outBuf;
                p1.Apply(begin(outBuf));
                p2.Apply(begin(outBuf));
                p3.Apply(begin(outBuf));
                p4.Apply(begin(outBuf));
                return std::copy_n(begin(outBuf), SubChunkSz, dst);
            }
        };

    } // namespace Decimation

} // namespace TRM
