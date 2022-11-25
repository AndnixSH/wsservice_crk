#include "stdafx.h"
#include "PeaceBird.h"
#include "Transforms.h"

namespace PeaceBird
{
	DWORD CChunk::ChunkSize(LPCVOID pChunk,size_t szChunkBuf)
	{
		if (szChunkBuf<4)
			throw CPeaceBirdExceptionInvalidDataFormat();
		DWORD ChunkDataSize = *(PDWORD)pChunk;
		if (szChunkBuf<(4+ChunkDataSize))
			throw CPeaceBirdExceptionInvalidDataFormat();
		return ChunkDataSize;
	}
	void CChunk::UnpackChunk(LPCVOID &pChunk,size_t &szChunkBuf,DWORD szChunkExpected,PVOID pDest)
	{
		DWORD dwSize;
		dwSize = ChunkSize(pChunk,szChunkBuf);
		if (dwSize!=szChunkExpected)
			throw CPeaceBirdExceptionInvalidDataFormat();
		pChunk = (LPBYTE)pChunk + 4;
		szChunkBuf-=4;
		memcpy(pDest,pChunk,dwSize);
		pChunk = (LPBYTE)pChunk + dwSize;
		szChunkBuf-=dwSize;
	}
	void CChunk::ParseAppend(LPCVOID &pBuf,size_t &sz)
	{
		size_t szDataOld = GetCount();
		DWORD dwSize;
		dwSize = ChunkSize(pBuf,sz);
		if (!SetCount(szDataOld+dwSize))
			throw CAtlException(E_OUTOFMEMORY);
		UnpackChunk(pBuf,sz,dwSize,GetData()+szDataOld);
	}
	void CChunk::Parse(LPCVOID &pBuf,size_t &sz)
	{
		RemoveAll();
		ParseAppend(pBuf,sz);
	}
	void CChunk::PackAppend(CAtlArray<BYTE> &target) const
	{
		size_t szDataOld = target.GetCount();
		size_t szThisData = GetCount();
		if (!target.SetCount(szDataOld+4+szThisData))
			throw CAtlException(E_OUTOFMEMORY);
		LPBYTE pBuf = target.GetData()+szDataOld;
		*(PDWORD)pBuf = (DWORD)GetCount();
		pBuf += 4;
		memcpy(pBuf,GetData(),szThisData);
	}
	void CChunk::Pack(CAtlArray<BYTE> &target) const
	{
		target.RemoveAll();
		PackAppend(target);
	}
	BOOL CChunk::IsDword() const
	{
		return GetCount()==4;
	}
	DWORD CChunk::GetDword() const
	{
		if (!IsDword())
			throw CPeaceBirdExceptionUnexpectedChunkStructure();
		return *(PDWORD)GetData();
	}
	void CChunk::SetDword(DWORD dw)
	{
		if (!IsDword())
			if (!SetCount(sizeof(DWORD)))
				throw CAtlException(E_OUTOFMEMORY);
		*(PDWORD)GetData() = dw;
	}
	CStringW CChunk::ToWString() const
	{
		CStringW ws;
		int wLen = (int)GetCount()/sizeof(WCHAR);
		if (PWCHAR pb = ws.GetBuffer(wLen+1))
		{
			memcpy(pb,GetData(),wLen*sizeof(WCHAR));
			pb[wLen]=0;
			ws.ReleaseBuffer();
		}
		else
			throw CAtlException(E_OUTOFMEMORY);
		return ws;
	}
	void CChunk::FromWString(LPCWSTR ws)
	{
		if (ws)
		{
			size_t szbuf = sizeof(*ws)*(wcslen(ws)+1);
			if (!SetCount(szbuf))
				throw CAtlException(E_OUTOFMEMORY);
			memcpy(GetData(),ws,szbuf);
		}
		else
			RemoveAll();
	}
	void CChunk::operator=(const CChunk &p)
	{
		if (!SetCount(p.GetCount()))
			throw CAtlException(E_OUTOFMEMORY);
		memcpy(GetData(),p.GetData(),GetCount());
	}


	DWORD CChunk::GetPackedSize() const
	{
		return 4+(DWORD)GetCount();
	}

