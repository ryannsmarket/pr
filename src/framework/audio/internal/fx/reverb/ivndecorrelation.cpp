/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ivndecorrelation.h"

namespace muse::audio::fx {
const float ivn15_ms[64][15] = {
    { 0.f, 1.00002f, 2.98952f, 5.04659f, 6.20219f, 9.14794f, 10.2062f, 12.5143f, 14.8911f, 16.2289f, 18.7191f, 20.5436f,
      22.4762f, 24.7153f, 26.6872f },
    { 0.f, 0.810028f, 2.41639f, 5.14998f, 6.20002f, 8.20014f, 10.5505f, 13.1491f, 14.9046f, 16.6035f, 18.5015f, 20.5228f,
      22.7625f, 24.6428f, 26.5613f },
    { 0.f, 1.05302f, 3.1473f, 4.3833f, 6.2036f, 9.12153f, 10.4559f, 12.4605f, 14.5945f, 16.8876f, 18.3166f, 20.873f,
      22.8401f, 24.498f, 26.5397f },
    { 0.f, 0.975441f, 2.92966f, 4.20003f, 6.83538f, 8.78924f, 11.1494f, 12.2061f, 14.6477f, 16.3013f, 18.9768f, 20.5048f,
      22.7928f, 24.4115f, 26.819f },
    { 0.f, 0.229679f, 3.1394f, 5.11003f, 6.39624f, 9.06704f, 11.0673f, 12.4352f, 15.1104f, 17.0765f, 18.8331f, 20.3125f,
      22.8144f, 24.8189f, 26.7133f },
    { 0.f, 1.05281f, 3.1497f, 4.20065f, 6.29897f, 8.45245f, 10.5013f, 12.204f, 14.5558f, 16.7426f, 18.792f, 20.9325f,
      22.578f, 24.7527f, 26.7318f },
    { 0.f, 1.1492f, 2.20018f, 5.1434f, 6.20741f, 8.30464f, 10.6883f, 12.5175f, 14.4627f, 16.2592f, 18.3171f, 20.3529f,
      22.8754f, 24.4725f, 26.4228f },
    { 0.f, 0.780095f, 3.11914f, 4.67848f, 6.2392f, 8.57778f, 10.2008f, 12.4784f, 14.8188f, 17.1489f, 18.7201f, 20.5406f,
      22.2091f, 24.8899f, 26.488f },
    { 0.f, 0.235683f, 2.3376f, 4.76667f, 6.55081f, 9.12697f, 11.15f, 13.1452f, 14.6244f, 16.2561f, 18.8917f, 20.2612f,
      22.8464f, 24.3841f, 26.8089f },
    { 0.f, 0.281086f, 2.28634f, 5.12724f, 6.65762f, 8.20726f, 10.2179f, 12.4535f, 15.0886f, 16.9984f, 18.8984f, 20.409f,
      22.8892f, 24.7156f, 26.7605f },
    { 0.f, 0.631558f, 3.14999f, 5.0391f, 6.60641f, 8.27989f, 11.1035f, 12.2716f, 14.2953f, 16.4279f, 18.4759f, 20.8261f,
      22.6998f, 24.8703f, 26.7151f },
    { 0.f, 0.341454f, 2.37623f, 5.0923f, 6.44479f, 8.66501f, 11.0351f, 12.8993f, 14.5953f, 16.2896f, 18.8443f, 20.7027f,
      22.8461f, 24.8805f, 26.6626f },
    { 0.f, 0.878885f, 2.6382f, 4.39631f, 7.03267f, 8.24679f, 10.2085f, 12.3057f, 14.9423f, 16.365f, 18.4575f, 21.0504f,
      22.7457f, 25.0469f, 26.8214f },
    { 0.f, 0.98651f, 2.9603f, 5.1494f, 6.95216f, 8.88247f, 10.8693f, 12.9155f, 14.9794f, 16.3683f, 18.3839f, 20.6685f,
      22.88f, 24.5897f, 26.6185f },
    { 0.f, 1.15f, 2.30641f, 5.15f, 6.2f, 8.2f, 11.15f, 13.15f, 14.2f, 16.6148f, 18.4171f, 20.6068f, 22.6102f, 24.5947f,
      26.7207f },
    { 0.f, 0.20035f, 3.00635f, 5.04855f, 6.22913f, 9.12152f, 10.3733f, 12.2255f, 14.3966f, 16.2964f, 18.8406f, 20.5506f,
      22.6643f, 24.5981f, 26.9652f },
    { 0.f, 1.11447f, 2.90834f, 4.9253f, 6.9358f, 8.4563f, 11.0815f, 12.5352f, 15.0788f, 16.9875f, 18.7724f, 20.7748f,
      22.5529f, 24.6221f, 26.7053f },
    { 0.f, 0.73208f, 2.92831f, 4.39243f, 6.58897f, 9.15f, 10.2483f, 12.2f, 14.2f, 16.835f, 19.0317f, 21.0921f, 22.3522f,
      24.8411f, 26.8447f },
    { 0.f, 0.247433f, 2.94414f, 5.13949f, 7.0678f, 8.58f, 10.5272f, 12.9167f, 14.6083f, 16.9106f, 18.4434f, 20.3069f,
      23.0108f, 24.7354f, 26.7955f },
    { 0.f, 1.14951f, 2.40137f, 4.80681f, 6.50949f, 8.85026f, 10.4642f, 13.1288f, 14.566f, 16.4926f, 18.8315f, 20.9966f,
      22.8755f, 24.3862f, 26.5519f },
    { 0.f, 0.737156f, 2.20096f, 5.14693f, 7.14516f, 9.14803f, 10.207f, 12.4366f, 14.86f, 16.5692f, 18.2981f, 20.6123f,
      22.5277f, 24.6467f, 26.7916f },
    { 0.f, 0.897849f, 2.67755f, 5.15f, 6.2f, 8.2f, 11.0982f, 12.4069f, 14.492f, 16.5145f, 18.7286f, 20.7659f, 22.7585f,
      24.5679f, 26.7697f },
    { 0.f, 1.06545f, 3.11018f, 5.03171f, 6.20101f, 8.34143f, 11.15f, 12.816f, 15.0186f, 16.6348f, 18.3162f, 20.7981f,
      22.4446f, 24.6683f, 26.4784f },
    { 0.f, 1.14983f, 2.74773f, 4.24768f, 6.99469f, 8.20597f, 10.7909f, 12.9912f, 15.1396f, 16.2308f, 19.0434f, 20.7277f,
      22.3288f, 24.4841f, 26.6194f },
    { 0.f, 0.887075f, 2.66336f, 5.15f, 6.21318f, 8.2f, 10.6509f, 13.0484f, 14.203f, 16.2439f, 18.8166f, 20.2053f,
      22.6291f, 24.6733f, 26.5517f },
    { 0.f, 0.329182f, 3.14305f, 4.68969f, 6.93142f, 8.25874f, 10.6706f, 12.9504f, 14.2024f, 16.3262f, 19.0409f, 20.4097f,
      22.5662f, 24.7643f, 27.1128f },
    { 0.f, 0.252239f, 2.40505f, 4.42967f, 6.33161f, 8.23752f, 11.1419f, 13.0307f, 14.4327f, 16.9673f, 18.4921f, 20.684f,
      22.4379f, 24.6227f, 26.6194f },
    { 0.f, 1.15f, 3.03942f, 4.20001f, 6.20006f, 8.3923f, 10.758f, 13.0469f, 14.8762f, 16.2001f, 18.3235f, 20.3095f,
      22.5246f, 24.7724f, 26.4549f },
    { 0.f, 0.735739f, 2.20788f, 5.15f, 6.94263f, 8.83299f, 10.4252f, 12.5982f, 14.7494f, 16.5319f, 18.7606f, 20.5978f,
      22.6145f, 24.6843f, 26.7563f },
    { 0.f, 0.814268f, 2.98881f, 4.34749f, 6.24744f, 8.28924f, 10.8451f, 12.3765f, 15.0329f, 16.5383f, 18.7613f, 20.5051f,
      22.5063f, 24.6477f, 26.5447f },
    { 0.f, 0.928743f, 3.0704f, 4.28232f, 6.22618f, 9.12834f, 10.9839f, 12.841f, 14.3362f, 16.4498f, 18.8418f, 20.4851f,
      22.7417f, 24.5158f, 26.7188f },
    { 0.f, 0.776219f, 2.32965f, 5.14985f, 6.20794f, 8.54002f, 10.6216f, 12.8779f, 14.8988f, 16.5901f, 18.2373f, 20.903f,
      22.6872f, 24.7537f, 26.7091f },
    { 0.f, 0.254183f, 2.29096f, 5.09347f, 6.87547f, 8.41314f, 10.4427f, 12.2236f, 14.8833f, 16.556f, 18.7474f, 20.4443f,
      22.6602f, 24.5939f, 26.6921f },
    { 0.f, 0.858367f, 2.57582f, 5.14939f, 6.86697f, 8.58493f, 10.7977f, 12.5737f, 14.4677f, 16.4223f, 18.4396f, 20.48f,
      22.6183f, 24.7142f, 26.7064f },
    { 0.f, 1.05208f, 2.2f, 5.15f, 6.39303f, 9.14928f, 10.3904f, 12.3164f, 15.0406f, 16.5872f, 18.297f, 20.7423f,
      22.5089f, 24.4153f, 26.5904f },
    { 0.f, 1.14993f, 2.4285f, 4.73003f, 6.77566f, 8.20064f, 10.7446f, 12.2487f, 14.2287f, 16.2887f, 18.3035f, 20.3522f,
      22.7612f, 24.326f, 26.444f },
    { 0.f, 0.200149f, 2.41807f, 5.03005f, 6.23165f, 8.24696f, 11.0693f, 12.5995f, 15.1248f, 16.2555f, 18.4566f, 20.4711f,
      22.8524f, 24.6454f, 26.9764f },
    { 0.f, 0.878229f, 2.20001f, 4.32425f, 6.96212f, 9.14998f, 10.4109f, 12.6057f, 14.6556f, 16.353f, 19.0803f, 20.4443f,
      22.4689f, 24.6712f, 26.5668f },
    { 0.f, 0.339864f, 2.96109f, 5.08273f, 6.21499f, 9.04051f, 10.4983f, 13.1215f, 14.7914f, 16.5572f, 18.4926f, 20.5187f,
      22.7077f, 24.7804f, 26.7209f },
    { 0.f, 0.493869f, 2.72979f, 4.97502f, 6.71751f, 8.201f, 10.8879f, 13.1439f, 14.8045f, 17.1117f, 18.9752f, 20.53f,
      22.6567f, 24.9557f, 26.6665f },
    { 0.f, 1.14063f, 3.15f, 4.71711f, 6.57034f, 8.85012f, 10.5662f, 12.8276f, 14.2769f, 16.2f, 18.5772f, 20.2458f,
      22.4405f, 24.8531f, 26.5461f },
    { 0.f, 0.788103f, 3.1457f, 4.71313f, 6.26781f, 8.25492f, 10.2694f, 12.8582f, 14.6711f, 16.5733f, 18.3749f, 20.9018f,
      23.0092f, 24.7528f, 26.7861f },
    { 0.f, 0.723978f, 2.21569f, 4.43158f, 6.59224f, 8.76791f, 10.2564f, 12.3221f, 14.4533f, 17.1476f, 18.3145f, 20.6085f,
      22.5892f, 24.6317f, 26.6417f },
    { 0.f, 1.14989f, 2.35249f, 4.70764f, 6.20044f, 9.14918f, 10.617f, 12.968f, 14.5056f, 16.9078f, 18.2561f, 20.6227f,
      22.3198f, 25.0587f, 26.5224f },
    { 0.f, 0.354579f, 3.10745f, 5.12772f, 6.29712f, 8.20162f, 10.4895f, 12.4738f, 14.7234f, 17.1053f, 18.4881f, 20.9051f,
      23.028f, 24.3537f, 27.0713f },
    { 0.f, 0.829745f, 2.85577f, 4.51563f, 6.39985f, 8.20104f, 11.0567f, 12.7158f, 15.15f, 16.7786f, 18.89f, 20.7671f,
      22.7128f, 24.8118f, 26.662f },
    { 0.f, 0.244296f, 2.20191f, 4.37372f, 6.25908f, 8.44312f, 10.3813f, 12.2319f, 15.1376f, 16.9551f, 18.2836f, 20.8263f,
      23.084f, 24.5968f, 26.8046f },
    { 0.f, 1.11967f, 2.90729f, 5.14223f, 6.31324f, 8.21586f, 10.9414f, 12.2434f, 15.1157f, 16.961f, 19.0057f, 20.8405f,
      22.6743f, 24.6991f, 26.7495f },
    { 0.f, 0.816146f, 2.44421f, 4.82774f, 6.46728f, 8.90589f, 10.5445f, 12.2462f, 15.0705f, 16.8237f, 19.0195f, 20.5662f,
      22.4097f, 24.6815f, 26.7913f },
    { 0.f, 1.04986f, 2.78092f, 5.13005f, 6.61293f, 9.14641f, 11.1319f, 12.2007f, 14.9608f, 16.44f, 18.2088f, 20.629f,
      22.8685f, 24.7039f, 26.6751f },
    { 0.f, 1.05486f, 3.14933f, 4.20247f, 6.28227f, 9.09846f, 10.5751f, 13.0262f, 14.7361f, 16.4724f, 18.4102f, 20.6455f,
      22.6666f, 24.8183f, 26.6407f },
    { 0.f, 1.15f, 3.03883f, 4.20001f, 6.30178f, 8.59185f, 10.4333f, 12.265f, 14.6302f, 16.9103f, 18.3478f, 20.6118f,
      22.9932f, 24.9828f, 26.5817f },
    { 0.f, 0.206497f, 3.08294f, 5.13081f, 6.20098f, 8.72299f, 10.4665f, 12.8347f, 14.2501f, 17.1005f, 19.1331f, 20.3476f,
      23.15f, 24.4755f, 26.6795f },
    { 0.f, 0.515236f, 2.24209f, 4.66235f, 6.57367f, 8.27057f, 11.058f, 12.9761f, 14.8758f, 16.4472f, 18.6901f, 20.8905f,
      22.7937f, 24.7041f, 26.8397f },
    { 0.f, 1.15f, 2.3512f, 4.70339f, 7.05511f, 8.25544f, 10.5055f, 12.305f, 14.6124f, 17.0018f, 18.709f, 21.0043f,
      22.4671f, 24.7171f, 26.866f },
    { 0.f, 0.858526f, 2.57616f, 5.15f, 6.86946f, 8.2746f, 10.2997f, 12.6417f, 14.7448f, 16.766f, 18.5823f, 20.6424f,
      22.6113f, 24.7255f, 26.6109f },
    { 0.f, 0.398615f, 3.14807f, 4.70253f, 7.05595f, 8.21035f, 10.6059f, 12.5915f, 14.2118f, 16.8584f, 18.8715f, 20.6525f,
      22.5616f, 24.7962f, 26.8756f },
    { 0.f, 0.288521f, 2.20345f, 4.58202f, 6.63005f, 9.00857f, 10.806f, 13.1381f, 14.8454f, 16.5744f, 18.4642f, 20.6844f,
      22.4941f, 24.9242f, 26.5221f },
    { 0.f, 1.02189f, 3.06676f, 5.11123f, 7.15f, 8.20001f, 10.5711f, 13.037f, 14.8231f, 16.4164f, 18.4818f, 20.8398f,
      22.8049f, 24.7879f, 26.7703f },
    { 0.f, 0.525942f, 3.14999f, 4.20002f, 6.28528f, 8.38723f, 10.488f, 12.2018f, 15.1494f, 16.3425f, 18.28f, 20.3997f,
      22.7999f, 24.772f, 26.5435f },
    { 0.f, 0.367796f, 2.2f, 5.13241f, 6.44021f, 8.96875f, 10.4383f, 12.6399f, 15.1366f, 16.2986f, 18.3354f, 20.8685f,
      22.8107f, 24.5728f, 26.7796f },
    { 0.f, 1.05027f, 2.66155f, 4.26501f, 6.96985f, 9.08061f, 10.7461f, 13.1225f, 14.9739f, 16.8434f, 18.8666f, 20.6497f,
      22.375f, 24.6004f, 26.6454f },
    { 0.f, 0.718007f, 3.15f, 4.58611f, 6.74017f, 8.45418f, 10.6537f, 12.324f, 14.2004f, 16.463f, 19.069f, 20.4972f,
      22.656f, 24.8144f, 26.5636f },
    { 0.f, 0.570525f, 2.84623f, 4.32042f, 6.8299f, 8.30551f, 11.0942f, 13.0921f, 14.4513f, 17.0262f, 18.5293f, 20.7121f,
      22.7164f, 24.8155f, 26.7556f },
};

const float ivn15_gn[64][15] = {
    { 0.488228f, -0.759648f, -0.392101f, -0.157755f, 0.0582463f, 0.0330062f, -0.0199799f, -0.0141977f, -0.00851141f,
      0.0228155f, 0.0137003f, 0.00774968f, -0.00140646f, 0.0030142f, 0.00186481f },
    { 0.500896f, -0.714572f, -0.464396f, 0.0822084f, -0.115292f, -0.0371877f, -0.0214421f, -0.014771f, -0.00847825f,
      0.0205999f, 0.0145646f, -0.00205345f, -0.00140451f, -0.000802845f, -0.000590508f },
    { 0.481621f, -0.746187f, -0.430981f, 0.0859108f, -0.124075f, 0.0377203f, -0.0223252f, -0.0124797f, -0.00848302f,
      0.0188728f, 0.0119098f, 0.00768713f, 0.00371198f, 0.00220182f, 0.00144204f },
    { 0.840563f, 0.345785f, 0.210721f, -0.156296f, 0.229485f, 0.215263f, -0.0345217f, -0.0222601f, 0.0632235f,
      -0.00970159f, -0.00594677f, 0.0139044f, -0.0024433f, 0.00625326f, -0.000869089f },
    { 0.771724f, 0.299215f, 0.218118f, -0.339465f, 0.356821f, 0.120594f, 0.0393101f, 0.085121f, -0.0143429f, 0.033241f,
      -0.00679988f, -0.00415033f, 0.00770403f, -0.00214343f, 0.00276909f },
    { 0.618536f, 0.393122f, -0.499069f, 0.42043f, -0.106538f, -0.129988f, 0.0605303f, -0.0690304f, 0.0117061f,
      0.00685539f, 0.00460959f, -0.0105422f, -0.00531497f, -0.00364958f, 0.00165073f },
    { 0.853182f, -0.398519f, 0.256977f, 0.139007f, -0.143889f, 0.0643693f, -0.0422677f, -0.0222511f, -0.013756f,
      -0.0186022f, 0.00672567f, -0.0126697f, 0.00277051f, -0.00476816f, 0.00172904f },
    { 0.855652f, 0.367076f, -0.216952f, -0.137182f, -0.117299f, 0.194637f, 0.0382871f, -0.0920622f, 0.0600167f,
      -0.0358033f, -0.0236377f, 0.0118278f, -0.00402026f, -0.00571331f, -0.00344339f },
    { 0.743001f, 0.322996f, -0.213965f, 0.366453f, -0.331155f, 0.220556f, 0.0338906f, 0.0201798f, 0.0503542f, 0.022871f,
      -0.020499f, 0.0129653f, -0.0018219f, 0.0052378f, 0.00345361f },
    { 0.582382f, -0.375025f, 0.627187f, -0.325456f, -0.0671237f, 0.0426661f, -0.0984655f, -0.0542353f, -0.0374257f,
      -0.0219372f, 0.00515769f, 0.00345863f, 0.00305337f, -0.0027411f, -0.00159225f },
    { 0.54091f, -0.373188f, 0.629465f, 0.336889f, -0.18252f, -0.146406f, 0.0215721f, 0.0550058f, 0.00853349f,
      0.00613337f, -0.00358738f, -0.00225284f, 0.00631178f, 0.00371578f, 0.00237337f },
    { 0.659525f, 0.486353f, -0.461515f, 0.304498f, 0.0686232f, -0.0877786f, -0.0310991f, 0.0772741f, -0.0464171f,
      0.0301639f, -0.013522f, 0.0113353f, 0.00552648f, 0.00327946f, 0.00164289f },
    { 0.824292f, 0.358652f, 0.211632f, 0.160689f, -0.211066f, 0.224396f, 0.14955f, -0.0249121f, 0.0521567f, -0.00978597f,
      0.0239503f, -0.00355562f, -0.00221109f, -0.00188315f, -0.0019423f },
    { 0.506676f, 0.792015f, 0.280881f, -0.084693f, 0.0720473f, 0.122128f, 0.0904711f, -0.0133485f, 0.0335865f,
      -0.00537964f, 0.0141921f, 0.00885298f, -0.00163112f, 0.0035531f, 0.00197487f },
    { 0.459874f, -0.724331f, -0.482665f, 0.0737074f, -0.150693f, -0.0326949f, -0.0207389f, -0.0113727f, 0.0305576f,
      -0.00495378f, 0.0118211f, -0.00214786f, 0.0045562f, -0.000780556f, -0.000552416f },
    { 0.577765f, -0.38733f, 0.669105f, -0.104667f, -0.216316f, 0.0486387f, -0.0853221f, 0.0165106f, 0.0102314f,
      -0.0256917f, 0.0038784f, 0.00281746f, 0.00191369f, 0.00158015f, 0.00120286f },
    { 0.616376f, -0.390991f, 0.516407f, 0.319148f, -0.23965f, -0.163152f, 0.0862181f, 0.0676328f, -0.0462159f,
      0.00704887f, -0.0172764f, 0.00948805f, -0.00582832f, 0.00143244f, -0.00244954f },
    { 0.803209f, 0.415255f, -0.22785f, -0.186525f, 0.250311f, 0.080465f, -0.145873f, -0.0197622f, 0.0573358f, 0.0312084f,
      -0.0202952f, 0.0141768f, -0.00241602f, -0.00432936f, 0.00353234f },
    { 0.778449f, 0.337677f, -0.236836f, -0.382354f, -0.0921239f, -0.21638f, 0.135357f, 0.0203595f, -0.0539638f,
      0.0088075f, -0.0204232f, -0.0134497f, -0.00753484f, 0.00363119f, 0.00216261f },
    { 0.573198f, 0.226342f, -0.63915f, -0.415523f, 0.0580109f, -0.172028f, 0.027333f, -0.0611349f, -0.0351332f,
      0.00719665f, 0.0144436f, 0.00914917f, -0.00167644f, 0.00393442f, -0.000745764f },
    { 0.49701f, -0.730893f, -0.433307f, 0.0843616f, 0.0598963f, 0.122508f, -0.0218765f, 0.0558856f, 0.0305746f,
      0.0214607f, 0.0140459f, 0.00924205f, 0.00519468f, -0.00101173f, -0.000777666f },
    { 0.528865f, -0.686752f, -0.461523f, 0.0973169f, -0.142557f, -0.038591f, -0.0212247f, 0.0562235f, -0.00909261f,
      0.0233981f, -0.00369852f, 0.00926768f, 0.00601166f, 0.00324338f, -0.000550317f },
    { 0.793014f, -0.3064f, -0.30185f, 0.125571f, -0.332997f, -0.187307f, -0.149743f, 0.0209932f, 0.0128764f, -0.0326912f,
      0.00593585f, -0.0130321f, 0.00891473f, -0.00140021f, 0.0032379f },
    { 0.630904f, 0.273283f, 0.470009f, -0.440005f, 0.263802f, -0.167893f, 0.106997f, 0.0180572f, 0.0452362f, 0.0265954f,
      0.0179392f, 0.00991974f, -0.00562051f, -0.00145376f, -0.00131334f },
    { 0.753364f, 0.4801f, 0.224115f, -0.191225f, 0.324014f, -0.0482672f, -0.0310066f, 0.0739463f, -0.0141617f,
      0.0310238f, -0.00487606f, 0.0119284f, 0.008024f, -0.00119405f, 0.00356727f },
    { 0.612604f, -0.389013f, 0.628115f, -0.0986777f, -0.251682f, 0.0470187f, 0.0251554f, 0.0153501f, -0.0389534f,
      -0.0279604f, -0.0164411f, -0.00940181f, -0.00565693f, 0.00134952f, -0.00210899f },
    { 0.545037f, 0.616236f, 0.324928f, -0.386183f, -0.232962f, -0.0382217f, 0.0834511f, 0.0591978f, -0.0394233f,
      -0.0216839f, -0.0130399f, 0.00758953f, 0.00345513f, 0.00230056f, 0.00132418f },
    { 0.637151f, -0.530417f, -0.424985f, -0.286756f, 0.0726757f, -0.192034f, 0.0263336f, 0.0628234f, -0.0414783f,
      0.0294688f, 0.0172331f, 0.0107184f, 0.00734696f, -0.00113711f, -0.000661717f },
    { 0.564049f, 0.746139f, 0.312103f, 0.0940761f, -0.0549933f, -0.0419035f, 0.106644f, -0.0140322f, 0.0405376f,
      0.0269445f, -0.00358014f, 0.0104224f, -0.00140518f, 0.0041904f, 0.00249932f },
    { 0.511941f, -0.542588f, 0.518381f, 0.363804f, 0.131875f, -0.128966f, -0.0744099f, 0.0495314f, -0.0135874f,
      -0.0119796f, -0.0079538f, -0.00445623f, -0.00333468f, -0.00210928f, 0.00136541f },
    { 0.469471f, -0.574015f, 0.522613f, 0.35326f, 0.185285f, -0.119202f, 0.0217894f, -0.0466408f, 0.0288478f,
      -0.00920389f, 0.00663172f, 0.00406483f, -0.00372608f, -0.0021469f, 0.00117236f },
    { 0.538119f, 0.762919f, 0.298231f, 0.0841375f, -0.0565617f, 0.128929f, 0.0905481f, 0.0527384f, 0.0343695f,
      -0.00559123f, 0.0146525f, 0.00965877f, 0.00567894f, -0.00101961f, 0.00232615f },
    { 0.649581f, 0.511105f, -0.288332f, -0.392561f, 0.260757f, 0.0516572f, 0.0434566f, -0.0737969f, -0.0318823f,
      -0.0184215f, -0.0122778f, 0.00630214f, 0.00472535f, -0.00266295f, 0.00181419f },
    { 0.578231f, 0.726992f, 0.311426f, -0.141611f, -0.0630907f, -0.0412214f, 0.106807f, -0.0165091f, 0.0421373f,
      0.0256415f, 0.0168652f, -0.00251404f, -0.00167358f, -0.00142811f, 0.00186393f },
    { 0.474752f, 0.746f, -0.402564f, 0.169639f, -0.135784f, -0.0606579f, 0.067589f, 0.0141121f, 0.00742909f, 0.0088886f,
      -0.0131611f, -0.00547756f, -0.00505641f, 0.00302843f, -0.00190798f },
    { 0.561211f, -0.634942f, -0.421625f, -0.309972f, 0.0598744f, 0.042643f, 0.0281149f, 0.0178038f, 0.0291099f,
      0.024599f, -0.00440554f, -0.0028138f, 0.00562518f, 0.00375712f, 0.00216138f },
    { 0.583747f, -0.274133f, 0.693453f, -0.307868f, 0.0648521f, 0.0497887f, 0.0237067f, 0.0164778f, 0.0109505f,
      -0.0248934f, 0.0115902f, -0.00487845f, -0.00286929f, -0.00202082f, 0.0018472f },
    { 0.809095f, 0.384289f, -0.244065f, -0.204281f, 0.141678f, 0.212418f, -0.146279f, -0.0900894f, 0.0303338f,
      0.00901412f, 0.0219904f, -0.013372f, 0.00213642f, 0.00160305f, -0.00312701f },
    { 0.640592f, -0.45837f, -0.195219f, 0.465458f, 0.280137f, 0.158692f, -0.120367f, -0.0643137f, -0.0448046f,
      -0.0187873f, 0.0086479f, -0.00588963f, -0.00348534f, -0.00269621f, -0.00170815f },
    { 0.70453f, -0.28001f, -0.493818f, -0.339975f, 0.0707316f, -0.208283f, -0.118894f, -0.0505454f, 0.0230506f,
      0.0083889f, 0.00518717f, 0.00758588f, -0.00342521f, 0.00383168f, 0.00210497f },
    { 0.703638f, -0.511564f, -0.330369f, -0.170887f, -0.248787f, -0.192427f, 0.0327586f, 0.0173555f, -0.0533113f,
      0.0323278f, -0.0197953f, -0.013261f, -0.00818983f, 0.00455043f, 0.00303056f },
    { 0.502795f, -0.617971f, 0.439203f, 0.340489f, 0.212697f, 0.0373525f, -0.0839832f, -0.0489438f, 0.0120163f,
      -0.0118712f, -0.0068611f, -0.00683533f, 0.00253129f, -0.00261332f, 0.00114403f },
    { 0.545938f, 0.230475f, -0.653433f, -0.401249f, -0.179111f, -0.124174f, -0.0908552f, -0.0653798f, 0.00974389f,
      -0.0250137f, 0.00404122f, -0.00705654f, 0.00540587f, 0.00345629f, -0.000601993f },
    { 0.598568f, 0.250823f, -0.616265f, -0.386885f, 0.0682075f, -0.177518f, -0.0946007f, -0.0618835f, 0.00930753f,
      0.00589474f, -0.0158792f, -0.00252346f, -0.00171983f, 0.00368487f, -0.000765975f },
    { 0.500105f, -0.575323f, -0.490979f, 0.344662f, 0.217855f, -0.0377593f, 0.0851715f, 0.0486429f, -0.00903548f,
      0.0202551f, -0.0038204f, -0.00234486f, -0.00463454f, 0.00105326f, -0.00219229f },
    { 0.790444f, 0.426289f, 0.229008f, 0.267166f, -0.0920133f, 0.235856f, -0.0314124f, -0.02112f, 0.0489275f, 0.0324846f,
      0.0210133f, 0.0126003f, 0.00783998f, -0.00121487f, 0.00377977f },
    { 0.54236f, -0.450355f, -0.514076f, -0.370699f, 0.257717f, -0.151349f, 0.0989672f, 0.0145469f, 0.0378353f,
      -0.0217275f, -0.0054008f, -0.00552614f, 0.00543619f, 0.00278937f, 0.00145596f },
    { 0.644532f, -0.508983f, 0.198126f, 0.393624f, 0.295048f, 0.162427f, -0.105934f, -0.0733827f, 0.0110677f, 0.0282703f,
      0.0170724f, -0.0025895f, 0.00757263f, 0.00102943f, 0.000647898f },
    { 0.808776f, -0.320082f, -0.307485f, -0.257502f, -0.210478f, -0.140683f, -0.132368f, 0.0220602f, 0.0134601f,
      0.00872731f, 0.0056483f, 0.0129837f, -0.00224295f, -0.00147026f, 0.00303588f },
    { 0.666471f, 0.268791f, 0.480366f, -0.344214f, 0.284311f, -0.186689f, 0.105519f, 0.0666362f, 0.0436796f, 0.0297651f,
      0.0157616f, -0.00362123f, -0.00340852f, -0.00190781f, -0.00147408f },
    { 0.573974f, -0.511184f, 0.431912f, 0.353949f, 0.25726f, -0.139343f, -0.0967815f, 0.0331101f, 0.0352968f,
      -0.00671239f, 0.00582252f, 0.00296095f, -0.00316478f, 0.00317204f, -0.00108935f },
    { 0.632047f, -0.535446f, -0.441594f, -0.291997f, 0.0754844f, -0.155377f, 0.0284143f, -0.0213323f, 0.044504f,
      0.0124667f, 0.0165113f, -0.0030386f, -0.00160851f, 0.00420897f, 0.00247172f },
    { 0.752077f, 0.306983f, -0.183397f, 0.459299f, -0.271736f, -0.0492063f, 0.122286f, 0.0195556f, 0.0479247f,
      0.0320558f, 0.019255f, 0.0128277f, 0.00741277f, 0.00555198f, -0.00297266f },
    { 0.535717f, 0.325478f, -0.635643f, -0.361751f, -0.237797f, 0.0395079f, -0.0972639f, -0.0524668f, -0.0345837f,
      -0.0220477f, -0.0111412f, -0.00542113f, -0.00226762f, -0.00156113f, -0.00102175f },
    { 0.473236f, -0.669643f, -0.549987f, 0.0845639f, 0.0563419f, -0.112259f, -0.0193897f, -0.0127122f, 0.0315697f,
      0.0216217f, 0.0130481f, -0.00203133f, 0.00473074f, 0.00310063f, 0.00202887f },
    { 0.599948f, 0.698349f, 0.315535f, -0.166615f, -0.0664979f, 0.12967f, -0.0268796f, -0.0169886f, 0.0429952f,
      0.0259455f, 0.0145957f, -0.00264132f, 0.00586596f, 0.00438405f, 0.00233752f },
    { 0.585865f, 0.545201f, 0.336844f, -0.39496f, -0.250678f, 0.15755f, -0.0291509f, -0.0245335f, -0.0179053f,
      0.0228881f, -0.00667042f, -0.00580426f, -0.00396057f, 0.0027146f, 0.00154392f },
    { 0.521717f, -0.408333f, 0.59855f, -0.359589f, -0.210108f, -0.13331f, -0.0934335f, -0.0501755f, 0.00998638f,
      0.00673967f, -0.0130865f, 0.00373021f, 0.00306118f, -0.00184667f, -0.0012274f },
    { 0.547159f, 0.759898f, 0.286852f, 0.116451f, 0.112013f, -0.0335622f, 0.0963419f, 0.0571354f, -0.010273f, 0.0250233f,
      0.0163803f, 0.00940735f, -0.00160056f, 0.00333937f, 0.00238151f },
    { 0.510171f, -0.562011f, -0.445687f, -0.381444f, 0.219051f, -0.148673f, 0.0886093f, 0.0127818f, 0.03348f, 0.0196829f,
      -0.00358054f, 0.00886413f, 0.00514986f, -0.000821141f, 0.00207857f },
    { 0.625912f, 0.427867f, -0.457251f, -0.393441f, 0.129927f, -0.166166f, -0.116753f, 0.0179003f, -0.0416928f,
      -0.0267294f, -0.0154719f, -0.0103055f, -0.00672953f, 0.00124772f, -0.000765191f },
    { 0.536132f, -0.395747f, 0.567031f, 0.388056f, -0.232221f, -0.141341f, 0.0930191f, -0.016878f, 0.0205998f,
      -0.0191579f, -0.0120737f, 0.00558604f, -0.00438587f, -0.00213824f, 0.00132595f },
    { 0.707918f, 0.514049f, -0.326191f, -0.270783f, -0.0755418f, 0.201865f, 0.0279346f, -0.0832132f, 0.0112654f,
      0.00775179f, -0.0174804f, -0.0120177f, -0.00703483f, -0.00497125f, 0.00310433f },
    { 0.519573f, 0.707109f, 0.391646f, 0.109504f, -0.216993f, -0.0898085f, 0.092334f, -0.0139642f, -0.00921733f,
      0.0217991f, 0.0144964f, -0.00241457f, -0.0012854f, -0.00342594f, -0.000542503f },
};
} // namespace muse::audio::fx
