//------------------------------------------------------------------------
// Copyright (C) 2025 Ték Róbert Máté <eppenpontaz@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------

#pragma once

#include "IIR.hpp"

#include <array>

namespace TRM
{

    struct IIR_Data {
        double p1, p2;
        double z1, z2;
        double gain;
        double param;
    };

    inline constexpr  std::array<IIR_Data, 21> Tone_IIR_Table = {{
        { 0.98857373354699329, 0.92177098135764268, 0.95161148515312033, -0.66551429414773422, 0.01064604793941966, 0.00 },
        { 0.98911982328589310, 0.92176951088013859, 0.96569769939760097, -0.66549544579917197, 0.01400890689799148, 0.02 },
        { 0.98976573203471485, 0.93339961956103057, 0.97487452129424845, -0.66540738615333850, 0.01515921840897236, 0.04 },
        { 0.99086111097075835, 0.94900732416517652, 0.98256168412775791, -0.66538337906086154, 0.01482661457106951, 0.06 },
        { 0.99168821989988676, 0.95580845882419951, 0.98627382014919829, -0.66540654967785162, 0.01478022546502335, 0.08 },
        { 0.99225091240762608, 0.95939174502075630, 0.98828402879109267, -0.66542167903085447, 0.01483172133280391, 0.1 },
        { 0.99262870638346490, 0.96315401546396051, 0.99005284849952091, -0.66549565670846955, 0.01494562361619176, 0.14 },
        { 0.99262160179977388, 0.96594915621440847, 0.99091870656604630, -0.66538791299298838, 0.01508876294346026, 0.2 },
        { 0.99190348627561120, 0.96858251489248492, 0.99089053550013440, -0.66536874606921281, 0.01520141151578191, 0.3 },
        { 0.98639558687674012, 0.96948956383839535, 0.98526039491754036, -0.66547389475450269, 0.01543283026945661, 0.4 },
        { 0.98555011143792770, 0.97152258665890290, 0.98536267640148267, -0.66549135997978337, 0.01554070934664679, 0.5 },
        { 0.98554324146512728, 0.97328018238134184, 0.98637399254312452, -0.66548490354050083, 0.01580915260725866, 0.6 },
        { 0.98563446878414307, 0.97465729954656333, 0.98741649169790768, -0.66549428404330990, 0.01626724385942613, 0.7 },
        { 0.98871032353619370, 0.97465423788322836, 0.99072033049274433, -0.66551479990473938, 0.01723938628197703, 0.8 },
        { 0.98860360257381241, 0.97452959722175669, 0.99127613610833609, -0.66552639697540250, 0.01845754199873891, 0.86 },
        { 0.98779519789153536, 0.97360932417946300, 0.99122724670801432, -0.66551769417183781, 0.02020369309289578, 0.9 },
        { 0.98554870766218627, 0.97362171152448784, 0.99025679178744397, -0.66548769347764958, 0.02163496872372875, 0.92 },
        { 0.98217259524870570, 0.97349696213379056, 0.98902740328817829, -0.66532595991591548, 0.02382726908486084, 0.94 },
        { 0.97618737570398473, 0.97349459010231554, 0.98734971247887438, -0.66530624571685071, 0.02765539173484386, 0.96 },
        { 0.97328656379169509, 0.96251594464816803, 0.98468906907221854, -0.66530245198237881, 0.03633765542418637, 0.98 },
        { 0.97329747275843070, 0.90078638748869400, 0.98030379788969091, -0.66532288702220455, 0.07494915386971208, 1.0 }
    }};

    constexpr IIR_3_2 getIIRCoefficients(double param) {
        param = std::clamp(param, 0.0, 1.0);

        constexpr auto ToCoefficients = [](const IIR_Data& data) constexpr -> IIR_3_2
        {
            return IIR_3_2{
                .b0 = data.gain,
                .b1 = -data.gain * (data.z1 + data.z2),
                .b2 = data.gain * data.z1 * data.z2,
                .a1 = -(data.p1 + data.p2),
                .a2 = data.p1 * data.p2
            };
        };

        if (param == 0.0) {
            return ToCoefficients(Tone_IIR_Table[0]);
        }

        if (param == 1.0) {
            return ToCoefficients(Tone_IIR_Table.back());
        }

        auto it = std::lower_bound(Tone_IIR_Table.begin(), Tone_IIR_Table.end(), param,
            [](const IIR_Data& data, double value) { return data.param < value; });
    
        if (it == Tone_IIR_Table.begin()) ++it;
    
        const auto& lo = *(it - 1);
        const auto& hi = *it;
        double t = (param - lo.param) / (hi.param - lo.param);
    
        double p1   = std::lerp(lo.p1,   hi.p1,   t);
        double p2   = std::lerp(lo.p2,   hi.p2,   t);
        double z1   = std::lerp(lo.z1,   hi.z1,   t);
        double z2   = std::lerp(lo.z2,   hi.z2,   t);
        double gain = std::lerp(lo.gain, hi.gain, t);
    
        return ToCoefficients({p1, p2, z1, z2, gain, param});
    }

} // namespace TRM
