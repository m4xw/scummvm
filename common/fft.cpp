/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Based on eos' (I)FFT code which is in turn
// Based upon the (I)FFT code in FFmpeg
// Copyright (c) 2008 Loren Merritt
// Copyright (c) 2002 Fabrice Bellard
// Partly based on libdjbfft by D. J. Bernstein

#include "common/cosinetables.h"
#include "common/fft.h"
#include "common/util.h"
#include "common/textconsole.h"

namespace Common {

FFT::FFT(int bits, int inverse) : _bits(bits), _inverse(inverse) {
	assert((_bits >= 2) && (_bits <= 16));

	int n = 1 << bits;

	_tmpBuf = new Complex[n];
	_expTab = new Complex[n / 2];
	_revTab = new uint16[n];

	_splitRadix = 1;

	for (int i = 0; i < n; i++)
		_revTab[-splitRadixPermutation(i, n, _inverse) & (n - 1)] = i;

	for (int i = 0; i < ARRAYSIZE(_cosTables); i++) {
		if (i+4 <= _bits)
			_cosTables[i] = new Common::CosineTable(i+4);
		else
			_cosTables[i] = 0;
	}
}

FFT::~FFT() {
	delete[] _revTab;
	delete[] _expTab;
	delete[] _tmpBuf;
}

void FFT::permute(Complex *z) {
	int np = 1 << _bits;

	if (_tmpBuf) {
		for (int j = 0; j < np; j++)
			_tmpBuf[_revTab[j]] = z[j];

		memcpy(z, _tmpBuf, np * sizeof(Complex));

		return;
	}

	// Reverse
	for (int j = 0; j < np; j++) {
		int k = _revTab[j];

		if (k < j)
			SWAP(z[k], z[j]);
	}
}

int FFT::splitRadixPermutation(int i, int n, int inverse) {
	if (n <= 2)
		return i & 1;

	int m = n >> 1;

	if (!(i & m))
		return splitRadixPermutation(i, m, inverse) * 2;

	m >>= 1;

	if (inverse == !(i & m))
		return splitRadixPermutation(i, m, inverse) * 4 + 1;

	return splitRadixPermutation(i, m, inverse) * 4 - 1;
}

#define sqrthalf (float)M_SQRT1_2

#define BF(x, y, a, b) { \
	x = a - b; \
	y = a + b; \
}

#define BUTTERFLIES(a0, a1, a2, a3) { \
	BF(t3, t5, t5, t1); \
	BF(a2.re, a0.re, a0.re, t5); \
	BF(a3.im, a1.im, a1.im, t3); \
	BF(t4, t6, t2, t6); \
	BF(a3.re, a1.re, a1.re, t4); \
	BF(a2.im, a0.im, a0.im, t6); \
}

// force loading all the inputs before storing any.
// this is slightly slower for small data, but avoids store->load aliasing
// for addresses separated by large powers of 2.
#define BUTTERFLIES_BIG(a0, a1, a2, a3) { \
	float r0 = a0.re, i0 = a0.im, r1 = a1.re, i1 = a1.im; \
	BF(t3, t5, t5, t1); \
	BF(a2.re, a0.re, r0, t5); \
	BF(a3.im, a1.im, i1, t3); \
	BF(t4, t6, t2, t6); \
	BF(a3.re, a1.re, r1, t4); \
	BF(a2.im, a0.im, i0, t6); \
}

#define TRANSFORM(a0, a1, a2, a3, wre, wim) { \
	t1 = a2.re * wre + a2.im * wim; \
	t2 = a2.im * wre - a2.re * wim; \
	t5 = a3.re * wre - a3.im * wim; \
	t6 = a3.im * wre + a3.re * wim; \
	BUTTERFLIES(a0, a1, a2, a3) \
}

#define TRANSFORM_ZERO(a0, a1, a2, a3) { \
	t1 = a2.re; \
	t2 = a2.im; \
	t5 = a3.re; \
	t6 = a3.im; \
	BUTTERFLIES(a0, a1, a2, a3) \
}

/* z[0...8n-1], w[1...2n-1] */
#define PASS(name) \
static void name(Complex *z, const float *wre, unsigned int n) { \
	float t1, t2, t3, t4, t5, t6; \
	int o1 = 2 * n; \
	int o2 = 4 * n; \
	int o3 = 6 * n; \
	const float *wim = wre + o1; \
	n--; \
	\
	TRANSFORM_ZERO(z[0], z[o1], z[o2], z[o3]); \
	TRANSFORM(z[1], z[o1 + 1], z[o2 + 1], z[o3 + 1], wre[1], wim[-1]); \
	do { \
		z += 2; \
		wre += 2; \
		wim -= 2; \
		TRANSFORM(z[0], z[o1], z[o2], z[o3], wre[0], wim[0]);\
		TRANSFORM(z[1], z[o1 + 1], z[o2 + 1], z[o3 + 1], wre[1], wim[-1]);\
	} while(--n);\
}

PASS(pass)
#undef BUTTERFLIES
#define BUTTERFLIES BUTTERFLIES_BIG
PASS(pass_big)

void FFT::fft4(Complex *z) {
	float t1, t2, t3, t4, t5, t6, t7, t8;

	BF(t3, t1, z[0].re, z[1].re);
	BF(t8, t6, z[3].re, z[2].re);
	BF(z[2].re, z[0].re, t1, t6);
	BF(t4, t2, z[0].im, z[1].im);
	BF(t7, t5, z[2].im, z[3].im);
	BF(z[3].im, z[1].im, t4, t8);
	BF(z[3].re, z[1].re, t3, t7);
	BF(z[2].im, z[0].im, t2, t5);
}

