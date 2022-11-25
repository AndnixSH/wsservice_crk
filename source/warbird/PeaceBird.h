#pragma once

#include <atlcoll.h>
#include <atlstr.h>

typedef LONG NTSTATUS;

// Peaceful warbird cryptor-decryptor
// WARBIRD is obfuscration system used in win8/2012 to deal with software protection entities
// WARBIRD user mode code construct encrypted query, pass it to NtSetSystemInformation
// with InformationClass = 134 (0x86)
// request is processed by the kernel and returned as encrypted result buffer
// kernel has generic encryption/decryption algorithm while user mode callers
// use very huge template-generated and obfuscrated cryptors with only a subset of all possible
// crypto transforms. user mode caller provide kernel with all necessary information
// about utilized crypto transoforms

namespace PeaceBird
{
	typedef NTSTATUS (WINAPI *t_NtSetSystemInformation)
		(IN DWORD SystemInformationClass,
		 IN PVOID SystemInformation,
		 IN ULONG SystemInformationLength);
	// SystemInformation data structure used in NtSetSystemInformation(134)
	typedef struct
	{
		PVOID pQuery,pResultBuf;
		DWORD szQuery,szResultBuf;
		DWORD Unk0; // always 0
	} SPQ,*PSPQ;

	// when calling PeaceBird functions
	// catch std::bad_alloc,CAtlException,CPeaceBirdException

	class CPeaceBirdException
	{
	public:
		CPeaceBirdException(CString msg) {m_message = msg;}
		CString GetMessage() const {return m_message;}
		void AddMessage(LPCTSTR msg) {m_message = msg + m_message;}
	protected:
		CString m_message;
	};
	class CPeaceBirdExceptionInvalidDataFormat : public CPeaceBirdException
	{
	public:
		CPeaceBirdExceptionInvalidDataFormat() : CPeaceBirdException(_T("invalid data format")) {};
	};
	class CPeaceBirdExceptionUnexpectedChunkStructure : public CPeaceBirdException
	{
	public:
		CPeaceBirdExceptionUnexpectedChunkStructure() : CPeaceBirdException(_T("unexpected chunk structure")) {};
	};
	class CPeaceBirdExceptionChecksumError : public CPeaceBirdException
	{
	public:
		CPeaceBirdExceptionChecksumError() : CPeaceBirdException(_T("invalid checksum")) {};
	};
	class CPeaceBirdExceptionCipherParamsError : public CPeaceBirdException
	{
	public:
		CPeaceBirdExceptionCipherParamsError() : CPeaceBirdException(_T("invalid cipher params")) {};
	};

	class CChunk : public CAtlArray<BYTE>
	{
	public:
		void ParseAppend(LPCVOID &pBuf,size_t &sz);
		void Parse(LPCVOID &pBuf,size_t &sz);
		void PackAppend(CAtlArray<BYTE> &target) const;
		void Pack(CAtlArray<BYTE> &target) const;
		void operator=(const CChunk &p);

		DWORD GetPackedSize() const;
		BOOL IsDword() const;
		DWORD GetDword() const;
		void SetDword(DWORD dw);
		CStringW ToWString() const;
		void FromWString(LPCWSTR ws);
	protected:
		static DWORD ChunkSize(LPCVOID pChunk,size_t szChunkBuf);
		static void UnpackChunk(LPCVOID &pChunk,size_t &szChunkBuf,DWORD szChunkExpected,PVOID pDest);
	};

	class CChunkCollection : public CAtlArray<CChunk>
	{
	public:
		// returns count of successfully parsed chunks
		// if MaxChunks = -1 - infinite chunks
		int ParseAppend(LPCVOID &pBuf,size_t &sz,int MaxChunks);
		int Parse(LPCVOID &pBuf,size_t &sz,int MaxChunks);
		void PackAppend(CAtlArray<BYTE> &target) const;
		void Pack(CAtlArray<BYTE> &target) const;
		void AddChunk(const CChunk &c);
		DWORD GetPackedSize() const;
	};
	class CWBCipherParams;
	class CChunkedMessage
	{
	public:
		void Parse(LPCVOID &pBuf,size_t &sz);
		void PackAppend(CAtlArray<BYTE> &target) const;
		void Pack(CAtlArray<BYTE> &target) const;
		DWORD GetPackedSize() const;
		// add 4 chunks : CommandID,CipherParams (2 chunks),rdtsc
		void PrepareQuery(DWORD CommandID,const CWBCipherParams &cp);

		CChunkCollection	m_Chunks;
	};

	class CWBCipherParams
	{
	public:
		CWBCipherParams();
		void Parse(LPCVOID &pBuf,size_t &sz);
		void PackAppend(CAtlArray<BYTE> &target) const;
		void Pack(CAtlArray<BYTE> &target) const;
		void operator=(const CWBCipherParams &p);
		void FromChunkedMessage(const CChunkedMessage &cmsg);
		void SetNullTransform();
		void CheckValid() const;
		struct SCiph2
		{
			BYTE X[128];	// CryptKeyData
			BYTE A[32];		// Algo IDs
		};
		SCiph2 *Ciph2;
		CChunk cCiph2,Ciph3;
	};

	class CWBEncryptedBlock
	{
	public:
		void Parse(LPCVOID &pBuf,size_t &sz);
		void PackAppend(CAtlArray<BYTE> &buf) const;
		void Pack(CAtlArray<BYTE> &buf) const;

		CWBCipherParams m_CipherParams;
		CChunk m_EncryptedData;
	};

	void Decrypt(const CWBEncryptedBlock &EncryptedBlock,CAtlArray<BYTE> &Decrypted);
	void Encrypt(LPCVOID pData,size_t szData,CWBEncryptedBlock &EncryptedBlock,const CWBCipherParams &CipherParams);
	void Encrypt(const CAtlArray<BYTE> &Data,CWBEncryptedBlock &EncryptedBlock,const CWBCipherParams &CipherParams);

	// warbird query with null cipher. can be executed from any process.
	// cmq must contain only chunks starting from 4
	// chunks 0,1,2,3 standard for all commands and automatically
	// filled inside the function
	// if status=0 cmr receives upnacked response data
	// dont forget to catch PeaceBird exceptions !
	NTSTATUS WarbirdQuery(const CChunkedMessage &cmq,CChunkedMessage &cmr);

	void PreparePolicyValueQuery(CChunkedMessage &cm,DWORD dwValueMaxLen,LPCWSTR lpValueName);
	void ParsePolicyValueQueryResponse(const CChunkedMessage &cm,CChunk &ValueData,DWORD &ValueType);
	NTSTATUS WarbirdQuery_PolicyValue(DWORD dwValueMaxLen,LPCWSTR lpValueName,CChunk &ValueData,DWORD &ValueType);
};

#define PB_CHECKED_CALL(CODE,AdditionalErrorString) \
try {##CODE;} \
catch (::PeaceBird::CPeaceBirdException &pbe) \
{ \
	pbe.AddMessage(_T(" : ")); \
	pbe.AddMessage(AdditionalErrorString); \
	throw; \
}

#define PPV_TYPE_NONE 0
#define PPV_TYPE_SZ 1
#define PPV_TYPE_BINARY 3
#define PPV_TYPE_DWORD 4
#define PPV_TYPE_MULTI_SZ 7
