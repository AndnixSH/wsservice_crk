#pragma once

BOOL ConvertUTF16ToUTF8(const WCHAR * pszTextUTF16, CStringA &strUTF8);
BOOL ConvertUTF8ToUTF16(const CHAR * pszTextUTF8, CStringW &strUTF16);