void FFT::fft8(Complex *z) {
	float t1, t2, t3, t4, t5, t6, t7, t8;

	fft4(z);

	BF(t1, z[5].re, z[4].re, -z[5].re);
	BF(t2, z[5].im, z[4].im, -z[5].im);
	BF(t3, z[7].re, z[6].re, -z[7].re);
	BF(t4, z[7].im, z[6].im, -z[7].im);
	BF(t8, t1, t3, t1);
	BF(t7, t2, t2, t4);
	BF(z[4].re, z[0].re, z[0].re, t1);
	BF(z[4].im, z[0].im, z[0].im, t2);
	BF(z[6].re, z[2].re, z[2].re, t7);
	BF(z[6].im, z[2].im, z[2].im, t8);

	TRANSFORM(z[1], z[3], z[5], z[7], sqrthalf, sqrthalf);
}

void FFT::fft16(Complex *z) {
	float t1, t2, t3, t4, t5, t6;

	fft8(z);
	fft4(z + 8);
	fft4(z + 12);

	assert(_cosTables[0]);
	const float * const cosTable = _cosTables[0]->getTable();

	TRANSFORM_ZERO(z[0], z[4], z[8], z[12]);
	TRANSFORM(z[2], z[6], z[10], z[14], sqrthalf, sqrthalf);
	TRANSFORM(z[1], z[5], z[9], z[13], cosTable[1],cosTable[3]);
	TRANSFORM(z[3], z[7], z[11], z[15], cosTable[3], cosTable[1]);
}

void FFT::fft32(Complex *z) {
	fft16(z);
	fft8(z + 8 * 2);
	fft8(z + 8 * 3);
	assert(_cosTables[1]);
	pass(z, _cosTables[1]->getTable(), 8 / 2);
}

void FFT::fft64(Complex *z) {
	fft32(z);
	fft16(z + 16 * 2);
	fft16(z + 16 * 3);
	assert(_cosTables[2]);
	pass(z, _cosTables[2]->getTable(), 16 / 2);
}

void FFT::fft128(Complex *z) {
	fft64(z);
	fft32(z + 32 * 2);
	fft32(z + 32 * 3);
	assert(_cosTables[3]);
	pass(z, _cosTables[3]->getTable(), 32 / 2);
}

void FFT::fft256(Complex *z) {
	fft128(z);
	fft64(z + 64 * 2);
	fft64(z + 64 * 3);
	assert(_cosTables[4]);
	pass(z, _cosTables[4]->getTable(), 64 / 2);
}

void FFT::fft512(Complex *z) {
	fft256(z);
	fft128(z + 128 * 2);
	fft128(z + 128 * 3);
	assert(_cosTables[5]);
	pass(z, _cosTables[5]->getTable(), 128 / 2);
}

void FFT::fft1024(Complex *z) {
	fft512(z);
	fft256(z + 256 * 2);
	fft256(z + 256 * 3);
	assert(_cosTables[6]);
	pass_big(z, _cosTables[6]->getTable(), 256 / 2);
}

void FFT::fft2048(Complex *z) {
	fft1024(z);
	fft512(z + 512 * 2);
	fft512(z + 512 * 3);
	assert(_cosTables[7]);
	pass_big(z, _cosTables[7]->getTable(), 512 / 2);
}

void FFT::fft4096(Complex *z) {
	fft2048(z);
	fft1024(z + 1024 * 2);
	fft1024(z + 1024 * 3);
	assert(_cosTables[8]);
	pass_big(z, _cosTables[8]->getTable(), 1024 / 2);
}

void FFT::fft8192(Complex *z) {
	fft4096(z);
	fft2048(z + 2048 * 2);
	fft2048(z + 2048 * 3);
	assert(_cosTables[9]);
	pass_big(z, _cosTables[9]->getTable(), 2048 / 2);
}

void FFT::fft16384(Complex *z) {
	fft8192(z);
	fft4096(z + 4096 * 2);
	fft4096(z + 4096 * 3);
	assert(_cosTables[10]);
	pass_big(z, _cosTables[10]->getTable(), 4096 / 2);
}

void FFT::fft32768(Complex *z) {
	fft16384(z);
	fft8192(z + 8192 * 2);
	fft8192(z + 8192 * 3);
	assert(_cosTables[11]);
	pass_big(z, _cosTables[11]->getTable(), 8192 / 2);
}

void FFT::fft65536(Complex *z) {
	fft32768(z);
	fft16384(z + 16384 * 2);
	fft16384(z + 16384 * 3);
	assert(_cosTables[12]);
	pass_big(z, _cosTables[12]->getTable(), 16384 / 2);
}

void FFT::calc(Complex *z) {
	switch (_bits) {
	case 2:
		fft4(z);
		break;
	case 3:
		fft8(z);
		break;
	case 4:
		fft16(z);
		break;
	case 5:
		fft32(z);
		break;
	case 6:
		fft64(z);
		break;
	case 7:
		fft128(z);
		break;
	case 8:
		fft256(z);
		break;
	case 9:
		fft512(z);
		break;
	case 10:
		fft1024(z);
		break;
	case 11:
		fft2048(z);
		break;
	case 12:
		fft4096(z);
		break;
	case 13:
		fft8192(z);
		break;
	case 14:
		fft16384(z);
		break;
	case 15:
		fft32768(z);
		break;
	case 16:
		fft65536(z);
		break;
	default:
		error("Should Not Happen!");
	}
}

} // End of namespace Common
