///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Image (gli.g-truc.net)
///
/// Copyright (c) 2008 - 2015 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @ref core
/// @file gli/core/gl.hpp
/// @date 2013-11-09 / 2013-11-09
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "storage.hpp"
#include <array>

namespace gli
{
	class dx
	{
	public:
		#define GLI_MAKEFOURCC(ch0, ch1, ch2, ch3) \
			(std::uint32_t)( \
			(((std::uint32_t)(std::uint8_t)(ch3) << 24) & 0xFF000000) | \
			(((std::uint32_t)(std::uint8_t)(ch2) << 16) & 0x00FF0000) | \
			(((std::uint32_t)(std::uint8_t)(ch1) <<  8) & 0x0000FF00) | \
			((std::uint32_t)(std::uint8_t)(ch0)        & 0x000000FF) )

		enum D3DFORMAT
		{
			D3DFMT_UNKNOWN				=  0,

			D3DFMT_R8G8B8				= 20,
			D3DFMT_A8R8G8B8				= 21,
			D3DFMT_X8R8G8B8				= 22,
			D3DFMT_R5G6B5				= 23,
			D3DFMT_X1R5G5B5				= 24,
			D3DFMT_A1R5G5B5				= 25,
			D3DFMT_A4R4G4B4				= 26,
			D3DFMT_R3G3B2				= 27,
			D3DFMT_A8					= 28,
			D3DFMT_A8R3G3B2				= 29,
			D3DFMT_X4R4G4B4				= 30,
			D3DFMT_A2B10G10R10			= 31,
			D3DFMT_A8B8G8R8				= 32,
			D3DFMT_X8B8G8R8				= 33,
			D3DFMT_G16R16				= 34,
			D3DFMT_A2R10G10B10			= 35,
			D3DFMT_A16B16G16R16			= 36,

			D3DFMT_A8P8					= 40,
			D3DFMT_P8					= 41,

			D3DFMT_L8					= 50,
			D3DFMT_A8L8					= 51,
			D3DFMT_A4L4					= 52,

			D3DFMT_V8U8					= 60,
			D3DFMT_L6V5U5				= 61,
			D3DFMT_X8L8V8U8				= 62,
			D3DFMT_Q8W8V8U8				= 63,
			D3DFMT_V16U16				= 64,
			D3DFMT_A2W10V10U10			= 67,

			D3DFMT_UYVY					= GLI_MAKEFOURCC('U', 'Y', 'V', 'Y'),
			D3DFMT_R8G8_B8G8			= GLI_MAKEFOURCC('R', 'G', 'B', 'G'),
			D3DFMT_YUY2					= GLI_MAKEFOURCC('Y', 'U', 'Y', '2'),
			D3DFMT_G8R8_G8B8			= GLI_MAKEFOURCC('G', 'R', 'G', 'B'),
			D3DFMT_DXT1					= GLI_MAKEFOURCC('D', 'X', 'T', '1'),
			D3DFMT_DXT2					= GLI_MAKEFOURCC('D', 'X', 'T', '2'),
			D3DFMT_DXT3					= GLI_MAKEFOURCC('D', 'X', 'T', '3'),
			D3DFMT_DXT4					= GLI_MAKEFOURCC('D', 'X', 'T', '4'),
			D3DFMT_DXT5					= GLI_MAKEFOURCC('D', 'X', 'T', '5'),

			D3DFMT_ATI1					= GLI_MAKEFOURCC('A', 'T', 'I', '1'),
			D3DFMT_AT1N					= GLI_MAKEFOURCC('A', 'T', '1', 'N'),
			D3DFMT_ATI2					= GLI_MAKEFOURCC('A', 'T', 'I', '2'),
			D3DFMT_AT2N					= GLI_MAKEFOURCC('A', 'T', '2', 'N'),

			D3DFMT_ETC					= GLI_MAKEFOURCC('E', 'T', 'C', ' '),
			D3DFMT_ETC1					= GLI_MAKEFOURCC('E', 'T', 'C', '1'),
			D3DFMT_ATC					= GLI_MAKEFOURCC('A', 'T', 'C', ' '),
			D3DFMT_ATCA					= GLI_MAKEFOURCC('A', 'T', 'C', 'A'),
			D3DFMT_ATCI					= GLI_MAKEFOURCC('A', 'T', 'C', 'I'),

			D3DFMT_POWERVR_2BPP			= GLI_MAKEFOURCC('P', 'T', 'C', '2'),
			D3DFMT_POWERVR_4BPP			= GLI_MAKEFOURCC('P', 'T', 'C', '4'),

			D3DFMT_D16_LOCKABLE			= 70,
			D3DFMT_D32					= 71,
			D3DFMT_D15S1				= 73,
			D3DFMT_D24S8				= 75,
			D3DFMT_D24X8				= 77,
			D3DFMT_D24X4S4				= 79,
			D3DFMT_D16					= 80,

