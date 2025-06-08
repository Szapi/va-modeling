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

    template<std::size_t ChunkSz, bool OnHeap, class Impl>
    struct FIR_Base
    {
        static_assert(ChunkSz % Impl::SourceSkip == 0);

        TRM_CONSTEXPR std::size_t Taps   = std::size(Impl::Coeffs);
        TRM_CONSTEXPR std::size_t TailSz = ((Taps - 1) / Impl::SourceSkip) * Impl::SourceSkip;
        // Alternative formula, should be equivalent:
        static_assert(TailSz == (Taps - 1) - ((Taps - 1) % Impl::SourceSkip));

        using WorkBuffer = CarryoverBuffer<ChunkSz, TailSz>;
        using SaveBuffer = typename WorkBuffer::SaveBuffer;

        std::conditional_t<OnHeap, SaveBuffer, WorkBuffer> persistentBuf;

    private:
        static auto ApplyImpl(auto dst, const WorkBuffer& workBuf) -> decltype(dst)
        {
            auto it = begin(workBuf);
            for (auto n = ChunkSz/Impl::SourceSkip; n-->0;)
            {
                Impl::OutputOp(*dst, std::inner_product(begin(Impl::Coeffs), end(Impl::Coeffs), it, 0.0));
                it  += Impl::SourceSkip;
                dst += 1;
            }
            return dst;
        }

    public:
        // Implementation when allocated on heap
        auto Load(auto src, WorkBuffer& workBuf) -> decltype(src) requires (OnHeap)
        {
            std::copy_n(src, ChunkSz, workBuf.Restore(persistentBuf));
            return src + ChunkSz;
        }
        auto Apply(auto dst, const WorkBuffer& workBuf) -> decltype(dst) requires (OnHeap)
        {
            const auto result = ApplyImpl(dst, workBuf);
            workBuf.Save(persistentBuf);
            return result;
        }

        // Implementation when allocated on stack
        auto Load(auto src) -> decltype(src) requires (!OnHeap)
        {
            std::copy_n(src, ChunkSz, persistentBuf.Carry());
            return src + ChunkSz;
        }
        auto Apply(auto dst) -> decltype(dst) requires (!OnHeap)
        {
            return ApplyImpl(dst, persistentBuf);
        }
    };

#define TRM_FIR_IMPL(Name,_SourceSkip,_Coeffs,_Op)      \
struct Name {                                           \
    TRM_CONSTEXPR std::size_t SourceSkip = _SourceSkip; \
    TRM_CONSTEXPR auto Coeffs   = _Coeffs;              \
    TRM_CONSTEXPR auto OutputOp = [](double& d, double v) _Op; \
}

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

        TRM_FIR_IMPL(D4x_Impl, 4, D4x_Coeffs, { d = v; });

        template<std::size_t ChunkSz, bool OnHeap>
        using D4x = FIR_Base<ChunkSz, OnHeap, D4x_Impl>;

        template<std::size_t ChunkSz, bool OnHeap>
        class D4x_Poly
        {
            TRM_FIR_IMPL(D4x_Poly_1_Impl, 1, (EveryNth<4, 0>(D4x_Coeffs)), { d  = v; });
            TRM_FIR_IMPL(D4x_Poly_2_Impl, 1, (EveryNth<4, 1>(D4x_Coeffs)), { d += v; });
            TRM_FIR_IMPL(D4x_Poly_3_Impl, 1, (EveryNth<4, 2>(D4x_Coeffs)), { d += v; });
            TRM_FIR_IMPL(D4x_Poly_4_Impl, 1, (EveryNth<4, 3>(D4x_Coeffs)), { d += v; });

            static_assert(ChunkSz % 4 == 0);
            TRM_CONSTEXPR std::size_t SubChunkSz = ChunkSz / 4;

            using Poly_1 = FIR_Base<SubChunkSz, OnHeap, D4x_Poly_1_Impl>;
            using Poly_2 = FIR_Base<SubChunkSz, OnHeap, D4x_Poly_2_Impl>;
            using Poly_3 = FIR_Base<SubChunkSz, OnHeap, D4x_Poly_3_Impl>;
            using Poly_4 = FIR_Base<SubChunkSz, OnHeap, D4x_Poly_4_Impl>;

            Poly_1 p1;
            Poly_2 p2;
            Poly_3 p3;
            Poly_4 p4;

        public:
            struct WorkBuffer
            {
                typename Poly_1::WorkBuffer buf1;
                typename Poly_2::WorkBuffer buf2;
                typename Poly_3::WorkBuffer buf3;
                typename Poly_4::WorkBuffer buf4;
            };

            // Implementation when allocated on heap
            auto Load(auto src, WorkBuffer& workBuf) -> decltype(src) requires (OnHeap)
            {
                return LoadImpl(src,
                                workBuf.buf1.Restore(p1.persistentBuf),
                                workBuf.buf2.Restore(p2.persistentBuf),
                                workBuf.buf3.Restore(p3.persistentBuf),
                                workBuf.buf4.Restore(p4.persistentBuf));
            }
            auto Apply(auto dst, const WorkBuffer& workBuf) -> decltype(dst) requires (OnHeap)
            {
                p1.Apply(dst, workBuf.buf1);
                p2.Apply(dst, workBuf.buf2);
                p3.Apply(dst, workBuf.buf3);
                return p4.Apply(dst, workBuf.buf4);
            }

            // Implementation when allocated on stack
            auto Load(auto src) -> decltype(src) requires (!OnHeap)
            {
                return LoadImpl(src,
                                p1.persistentBuf.Carry(),
                                p2.persistentBuf.Carry(),
                                p3.persistentBuf.Carry(),
                                p4.persistentBuf.Carry());
            }
            auto Apply(auto dst) -> decltype(dst) requires (!OnHeap)
            {
                p1.Apply(dst);
                p2.Apply(dst);
                p3.Apply(dst);
                return p4.Apply(dst);
            }

        private:
            static auto LoadImpl(auto src, auto buf1, auto buf2, auto buf3, auto buf4) -> decltype(src)
            {
                auto ReadTo = [&src](auto& it)
                {
                    *it = *src;
                    ++src;
                    ++it;
                };
                for(auto n = SubChunkSz; n-->0;)
                {
                    ReadTo(buf1);
                    ReadTo(buf2);
                    ReadTo(buf3);
                    ReadTo(buf4);
                }
                return src;
            }

        };

    } // namespace Decimation

} // namespace TRM
