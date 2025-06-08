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

#include "TS808Components.hpp"
#include "Utility.hpp"

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <ranges>
#include <utility>

using namespace std;
using namespace TRM;

// Create antiparallel diode table from 'Diode_1N4148_IVTable', and then greedily
// remove data points as long as the relative error remains below 0.5%
int main ()
{
    const decltype(Diode_1N4148_IVTable) antiParallelDiodeClippingTable = [&]{
        remove_const_t<decltype(Diode_1N4148_IVTable)> res;
        auto dst  = begin(res);
        auto rvrs = rbegin(Diode_1N4148_IVTable);
        for (const auto& m : Diode_1N4148_IVTable)
        {
            *dst = m;
            dst->y -= rvrs->y;
            ++dst;
            ++rvrs;
        }
        return res;
    }();

    auto it = begin(antiParallelDiodeClippingTable);
    while(it->x < 0.0) ++it;

    // now it --> x = 0.0

    // Algo: skip the next 1 points and check the relative error
    // If OK then check what if we skipped 2 points, etc...
    constexpr double AllowedRelativeError = 0.005; // 0.5 % error
    struct Sector { decltype(it) start, end; };
    auto Evaluate = [](const Sector& s, const double x){
        if (x <= s.start->x || s.end->x <= x) [[unlikely]] cout << " ! Evaluate(): Something went wrong !\n";

        const double deltaStart = x - s.start->x;
        const double deltaEnd   = s.end->x - x;
        const double segmentLen = s.end->x - s.start->x;

        return deltaStart > deltaEnd ? lerp(s.start->y, s.end->y,   deltaStart / segmentLen) :
                                       lerp(s.end->y,   s.start->y, deltaEnd   / segmentLen);
    };

    auto IsWithinError = [&](const Sector& s, const decltype(it) p){
        const double exactValue   = p->y;
        const double interpolated = Evaluate(s, p->x);
        const double minAccepted  = (1.0 - AllowedRelativeError) * exactValue;
        const double maxAccepted  = (1.0 + AllowedRelativeError) * exactValue;
        return (minAccepted <= interpolated && interpolated <= maxAccepted);
    };

    cout << setprecision(5);

    Sector currSector {it, it+2};
    auto CheckIntermediatePoints = [&]() -> bool {
        for(auto i = (currSector.start + 1); i < currSector.end; ++i)
            if (!IsWithinError(currSector, i))
                return false;
        return true;
    };
    do
    {
        while(currSector.end < end(antiParallelDiodeClippingTable) && CheckIntermediatePoints() ) ++currSector.end;
        --currSector.end;

        cout << "Shrank a sector: " << currSector.start->x << " - " << currSector.end->x << "\t([" << distance(it, currSector.start) << "] - [" << distance(it, currSector.end) << "])\n";

        currSector.start = currSector.end;
        currSector.end += 2;
    } while (currSector.end < end(antiParallelDiodeClippingTable));
}
