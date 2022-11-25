#include "stdafx.h"
#include "Transforms.h"

namespace PeaceBird
{
	DWORD ReverseByteOrder(DWORD dw)
	{
		return ((dw>>24)&0x000000FF) | ((dw>>8)&0x0000FF00) | ((dw<<8)&0x00FF0000) | (dw<<24);
	}
	DWORD ROL(DWORD v, BYTE ct)
	{
		return (v << ct) | (v >> (32-ct));
	}
	DWORD ROR(DWORD v, BYTE ct)
	{
		return ROL(v,32-ct);
	}


	static DWORD Transform0(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		return *(WORD *)(a6 + 2 * (a1 & 3)) * (a7 ^ *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3))) + (a7 >> (a3 % 0xFu + 1));
	}
	static DWORD Transform1(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		return *(WORD *)(a6 + 2 * (a1 & 3)) * (a7 - *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3))) - (a7 >> (a3 % 0xFu + 1));
	}
	static DWORD Transform2(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		return (a7 >> (a3 % 0xFu + 1)) ^ *(WORD *)(a6 + 2 * (a1 & 3)) * (a7 ^ *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3)));
	}
	static DWORD Transform3(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;

		v7 = ROR(a7, a3 % 0xF + 1);
		return v7 + *(WORD *)(a6 + 2 * (a1 & 3)) * (a7 ^ *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3)));
	}
	static DWORD Transform4(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;

		v7 = ROR(a7, a3 % 0xF + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * (a7 ^ *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3))) - v7;
	}
	static DWORD Transform5(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;

		v7 = ROR(a7, a3 % 0xF + 1);
		return v7 ^ *(WORD *)(a6 + 2 * (a1 & 3)) * (a7 ^ *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3)));
	}
	static DWORD Transform6(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;

		v7 = ROL(a7 ^ *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3)), a3 % 7 + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 + (a7 >> (a4 % 0xFu + 1));
	}
	static DWORD Transform7(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;

		v7 = ROL(a7 ^ *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)), a3 % 7 + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 - (a7 >> (a4 % 0xFu + 1));
	}
	static DWORD Transform8(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;

		v7 = ROL(a7 - *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3)), a3 % 7 + 1);
		return (a7 >> (a4 % 0xFu + 1)) ^ *(WORD *)(a6 + 2 * (a1 & 3)) * v7;
	}
	static DWORD Transform9(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROL(a7 ^ *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3)), a3 % 7 + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 + v8;
	}
	static DWORD Transform10(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROL(a7 ^ *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3)), a3 % 7 + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 - v8;
	}
	static DWORD Transform11(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROL(a7 ^ *(WORD *)(a6 + 2 * ((a2 % 3 + a1 + 1) & 3)), a3 % 7 + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return v8 ^ *(WORD *)(a6 + 2 * (a1 & 3)) * v7;
	}
	static DWORD Transform12(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(a7 + *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)), a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 + v8;
	}
	static DWORD Transform13(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(*(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)) - a7, a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 + v8;
	}
	static DWORD Transform14(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(a7 ^ *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)), a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 + v8;
	}
	static DWORD Transform15(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(a7 + *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)), a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 - v8;
	}
	static DWORD Transform16(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(*(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)) - a7, a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 - v8;
	}
	static DWORD Transform17(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(a7 ^ *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)), a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 - v8;
	}
	static DWORD Transform18(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(a7 - *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)), a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return v8 ^ *(WORD *)(a6 + 2 * (a1 & 3)) * v7;
	}
	static DWORD Transform19(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(*(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)) - a7, a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return v8 ^ *(WORD *)(a6 + 2 * (a1 & 3)) * v7;
	}
	static DWORD Transform20(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(a7 ^ *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)), a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return v8 ^ *(WORD *)(a6 + 2 * (a1 & 3)) * v7;
	}
	static DWORD Transform21(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(a7 - *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)), a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 + v8;
	}
	static DWORD Transform22(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(a7 - *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)), a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return *(WORD *)(a6 + 2 * (a1 & 3)) * v7 - v8;
	}
	static DWORD Transform23(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;
		int v8;

		v7 = ROR(a7 - *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)), a3 % 0x1F + 1);
		v8 = ROR(a7, a4 % 0x1Fu + 1);
		return v8 ^ *(WORD *)(a6 + 2 * (a1 & 3)) * v7;
	}
	static DWORD Transform24(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;

		v7 = ROR(~a7, a2 % 0xF + 1);
		return (*(WORD *)(a6 + 2 * (a1 & 3)) + v7) * *(WORD *)(a6 + 2 * ((a3 % 3 + a1 + 1) & 3));
	}
	static DWORD Transform25(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;

		v7 = ROR(a7, a2 % 0xF + 1);
		return (v7 - *(WORD *)(a6 + 2 * (a1 & 3))) * *(WORD *)(a6 + 2 * ((a3 % 3 + a1 + 1) & 3));
	}
	static DWORD Transform26(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		int v7;

		v7 = ROR(a7, a2 % 0xF + 1);
		return (*(WORD *)(a6 + 2 * (a1 & 3)) ^ v7) * *(WORD *)(a6 + 2 * ((a3 % 3 + a1 + 1) & 3));
	}
	static DWORD Transform27(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		return *(DWORD *)(a6 + 4 * ((a1 - 1) & 1)) - (a7 ^ *(DWORD *)(a6 + 4 * (a1 & 1)));
	}
	static DWORD Transform28(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		return a7 ^ *(DWORD *)(a6 + 4 * (a1 & 1)) ^ *(DWORD *)(a6 + 4 * ((a1 - 1) & 1));
	}
	static DWORD Transform29(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		return *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)) ^ (a7 - *(WORD *)(a6 + 2 * (a1 & 3)));
	}
	static DWORD Transform30(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7)
	{
		return a7 - *(DWORD *)(a6 + 4 * (((BYTE)(a1 >> 1) - 1) & 1)) - *(WORD *)(a6 + 2 * (a1 & 3));
	}


	t_WBTransform WBTransform[31] =
	{
		Transform0,Transform1,Transform2,Transform3,Transform4,
		Transform5,Transform6,Transform7,Transform8,Transform9,
		Transform10,Transform11,Transform12,Transform13,Transform14,
		Transform15,Transform16,Transform17,Transform18,Transform19,
		Transform20,Transform21,Transform22,Transform23,Transform24,
		Transform25,Transform26,Transform27,Transform28,Transform29,
		Transform30
	};

}