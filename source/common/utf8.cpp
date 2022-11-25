#include "stdafx.h"
#include <strsafe.h>
#include "utf8.h"


BOOL ConvertUTF8ToUTF16( const CHAR * pszTextUTF8, CStringW &strUTF16)
{
    strUTF16.Empty();

    //
    // Special case of NULL or empty input string
    //
    if ( (pszTextUTF8 == NULL) || (*pszTextUTF8 == '\0') )
		return TRUE;


    //
    // Consider CHAR's count corresponding to total input string length,
    // including end-of-string (\0) character
    //

    const size_t cchUTF8Max = INT_MAX - 1;
    size_t cchUTF8;
    HRESULT hr = ::StringCchLengthA( pszTextUTF8, cchUTF8Max, &cchUTF8 );
    if ( FAILED( hr ) )
	{
		SetLastError(ERROR_INVALID_DATA);
        return FALSE;
	}

    // Consider also terminating \0
    ++cchUTF8;

    // Convert to 'int' for use with MultiByteToWideChar API
    int cbUTF8 = static_cast<int>( cchUTF8 );

    //
    // Get size of destination UTF-16 buffer, in WCHAR's
    //
    int cchUTF16 = ::MultiByteToWideChar(
        CP_UTF8,                // convert from UTF-8
        MB_ERR_INVALID_CHARS,   // error on invalid chars
        pszTextUTF8,            // source UTF-8 string
        cbUTF8,                 // total length of source UTF-8 string,
                                // in CHAR's (= bytes), including end-of-string \0
        NULL,                   // unused - no conversion done in this step
        0                       // request size of destination buffer, in WCHAR's
        );

    if ( cchUTF16 == 0 )
        return FALSE;
 
    WCHAR * pszUTF16 = strUTF16.GetBuffer( cchUTF16 );

    //
    // Do the conversion from UTF-8 to UTF-16
    //
    int result = ::MultiByteToWideChar(
        CP_UTF8,                // convert from UTF-8
        MB_ERR_INVALID_CHARS,   // error on invalid chars
        pszTextUTF8,            // source UTF-8 string
        cbUTF8,                 // total length of source UTF-8 string,
                                // in CHAR's (= bytes), including end-of-string \0
        pszUTF16,               // destination buffer
        cchUTF16                // size of destination buffer, in WCHAR's
        );

    if ( result == 0 )
	{
        strUTF16.Empty();
		return FALSE;
	}

    strUTF16.ReleaseBuffer();
    return TRUE;
}

 

BOOL ConvertUTF16ToUTF8(const WCHAR * pszTextUTF16, CStringA &strUTF8)
{
	strUTF8.Empty();

    //
    // Special case of NULL or empty input string
    //
    if ( (pszTextUTF16 == NULL) || (*pszTextUTF16 == L'\0') )
        return TRUE;

 
    //
    // Consider WCHAR's count corresponding to total input string length,
    // including end-of-string (L'\0') character.
    //

    const size_t cchUTF16Max = INT_MAX - 1;
    size_t cchUTF16;
    HRESULT hr = ::StringCchLengthW( pszTextUTF16, cchUTF16Max, &cchUTF16 );
    if ( FAILED( hr ) )
	{
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
 
    // Consider also terminating \0
    ++cchUTF16;


    //
    // WC_ERR_INVALID_CHARS flag is set to fail if invalid input character
    // is encountered.
    // This flag is supported on Windows Vista and later.
    // Don't use it on Windows XP and previous.
    //

#if (WINVER >= 0x0600)
    DWORD dwConversionFlags = WC_ERR_INVALID_CHARS;
#else
    DWORD dwConversionFlags = 0;
#endif

    //
    // Get size of destination UTF-8 buffer, in CHAR's (= bytes)
    //

    int cbUTF8 = ::WideCharToMultiByte(
        CP_UTF8,                // convert to UTF-8
        dwConversionFlags,      // specify conversion behavior
        pszTextUTF16,           // source UTF-16 string
        static_cast<int>( cchUTF16 ),   // total source string length, in WCHAR's,
                                        // including end-of-string \0
        NULL,                   // unused - no conversion required in this step
        0,                      // request buffer size
        NULL, NULL              // unused
        );

    if ( cbUTF8 == 0 )
		return FALSE;

    //
    // Allocate destination buffer for UTF-8 string
    //

    int cchUTF8 = cbUTF8; // sizeof(CHAR) = 1 byte
    CHAR * pszUTF8 = strUTF8.GetBuffer( cchUTF8 );
 

    //
    // Do the conversion from UTF-16 to UTF-8
    //
    int result = ::WideCharToMultiByte(
        CP_UTF8,                // convert to UTF-8
        dwConversionFlags,      // specify conversion behavior
        pszTextUTF16,           // source UTF-16 string
        static_cast<int>( cchUTF16 ),   // total source string length, in WCHAR's,
                                        // including end-of-string \0
        pszUTF8,                // destination buffer
        cbUTF8,                 // destination buffer size, in bytes
        NULL, NULL              // unused
        ); 


    if ( result == 0 )
	{
		strUTF8.Empty();
		return FALSE;
	}

    strUTF8.ReleaseBuffer();
    return TRUE;

}

 

 