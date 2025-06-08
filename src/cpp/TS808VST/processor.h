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
//----------------------------------------------------------------------

#pragma once

#include "pids.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "public.sdk/source/vst/utility/audiobuffers.h"
#include "public.sdk/source/vst/utility/rttransfer.h"
#include "public.sdk/source/vst/utility/sampleaccurate.h"
#include "public.sdk/source/vst/utility/sampleaccurate.h"

#include "TS808Components.hpp"
#include "Decimation.hpp"
#include "IIR.hpp"
#include "FIR.hpp"
#include "Tone_IIR_Table.hpp"

#include <cmath>
#include <array>
#include <algorithm>
#include <numeric>

namespace TRM {

//------------------------------------------------------------------------
//  TS808ClipperProcessor
//------------------------------------------------------------------------
class TS808ClipperProcessor : public Steinberg::Vst::AudioEffect
{
public:
	struct StateModel
	{
		double gain  = 0.0;
		double tone  = 0.5;
		double level = 0.5;
	};
	using RTTransfer = Steinberg::Vst::RTTransferT<StateModel>;

public:
	TS808ClipperProcessor ();
	~TS808ClipperProcessor () SMTG_OVERRIDE;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/) 
	{ 
		return (Steinberg::Vst::IAudioProcessor*)new TS808ClipperProcessor; 
	}

	//------------------------------------------------------------------------
	// AudioEffect overrides:
	//------------------------------------------------------------------------
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
		
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

//------------------------------------------------------------------------
private:
	void handleParameterChanges (Steinberg::Vst::IParameterChanges* changes);

	template <Steinberg::Vst::SymbolicSampleSizes SampleSize>
	void process (Steinberg::Vst::ProcessData& data);

	Steinberg::Vst::SampleAccurate::Parameter gainParameter  {ParameterID::Gain,  0.};
	Steinberg::Vst::SampleAccurate::Parameter toneParameter  {ParameterID::Tone,  0.};
	Steinberg::Vst::SampleAccurate::Parameter levelParameter {ParameterID::Level, 0.};
	RTTransfer stateTransfer;

	double prevClippingStageOut = 0.;

	std::array<double, 6> prev_in  = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	std::array<double, 3> prev_din = {0.0, 0.0, 0.0};

	std::array<double, TRM::Decimation::D4x_Poly_Taps-1> prev_d4x_a{},
														 prev_d4x_b{},
														 prev_d4x_c{},
														 prev_d4x_d{};

	TRM::IIR_HighPass clippingStageHP {-0.976696930369159, 0.988348465184579};
	TRM::IIR_3_2_Executor toneCircuit {TRM::getIIRCoefficients(0.5)};
	double lastToneParameter = 0.5;
};

#define CLIP
#define TONE

//------------------------------------------------------------------------