			D3DFMT_D32F_LOCKABLE		= 82,
			D3DFMT_D24FS8				= 83,

			D3DFMT_L16					= 81,

			D3DFMT_VERTEXDATA			=100,
			D3DFMT_INDEX16				=101,
			D3DFMT_INDEX32				=102,

			D3DFMT_Q16W16V16U16			=110,

			D3DFMT_MULTI2_ARGB8			= GLI_MAKEFOURCC('M','E','T','1'),

			D3DFMT_R16F					= 111,
			D3DFMT_G16R16F				= 112,
			D3DFMT_A16B16G16R16F		= 113,

			D3DFMT_R32F					= 114,
			D3DFMT_G32R32F				= 115,
			D3DFMT_A32B32G32R32F		= 116,

			D3DFMT_CxV8U8				= 117,

			D3DFMT_DX10					= GLI_MAKEFOURCC('D', 'X', '1', '0'),

			D3DFMT_FORCE_DWORD			= 0x7fffffff
		};

		enum dxgiFormat
		{
			DXGI_FORMAT_UNKNOWN							= 0,
			DXGI_FORMAT_R32G32B32A32_TYPELESS			= 1,
			DXGI_FORMAT_R32G32B32A32_FLOAT				= 2,
			DXGI_FORMAT_R32G32B32A32_UINT				= 3,
			DXGI_FORMAT_R32G32B32A32_SINT				= 4,
			DXGI_FORMAT_R32G32B32_TYPELESS				= 5,
			DXGI_FORMAT_R32G32B32_FLOAT					= 6,
			DXGI_FORMAT_R32G32B32_UINT					= 7,
			DXGI_FORMAT_R32G32B32_SINT					= 8,
			DXGI_FORMAT_R16G16B16A16_TYPELESS			= 9,
			DXGI_FORMAT_R16G16B16A16_FLOAT				= 10,
			DXGI_FORMAT_R16G16B16A16_UNORM				= 11,
			DXGI_FORMAT_R16G16B16A16_UINT				= 12,
			DXGI_FORMAT_R16G16B16A16_SNORM				= 13,
			DXGI_FORMAT_R16G16B16A16_SINT				= 14,
			DXGI_FORMAT_R32G32_TYPELESS					= 15,
			DXGI_FORMAT_R32G32_FLOAT					= 16,
			DXGI_FORMAT_R32G32_UINT						= 17,
			DXGI_FORMAT_R32G32_SINT						= 18,
			DXGI_FORMAT_R32G8X24_TYPELESS				= 19,
			DXGI_FORMAT_D32_FLOAT_S8X24_UINT			= 20,
			DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS		= 21,
			DXGI_FORMAT_X32_TYPELESS_G8X24_UINT			= 22,
			DXGI_FORMAT_R10G10B10A2_TYPELESS			= 23,
			DXGI_FORMAT_R10G10B10A2_UNORM				= 24,
			DXGI_FORMAT_R10G10B10A2_UINT				= 25,
			DXGI_FORMAT_R11G11B10_FLOAT					= 26,
			DXGI_FORMAT_R8G8B8A8_TYPELESS				= 27,
			DXGI_FORMAT_R8G8B8A8_UNORM					= 28,
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB				= 29,
			DXGI_FORMAT_R8G8B8A8_UINT					= 30,
			DXGI_FORMAT_R8G8B8A8_SNORM					= 31,
			DXGI_FORMAT_R8G8B8A8_SINT					= 32,
			DXGI_FORMAT_R16G16_TYPELESS					= 33,
			DXGI_FORMAT_R16G16_FLOAT					= 34,
			DXGI_FORMAT_R16G16_UNORM					= 35,
			DXGI_FORMAT_R16G16_UINT						= 36,
			DXGI_FORMAT_R16G16_SNORM					= 37,
			DXGI_FORMAT_R16G16_SINT						= 38,
			DXGI_FORMAT_R32_TYPELESS					= 39,
			DXGI_FORMAT_D32_FLOAT						= 40,
			DXGI_FORMAT_R32_FLOAT						= 41,
			DXGI_FORMAT_R32_UINT						= 42,
			DXGI_FORMAT_R32_SINT						= 43,
			DXGI_FORMAT_R24G8_TYPELESS					= 44,
			DXGI_FORMAT_D24_UNORM_S8_UINT				= 45,
			DXGI_FORMAT_R24_UNORM_X8_TYPELESS			= 46,
			DXGI_FORMAT_X24_TYPELESS_G8_UINT			= 47,
			DXGI_FORMAT_R8G8_TYPELESS					= 48,
			DXGI_FORMAT_R8G8_UNORM						= 49,
			DXGI_FORMAT_R8G8_UINT						= 50,
			DXGI_FORMAT_R8G8_SNORM						= 51,
			DXGI_FORMAT_R8G8_SINT						= 52,
			DXGI_FORMAT_R16_TYPELESS					= 53,
			DXGI_FORMAT_R16_FLOAT						= 54,
			DXGI_FORMAT_D16_UNORM						= 55,
			DXGI_FORMAT_R16_UNORM						= 56,
			DXGI_FORMAT_R16_UINT						= 57,
			DXGI_FORMAT_R16_SNORM						= 58,
			DXGI_FORMAT_R16_SINT						= 59,
			DXGI_FORMAT_R8_TYPELESS						= 60,
			DXGI_FORMAT_R8_UNORM						= 61,
			DXGI_FORMAT_R8_UINT							= 62,
			DXGI_FORMAT_R8_SNORM						= 63,
			DXGI_FORMAT_R8_SINT							= 64,
			DXGI_FORMAT_A8_UNORM						= 65,
			DXGI_FORMAT_R1_UNORM						= 66,
			DXGI_FORMAT_R9G9B9E5_SHAREDEXP				= 67,
			DXGI_FORMAT_R8G8_B8G8_UNORM					= 68,
			DXGI_FORMAT_G8R8_G8B8_UNORM					= 69,
			DXGI_FORMAT_BC1_TYPELESS					= 70,
			DXGI_FORMAT_BC1_UNORM						= 71,
			DXGI_FORMAT_BC1_UNORM_SRGB					= 72,
			DXGI_FORMAT_BC2_TYPELESS					= 73,
			DXGI_FORMAT_BC2_UNORM						= 74,
			DXGI_FORMAT_BC2_UNORM_SRGB					= 75,
			DXGI_FORMAT_BC3_TYPELESS					= 76,
			DXGI_FORMAT_BC3_UNORM						= 77,
			DXGI_FORMAT_BC3_UNORM_SRGB					= 78,
			DXGI_FORMAT_BC4_TYPELESS					= 79,
			DXGI_FORMAT_BC4_UNORM						= 80,
			DXGI_FORMAT_BC4_SNORM						= 81,
			DXGI_FORMAT_BC5_TYPELESS					= 82,
			DXGI_FORMAT_BC5_UNORM						= 83,
			DXGI_FORMAT_BC5_SNORM						= 84,
			DXGI_FORMAT_B5G6R5_UNORM					= 85,
			DXGI_FORMAT_B5G5R5A1_UNORM					= 86,
			DXGI_FORMAT_B8G8R8A8_UNORM					= 87,
			DXGI_FORMAT_B8G8R8X8_UNORM					= 88,
			DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM		= 89,
			DXGI_FORMAT_B8G8R8A8_TYPELESS				= 90,
			DXGI_FORMAT_B8G8R8A8_UNORM_SRGB				= 91,
			DXGI_FORMAT_B8G8R8X8_TYPELESS				= 92,
			DXGI_FORMAT_B8G8R8X8_UNORM_SRGB				= 93,
			DXGI_FORMAT_BC6H_TYPELESS					= 94,
			DXGI_FORMAT_BC6H_UF16						= 95,
			DXGI_FORMAT_BC6H_SF16						= 96,
			DXGI_FORMAT_BC7_TYPELESS					= 97,
			DXGI_FORMAT_BC7_UNORM						= 98,
			DXGI_FORMAT_BC7_UNORM_SRGB					= 99,
			DXGI_FORMAT_AYUV							= 100,
			DXGI_FORMAT_Y410							= 101,
			DXGI_FORMAT_Y416							= 102,
			DXGI_FORMAT_NV12							= 103,
			DXGI_FORMAT_P010							= 104,
			DXGI_FORMAT_P016							= 105,
			DXGI_FORMAT_420_OPAQUE						= 106,
			DXGI_FORMAT_YUY2							= 107,
			DXGI_FORMAT_Y210							= 108,
			DXGI_FORMAT_Y216							= 109,
			DXGI_FORMAT_NV11							= 110,
			DXGI_FORMAT_AI44							= 111,
			DXGI_FORMAT_IA44							= 112,
			DXGI_FORMAT_P8								= 113,
			DXGI_FORMAT_A8P8							= 114,
			DXGI_FORMAT_B4G4R4A4_UNORM					= 115,