	int CChunkCollection::ParseAppend(LPCVOID &pBuf,size_t &sz,int MaxChunks)
	{
		CChunk chunk;
		for(int i=0;;i++)
		{
			if (MaxChunks!=-1)
			{
				if (!MaxChunks) return i;
				MaxChunks--;
			}
			size_t ct = GetCount();
			if (!SetCount(ct+1)) return i;
			try
			{
				GetAt(ct).Parse(pBuf,sz);
			}
			catch(CPeaceBirdExceptionInvalidDataFormat)
			{
				RemoveAt(ct);
				return i;
			}
		}
	}
	int CChunkCollection::Parse(LPCVOID &pBuf,size_t &sz,int MaxChunks)
	{
		RemoveAll();
		return ParseAppend(pBuf,sz,MaxChunks);
	}
	void CChunkCollection::PackAppend(CAtlArray<BYTE> &target) const
	{
		for(size_t i=0;i<GetCount();i++)
			GetAt(i).PackAppend(target);
	}
	void CChunkCollection::Pack(CAtlArray<BYTE> &target) const
	{
		target.RemoveAll();
		PackAppend(target);
	}
	DWORD CChunkCollection::GetPackedSize() const
	{
		DWORD dwSize=0;
		for(size_t i=0;i<GetCount();i++) dwSize+=(DWORD)GetAt(i).GetPackedSize();
		return dwSize;
	}
	void CChunkCollection::AddChunk(const CChunk &c)
	{
		size_t idx = GetCount();
		if (!SetCount(GetCount()+1))
			throw CAtlException(E_OUTOFMEMORY);
		try
		{
			GetAt(idx).Copy(c);
		}
		catch(...)
		{
			RemoveAt(idx);
			throw;
		}
	}

	void CChunkedMessage::Parse(LPCVOID &pBuf,size_t &sz)
	{
		if (sz<8)
			throw CPeaceBirdExceptionInvalidDataFormat();
		DWORD dwChunkCount = *(PDWORD)pBuf;
		DWORD dwTotalSize = *((PDWORD)pBuf+1);
		if ((dwTotalSize+8)>sz)
			throw CPeaceBirdExceptionInvalidDataFormat();
		pBuf = (PBYTE)pBuf+8;
		if (m_Chunks.Parse(pBuf,sz,dwChunkCount)!=dwChunkCount)
			throw CPeaceBirdExceptionInvalidDataFormat();
	}
	void CChunkedMessage::PackAppend(CAtlArray<BYTE> &target) const
	{
		size_t szOld = target.GetCount();
		if (!target.SetCount(szOld+8))
			throw CAtlException(E_OUTOFMEMORY);
		*(PDWORD)(target.GetData()+szOld) = (DWORD)m_Chunks.GetCount();
		*(PDWORD)(target.GetData()+szOld+4) = m_Chunks.GetPackedSize();
		m_Chunks.PackAppend(target);
	}
	void CChunkedMessage::Pack(CAtlArray<BYTE> &target) const
	{
		target.RemoveAll();
		PackAppend(target);
	}
	DWORD CChunkedMessage::GetPackedSize() const
	{
		return 8 + m_Chunks.GetPackedSize();
	}
	void CChunkedMessage::PrepareQuery(DWORD CommandID,const CWBCipherParams &cp)
	{
		m_Chunks.RemoveAll();
		if (!m_Chunks.SetCount(1))
			throw CAtlException(E_OUTOFMEMORY);
		m_Chunks[0].SetDword(CommandID);
		m_Chunks.AddChunk(cp.cCiph2);
		m_Chunks.AddChunk(cp.Ciph3);
		CChunk c;
		if (!c.SetCount(8))
			throw CAtlException(E_OUTOFMEMORY);
		*(DWORD64*)c.GetData() = __rdtsc();
		m_Chunks.AddChunk(c);
	}