struct H48  { inline static constexpr double value = 1. / 48'000.; };
struct H192 { inline static constexpr double value = 1. / 192'000.; };

template <Steinberg::Vst::SymbolicSampleSizes SampleSize>
void TS808ClipperProcessor::process (Steinberg::Vst::ProcessData& data)
{
	using namespace std;
	using namespace TRM;

	const Steinberg::Vst::ParamValue gain  = gainParameter.advance (data.numSamples);
	const Steinberg::Vst::ParamValue tone  = toneParameter.advance (data.numSamples);
	const Steinberg::Vst::ParamValue level = levelParameter.advance (data.numSamples);

	if (tone != lastToneParameter)
	{
		toneCircuit.UpdateCoefs(getIIRCoefficients(tone));
		lastToneParameter = tone;
	}

	const bool isSupportedSampleRate = data.processContext && data.processContext->sampleRate == 48'000.;
	if (!isSupportedSampleRate) return;

	constexpr Steinberg::int32 Left  = 0;
	constexpr Steinberg::int32 Right = 1;

	constexpr double FullScaleSampleVoltage = 3.88;

	const bool supportedBufferSize = data.numSamples == 32 || data.numSamples == 64 || data.numSamples == 128 ||
									 data.numSamples == 256 || data.numSamples == 512 || data.numSamples == 1024;
	if (!supportedBufferSize) return;

	auto processImpl = [&]<size_t BufferSize>() -> void
	{
		// 48kHz input samples
		const auto inBuf = [&]() -> array<double, 6 + BufferSize>
		{
			array<double, 6 + BufferSize> inBuf;
			copy_n(prev_in.begin(), 6, inBuf.begin());
			const auto& in = getChannelBuffers<SampleSize> (data.inputs[0]);
			copy_n(in[Left], BufferSize, inBuf.begin() + 6);
			copy_n(inBuf.end()-6, 6, prev_in.begin());
			return inBuf;
		}();

		// 48kHz input derivatives
		const auto dinBuf = [&]() -> array<double, 3 + BufferSize>
		{
			constexpr double r = 1. / (H48::value * 60);
			constexpr auto diff_kernel = FIR(-1.*r, 9.*r, -45.*r, 0.0, 45.*r, -9.*r, 1.*r);
			array<double, 3 + BufferSize> dinBuf;
			copy_n(prev_din.begin(), 3, dinBuf.begin());
			diff_kernel(inBuf.begin(), BufferSize, dinBuf.begin() + 3);
			copy_n(dinBuf.rbegin(), 3, prev_din.rbegin());
			return dinBuf;
		}();

		struct SampleAndDerivative
		{
			double sample = 0.0;
			double derivative = 0.0;
		};

		// 192kHz upsampled input + derivatives
		const auto inUp = [&]() -> array<SampleAndDerivative, 4 * BufferSize>
		{
			constexpr double h = H48::value;

			array<SampleAndDerivative, 4 * BufferSize> inUp{};

			for (int i = 0; i < BufferSize; ++i)
			{
				auto dst = [cur = i * 4, &inUp](int r) -> SampleAndDerivative& { return inUp[cur + r]; };

				// Not the nicest way to do this, but it works
				constexpr auto a  = Basic_FIR(3283., 165375., 25725., 2225.);
				constexpr auto b  = Basic_FIR(13., 243., 243., 13.);
				constexpr auto c  = Basic_FIR(2225., 25725., 165375., 3283.);
				constexpr auto da = Basic_FIR(735.*h, 33075.*h, -11025.*h, -525.*h);
				constexpr auto db = Basic_FIR(3.*h, 81.*h, -81.*h, -3.*h);
				constexpr auto dc = Basic_FIR(525.*h, 11025.*h, -33075.*h, -735.*h);

				constexpr auto d_a  = Basic_FIR(11935., -174825., 152145., 10745.);
				constexpr auto d_b  = Basic_FIR(-5., -405., 405., 5.);
				constexpr auto d_c  = Basic_FIR(-11935., -174825., 152145., -10745.);
				constexpr auto d_da = Basic_FIR(2751.*h, 44415.*h, -58905.*h, -2505.*h);
				constexpr auto d_db = Basic_FIR(-1.*h, -81.*h, -81.*h, -1.*h);
				constexpr auto d_dc = Basic_FIR(-2751.*h, -44415.*h, 58905.*h, 2505.*h);

				dst(0).sample = a(begin(inBuf) + i);
				dst(1).sample = b(begin(inBuf) + i);
				dst(2).sample = c(begin(inBuf) + i);
				
				dst(0).derivative = d_a(begin(inBuf) + i);
				dst(1).derivative = d_b(begin(inBuf) + i);
				dst(2).derivative = d_c(begin(inBuf) + i);

				dst(0).sample += da(begin(dinBuf) + i);
				dst(1).sample += db(begin(dinBuf) + i);
				dst(2).sample += dc(begin(dinBuf) + i);

				dst(0).derivative += d_da(begin(dinBuf) + i);
				dst(1).derivative += d_db(begin(dinBuf) + i);
				dst(2).derivative += d_dc(begin(dinBuf) + i);

				dst(0).sample /= 196608.;
				dst(1).sample /= 512.;
				dst(2).sample /= 196608.;

				dst(0).derivative /= 147456. * h;
				dst(1).derivative /= 256. * h;
				dst(2).derivative /= 147456. * h;

				dst(3).sample = inBuf[i+2];
				dst(3).derivative = dinBuf[i+2];
			}

			return inUp;
		}();
		
		constexpr double h = H192::value;

		auto CalcClipping = [A = (Cf/h) + (1./(Rf + gain * Rd))](const double _C) -> double {
			const double C = abs(_C);
			auto Pred = [&](const double, const Measurement& m){ return C < fma(m.x, A, m.y); };
			
			const auto Upper = upper_bound(begin(Diode_1N4148_AntiPar_IVTable_SparsePoint5),
										   end(Diode_1N4148_AntiPar_IVTable_SparsePoint5),
										   0.0, // dummy value
										   Pred);
			const auto Lower = Upper-1;

			if (Upper == end(Diode_1N4148_AntiPar_IVTable_SparsePoint5)) [[unlikely]]
			{
				return 0.0;
			}
			if (Upper == begin(Diode_1N4148_AntiPar_IVTable_SparsePoint5)) [[unlikely]]
			{
				return 0.0;
			}

			const Measurement& upper = *Upper;
			const Measurement& lower = *Lower;

			const double distLower = C - fma(lower.x, A, lower.y);
			const double distUpper = fma(upper.x, A, upper.y) - C;

			const double range = fma(upper.x, A, upper.y) - fma(lower.x, A, lower.y);

			return copysign((distLower < distUpper ? (lerp(upper.x, lower.x, distUpper/range)) :
													 (lerp(lower.x, upper.x, distLower/range))),
							_C);
		};

		// Prepare for decimation
		array<double, BufferSize + Decimation::D4x_Poly_Taps-1> poly_a{}, poly_b{}, poly_c{}, poly_d{};
		copy_n(prev_d4x_a.begin(), Decimation::D4x_Poly_Taps-1, poly_a.begin());
		copy_n(prev_d4x_b.begin(), Decimation::D4x_Poly_Taps-1, poly_b.begin());
		copy_n(prev_d4x_c.begin(), Decimation::D4x_Poly_Taps-1, poly_c.begin());
		copy_n(prev_d4x_d.begin(), Decimation::D4x_Poly_Taps-1, poly_d.begin());

		const array<decltype(&poly_a), 4> polyBufs = {&poly_a, &poly_b, &poly_c, &poly_d};


		auto ClippingStage_DoOne = [&](const double in, const double din) -> double {
#ifdef CLIP
			const double Y     = clippingStageHP(in);
			const double C     = fma(1./Rg, Y, fma(-(Cf/h), in, fma(Cf/h, prevClippingStageOut, fma(Cf, din, 0.0))));
			const double delta = CalcClipping(C);

			const double clippingStageOut = in + delta;
			prevClippingStageOut = clippingStageOut;

			return clippingStageOut;
#else
			return in;
#endif
		};

		for (size_t i = 0; i < inUp.size(); ++i)
		{
			const double in  = inUp[i].sample;
			const double din = inUp[i].derivative;
#ifdef TONE
			const double clipOut = ClippingStage_DoOne(in, din);
			const double toneOut = toneCircuit(clipOut);
#else
			const double toneOut  = ClippingStage_DoOne(in, din);
#endif
			polyBufs[i % 4]->at((i / 4) + (Decimation::D4x_Poly_Taps-1)) = toneOut;
		}

		auto copyToOutput = [&, nextSampleIdx = 0u, out = getChannelBuffers<SampleSize> (data.outputs[0])] (const double _val) mutable
		{
			const double val = (_val / FullScaleSampleVoltage) * 2. * level;
			out[Left][nextSampleIdx] = val;
			if (data.outputs[0].numChannels > 1) [[likely]] out[Right][nextSampleIdx] = val;
			++nextSampleIdx;
		};

		// Decimate
		Decimation::D4x_Poly_1 d1;
		Decimation::D4x_Poly_2 d2;
		Decimation::D4x_Poly_3 d3;
		Decimation::D4x_Poly_4 d4;

		auto it1 = begin(poly_a) + Decimation::D4x_Poly_Taps-1,
			it2 = begin(poly_b) + Decimation::D4x_Poly_Taps-1,
			it3 = begin(poly_c) + Decimation::D4x_Poly_Taps-1,
			it4 = begin(poly_d) + Decimation::D4x_Poly_Taps-1;
		for (size_t i = 0; i < BufferSize; ++i)
		{
			copyToOutput(d1.Apply(it1) + d2.Apply(it2) + d3.Apply(it3) + d4.Apply(it4));
			++it1;
			++it2;
			++it3;
			++it4;
		}

		copy_n(poly_a.rbegin(), Decimation::D4x_Poly_Taps-1, prev_d4x_a.rbegin());
		copy_n(poly_b.rbegin(), Decimation::D4x_Poly_Taps-1, prev_d4x_b.rbegin());
		copy_n(poly_c.rbegin(), Decimation::D4x_Poly_Taps-1, prev_d4x_c.rbegin());
		copy_n(poly_d.rbegin(), Decimation::D4x_Poly_Taps-1, prev_d4x_d.rbegin());
	};

	switch(data.numSamples)
	{
		case 32: processImpl.template operator()<32>(); break;
		case 64: processImpl.template operator()<64>(); break;
		case 128: processImpl.template operator()<128>(); break;
		case 256: processImpl.template operator()<256>(); break;
		case 512: processImpl.template operator()<512>(); break;
		case 1024: processImpl.template operator()<1024>(); break;
		default: break;
	}
}

} // namespace TRM
