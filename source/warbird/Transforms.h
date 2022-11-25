#pragma once

namespace PeaceBird
{

	DWORD ReverseByteOrder(DWORD dw);
	DWORD ROL(DWORD v, BYTE ct);
	DWORD ROR(DWORD v, BYTE ct);

	// a1 - index in TransformID array (0..31)
	// a2..a5 - CipherKey[0],CipherKey[1],CipherKey[2],CipherKey[3]
	// a6 points to Chunk3Data. It is array of 4 WORDs (8 bytes).
	// a7 - DWORD being transformed
	// example :
	//  Chunk3 = 08 00 00 00 60 48 54 48 32 6b ec 5f
	//  a6 = 60 48 54 48 32 6b ec 5f
	typedef DWORD (*t_WBTransform)(DWORD a1, BYTE a2, BYTE a3, BYTE a4, BYTE a5, const BYTE *a6, DWORD a7);

	extern t_WBTransform WBTransform[31];
}