			DXGI_FORMAT_P208							= 130,
			DXGI_FORMAT_V208							= 131,
			DXGI_FORMAT_V408							= 132,
			DXGI_FORMAT_ASTC_4X4_TYPELESS				= 133,
			DXGI_FORMAT_ASTC_4X4_UNORM					= 134,
			DXGI_FORMAT_ASTC_4X4_UNORM_SRGB				= 135,
			DXGI_FORMAT_ASTC_5X4_TYPELESS				= 137,
			DXGI_FORMAT_ASTC_5X4_UNORM					= 138,
			DXGI_FORMAT_ASTC_5X4_UNORM_SRGB				= 139,
			DXGI_FORMAT_ASTC_5X5_TYPELESS				= 141,
			DXGI_FORMAT_ASTC_5X5_UNORM					= 142,
			DXGI_FORMAT_ASTC_5X5_UNORM_SRGB				= 143,
			DXGI_FORMAT_ASTC_6X5_TYPELESS				= 145,
			DXGI_FORMAT_ASTC_6X5_UNORM					= 146,
			DXGI_FORMAT_ASTC_6X5_UNORM_SRGB				= 147,
			DXGI_FORMAT_ASTC_6X6_TYPELESS				= 149,
			DXGI_FORMAT_ASTC_6X6_UNORM					= 150,
			DXGI_FORMAT_ASTC_6X6_UNORM_SRGB				= 151,
			DXGI_FORMAT_ASTC_8X5_TYPELESS				= 153,
			DXGI_FORMAT_ASTC_8X5_UNORM					= 154,
			DXGI_FORMAT_ASTC_8X5_UNORM_SRGB				= 155,
			DXGI_FORMAT_ASTC_8X6_TYPELESS				= 157,
			DXGI_FORMAT_ASTC_8X6_UNORM					= 158,
			DXGI_FORMAT_ASTC_8X6_UNORM_SRGB				= 159,
			DXGI_FORMAT_ASTC_8X8_TYPELESS				= 161,
			DXGI_FORMAT_ASTC_8X8_UNORM					= 162,
			DXGI_FORMAT_ASTC_8X8_UNORM_SRGB				= 163,
			DXGI_FORMAT_ASTC_10X5_TYPELESS				= 165,
			DXGI_FORMAT_ASTC_10X5_UNORM					= 166,
			DXGI_FORMAT_ASTC_10X5_UNORM_SRGB			= 167,
			DXGI_FORMAT_ASTC_10X6_TYPELESS				= 169,
			DXGI_FORMAT_ASTC_10X6_UNORM					= 170,
			DXGI_FORMAT_ASTC_10X6_UNORM_SRGB			= 171,
			DXGI_FORMAT_ASTC_10X8_TYPELESS				= 173,
			DXGI_FORMAT_ASTC_10X8_UNORM					= 174,
			DXGI_FORMAT_ASTC_10X8_UNORM_SRGB			= 175,
			DXGI_FORMAT_ASTC_10X10_TYPELESS				= 177,
			DXGI_FORMAT_ASTC_10X10_UNORM				= 178,
			DXGI_FORMAT_ASTC_10X10_UNORM_SRGB			= 179,
			DXGI_FORMAT_ASTC_12X10_TYPELESS				= 181,
			DXGI_FORMAT_ASTC_12X10_UNORM				= 182,
			DXGI_FORMAT_ASTC_12X10_UNORM_SRGB			= 183,
			DXGI_FORMAT_ASTC_12X12_TYPELESS				= 185,
			DXGI_FORMAT_ASTC_12X12_UNORM				= 186,
			DXGI_FORMAT_ASTC_12X12_UNORM_SRGB			= 187, DXGI_FORMAT_LAST = DXGI_FORMAT_ASTC_12X12_UNORM_SRGB,
			DXGI_FORMAT_FORCE_UINT						= 0xffffffffUL
		};