	CWBCipherParams::CWBCipherParams()
	{
		Ciph2 = NULL;
	}
	void CWBCipherParams::Parse(LPCVOID &pBuf,size_t &sz)
	{
		cCiph2.Parse(pBuf,sz);
		Ciph3.Parse(pBuf,sz);
		if (cCiph2.GetCount()!=sizeof(SCiph2) || Ciph3.GetCount()!=8)
			throw CPeaceBirdExceptionUnexpectedChunkStructure();
		Ciph2 = (SCiph2*)cCiph2.GetData();
	}
	void CWBCipherParams::PackAppend(CAtlArray<BYTE> &target) const
	{
		cCiph2.PackAppend(target);
		Ciph3.PackAppend(target);
	}
	void CWBCipherParams::Pack(CAtlArray<BYTE> &target) const
	{
		target.RemoveAll();
		PackAppend(target);
	}
	void CWBCipherParams::operator=(const CWBCipherParams &p)
	{
		cCiph2.Copy(p.cCiph2);
		Ciph3.Copy(p.Ciph3);
		Ciph2 = (SCiph2*)cCiph2.GetData();
	}
	void CWBCipherParams::FromChunkedMessage(const CChunkedMessage &cmsg)
	{
		if (cmsg.m_Chunks.GetCount()<3 || cmsg.m_Chunks[1].GetCount()!=sizeof(SCiph2) || cmsg.m_Chunks[2].GetCount()!=8) 
			throw CPeaceBirdExceptionUnexpectedChunkStructure();
		cCiph2.Copy(cmsg.m_Chunks[1]);
		Ciph3.Copy(cmsg.m_Chunks[2]);
		Ciph2 = (SCiph2*)cCiph2.GetData();
	}
	void CWBCipherParams::SetNullTransform()
	{
		if (!cCiph2.SetCount(sizeof(SCiph2)))
			throw CAtlException(E_OUTOFMEMORY);
		if (!Ciph3.SetCount(8))
			throw CAtlException(E_OUTOFMEMORY);
		Ciph2 = (SCiph2*)cCiph2.GetData();
		memset(Ciph2->X,0,sizeof(Ciph2->X));
		memset(Ciph2->A,0x1F,sizeof(Ciph2->A));
		memset(Ciph3.GetData(),0,Ciph3.GetCount());
	}
	void CWBCipherParams::CheckValid() const
	{
		if (cCiph2.GetCount()!=sizeof(SCiph2) ||
			!Ciph2 ||
			Ciph3.GetCount()!=8)
		{
			throw CPeaceBirdExceptionCipherParamsError();
		}
		
	}

	void CWBEncryptedBlock::Parse(LPCVOID &pBuf,size_t &sz)
	{
		m_EncryptedData.Parse(pBuf,sz);
		if (m_EncryptedData.GetCount()<8)
			throw CPeaceBirdExceptionUnexpectedChunkStructure();
		m_CipherParams.Parse(pBuf,sz);
	}
	void CWBEncryptedBlock::PackAppend(CAtlArray<BYTE> &buf) const
	{
		m_EncryptedData.PackAppend(buf);
		m_CipherParams.PackAppend(buf);
	}
	void CWBEncryptedBlock::Pack(CAtlArray<BYTE> &buf) const
	{
		buf.RemoveAll();
		PackAppend(buf);
	}


	static void ApplyTransform(const CWBCipherParams &cp,DWORD &DW1,DWORD &DW2,int Ciph2_A_Idx)
	{
		DWORD xor;
		BYTE TransformID = cp.Ciph2->A[Ciph2_A_Idx];
		if (TransformID<=30)
		{
			xor = WBTransform[TransformID]
				(Ciph2_A_Idx,
				 cp.Ciph2->X[Ciph2_A_Idx*4],
				 cp.Ciph2->X[Ciph2_A_Idx*4+1],
				 cp.Ciph2->X[Ciph2_A_Idx*4+2],
				 cp.Ciph2->X[Ciph2_A_Idx*4+3],
				 cp.Ciph3.GetData(),DW1);
			DW2 ^= xor;
		}
	}
	static void ApplyTransformsDecrypt(const CWBCipherParams &cp,DWORD &DW1,DWORD &DW2)
	{
		for(int Ciph2_A_Idx=30 ; Ciph2_A_Idx>=0 ; Ciph2_A_Idx-=2)
		{
			ApplyTransform(cp,DW1,DW2,Ciph2_A_Idx+1);
			ApplyTransform(cp,DW2,DW1,Ciph2_A_Idx);
		}
	}
	static void ApplyTransformsEncrypt(const CWBCipherParams &cp,DWORD &DW1,DWORD &DW2)
	{
		for(int Ciph2_A_Idx=0 ; Ciph2_A_Idx<=30 ; Ciph2_A_Idx+=2)
		{
			ApplyTransform(cp,DW2,DW1,Ciph2_A_Idx);
			ApplyTransform(cp,DW1,DW2,Ciph2_A_Idx+1);
		}
	}