		enum DDPF
		{
			DDPF_ALPHAPIXELS = 0x1,
			DDPF_ALPHA = 0x2,
			DDPF_FOURCC = 0x4,
			DDPF_RGB = 0x40,
			DDPF_YUV = 0x200,
			DDPF_LUMINANCE = 0x20000,
			DDPF_LUMINANCE_ALPHA = DDPF_LUMINANCE | DDPF_ALPHA,
			DDPF_FOURCC_ALPHAPIXELS = DDPF_FOURCC | DDPF_ALPHAPIXELS,
			DDPF_RGBAPIXELS = DDPF_RGB | DDPF_ALPHAPIXELS,
			DDPF_RGBA = DDPF_RGB | DDPF_ALPHA,
			DDPF_LUMINANCE_ALPHAPIXELS = DDPF_LUMINANCE | DDPF_ALPHAPIXELS,

		};

		struct format
		{
			DDPF DDPixelFormat;
			D3DFORMAT D3DFormat;
			dxgiFormat DXGIFormat;
			glm::u32vec4 Mask;
		};

	public:
		dx();

		format const & translate(gli::format const & Format) const;
		gli::format find(D3DFORMAT FourCC);
		gli::format find(dxgiFormat Format);

	private:
		std::array<format, FORMAT_COUNT> Translation;
	};
}//namespace gli

#include "dx.inl"