	static void ProcessSlack(const CWBCipherParams &CipherParams,size_t szData,const BYTE* &pData,LPBYTE &pOutput,DWORD &Chain1,DWORD &Chain2,DWORD &X1,DWORD &X2)
	{
		Chain1=0;
		Chain2=0;
		X1=0;
		X2=-1;
		if (BYTE szSlack = (BYTE)(szData % 8))
		{
			DWORD Slack[2];
			memcpy(Slack,pData,szSlack);
			memset((PBYTE)Slack+szSlack,0,sizeof(Slack)-szSlack);
			X1 = Slack[0] = ReverseByteOrder(Slack[0]);
			X2 = Slack[1] = ReverseByteOrder(Slack[1]);
			DWORD DW1=0,DW2=-1;
			ApplyTransformsDecrypt(CipherParams,DW1,DW2);
			Slack[0] ^= DW1;
			Slack[1] ^= DW2;
			int sz = szSlack-4;
			if (szSlack>4)
			{
				Chain1 = Slack[0];
				Chain2 = Slack[1] >> (8*(4-sz)) << (8*(4-sz));
			}
			else
			{
				Chain1 = Slack[0] >> (-8*sz) << (-8*sz);
				Chain2 = 0;
			}
			Slack[0] = ReverseByteOrder(Slack[0]);
			Slack[1] = ReverseByteOrder(Slack[1]);
			memcpy(pOutput,Slack,szSlack);
			pData += szSlack;
			pOutput += szSlack;
		}
	}

	void Decrypt(const CWBEncryptedBlock &EncryptedBlock,CAtlArray<BYTE> &Decrypted)
	{
		EncryptedBlock.m_CipherParams.CheckValid();
		if (EncryptedBlock.m_EncryptedData.GetCount() < 8)
			throw CPeaceBirdExceptionInvalidDataFormat();

		size_t szData = EncryptedBlock.m_EncryptedData.GetCount() - 8;
		if (!Decrypted.SetCount(szData))
			CAtlException(E_OUTOFMEMORY);
		//memset(Decrypted.GetData(),0xAB,szData);
		LPBYTE pOutput = Decrypted.GetData();
		const BYTE* pData = EncryptedBlock.m_EncryptedData.GetData();

		DWORD Chain1,Chain2,X1,X2;
		ProcessSlack(EncryptedBlock.m_CipherParams,szData,pData,pOutput,Chain1,Chain2,X1,X2);
		for (size_t d=0,Chunk8Count = szData / 8 ; d<Chunk8Count ; d++,pData+=8,pOutput+=8)
		{
			DWORD BigEndianDW1 = ReverseByteOrder(*(PDWORD)pData);
			DWORD BigEndianDW2 = ReverseByteOrder(*((PDWORD)pData+1));
			DWORD DW1Xored = BigEndianDW1 ^ Chain1;
			DWORD DW2Xored = BigEndianDW2 ^ Chain2;

			ApplyTransformsDecrypt(EncryptedBlock.m_CipherParams,DW1Xored,DW2Xored);

			DWORD DecryptedDW1 = X1 ^ DW1Xored;
			DWORD DecryptedDW2 = X2 ^ DW2Xored;
			X1 = BigEndianDW1;
			X2 = BigEndianDW2;
			Chain1 = DecryptedDW1;
			Chain2 = DecryptedDW2;
			*(PDWORD)pOutput = ReverseByteOrder(DecryptedDW1);
			*((PDWORD)pOutput+1) = ReverseByteOrder(DecryptedDW2);
		}

		BYTE XorAllBytes=0;
		for(size_t i=0;i<Decrypted.GetCount();i++)
			XorAllBytes ^= Decrypted[i];
		BYTE Checksum = EncryptedBlock.m_EncryptedData[szData];

		if (Checksum!=XorAllBytes)
			throw CPeaceBirdExceptionChecksumError();
	}


	void Encrypt(const CAtlArray<BYTE> &Data,CWBEncryptedBlock &EncryptedBlock,const CWBCipherParams &CipherParams)
	{
		Encrypt(Data.GetData(),Data.GetCount(),EncryptedBlock,CipherParams);
	}
	void Encrypt(LPCVOID pData,size_t szData,CWBEncryptedBlock &EncryptedBlock,const CWBCipherParams &CipherParams)
	{
		CipherParams.CheckValid();
		if (!EncryptedBlock.m_EncryptedData.SetCount(szData+8)) 
			CAtlException(E_OUTOFMEMORY);
		//memset(EncryptedBlock.m_EncryptedData.GetData(),0xAB,EncryptedBlock.m_EncryptedData.GetCount());
		const BYTE* pData2 = (LPBYTE)pData;
		LPBYTE pOutput = EncryptedBlock.m_EncryptedData.GetData();

		EncryptedBlock.m_CipherParams = CipherParams;

		BYTE XorAllBytes=0;
		for(size_t i=0;i<szData;i++)
			XorAllBytes ^= pData2[i];

		DWORD Chain1,Chain2,X1,X2;
		ProcessSlack(EncryptedBlock.m_CipherParams,szData,pData2,pOutput,Chain1,Chain2,X1,X2);

		for (size_t d=0,Chunk8Count = szData / 8 ; d<Chunk8Count ; d++,pData2+=8,pOutput+=8)
		{
			DWORD BigEndianDW1 = ReverseByteOrder(*(PDWORD)pData2);
			DWORD BigEndianDW2 = ReverseByteOrder(*((PDWORD)pData2+1));
			DWORD DW1Xored = BigEndianDW1 ^ Chain1;
			DWORD DW2Xored = BigEndianDW2 ^ Chain2;

			ApplyTransformsEncrypt(EncryptedBlock.m_CipherParams,DW1Xored,DW2Xored);

			DWORD EncryptedDW1 = X1 ^ DW1Xored;
			DWORD EncryptedDW2 = X2 ^ DW2Xored;
			X1 = BigEndianDW1;
			X2 = BigEndianDW2;
			Chain1 = EncryptedDW1;
			Chain2 = EncryptedDW2;
			*(PDWORD)pOutput = ReverseByteOrder(EncryptedDW1);
			*((PDWORD)pOutput+1) = ReverseByteOrder(EncryptedDW2);
		}
		*pOutput = XorAllBytes;
		memset(pOutput+1,0,7);
	}

	NTSTATUS WarbirdQuery(const CChunkedMessage &cmq,CChunkedMessage &cmr)
	{
		CChunkedMessage cmqq;
		CAtlArray<BYTE> data,data2;
		CWBEncryptedBlock encblock;
		CWBCipherParams cp;
		const void *p;
		size_t sz;

		cp.SetNullTransform();
		cmqq.PrepareQuery(0,cp);
		cmqq.m_Chunks.Append(cmq.m_Chunks);
		cmqq.Pack(data);

		Encrypt(data,encblock,cp);
		encblock.Pack(data);

		if (!data2.SetCount(4000)) // should be enough
			throw CAtlException(E_OUTOFMEMORY);
		SPQ spq;
		spq.pQuery = data.GetData();
		spq.szQuery = (DWORD)data.GetCount();
		spq.pResultBuf = data2.GetData();
		spq.szResultBuf = (DWORD)data2.GetCount();
		spq.Unk0 = 0;
		t_NtSetSystemInformation f_NtSetSystemInformation = (t_NtSetSystemInformation)GetProcAddress(GetModuleHandle(_T("ntdll.dll")),"NtSetSystemInformation");
		NTSTATUS status = f_NtSetSystemInformation(134,&spq,sizeof(spq));
		if (status==0)
		{
			encblock.Parse(p=spq.pResultBuf,sz=spq.szResultBuf);
			Decrypt(encblock,data);
			cmr.Parse(p=data.GetData(),sz=data2.GetCount());
		}
		return status;
	}

	void PreparePolicyValueQuery(CChunkedMessage &cm,DWORD dwValueMaxLen,LPCWSTR lpValueName)
	{
		CChunk c;
		cm.m_Chunks.RemoveAll();
		c.FromWString(lpValueName); // Chunk 4 - value name
		cm.m_Chunks.AddChunk(c);
		c.SetDword(0); // Chunk 5 - dont know what it is
		cm.m_Chunks.AddChunk(c);
		c.SetDword(dwValueMaxLen); // Chunk 6 - maximum length of value data in bytes
		cm.m_Chunks.AddChunk(c);
	}
	void ParsePolicyValueQueryResponse(const CChunkedMessage &cm,CChunk &ValueData,DWORD &ValueType)
	{
		if (cm.m_Chunks.GetCount()<6)
			throw CPeaceBirdExceptionUnexpectedChunkStructure();
		ValueType = cm.m_Chunks[2].GetDword();
		ValueData = cm.m_Chunks[3];
	}
	NTSTATUS WarbirdQuery_PolicyValue(DWORD dwValueMaxLen,LPCWSTR lpValueName,CChunk &ValueData,DWORD &ValueType)
	{
		CChunkedMessage cmq,cmr;
		PreparePolicyValueQuery(cmq,dwValueMaxLen,lpValueName);
		NTSTATUS status = WarbirdQuery(cmq,cmr);
		if (status==0)
			ParsePolicyValueQueryResponse(cmr,ValueData,ValueType);
		return status;

	}

};
