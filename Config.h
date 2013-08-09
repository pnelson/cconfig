#ifndef _CCONFIG_H
#define _CCONFIG_H

#include <Windows.h>

#include <Shlwapi.h>
#include <stdio.h>
#include <tchar.h>

#pragma comment(lib, "shlwapi.lib")

#define MAX_PARAMETERS    4

#define VARIABLE_SIZE     128
#define PARAMETER_SIZE    64
#define VALUE_SIZE        64

typedef struct Variable_t
{
  LPTSTR szName;
  LPVOID pBuffer;
  LPBYTE iToggle;
  UINT nSize[MAX_PARAMETERS];
} VARIABLE, *LPVARIABLE;

class CConfig
{
public:
  CConfig()
  {
    ::memset(m_szProfilePath, NULL, sizeof(m_szProfilePath));
    ::memset(m_szVocabularyPath, NULL, sizeof(m_szProfilePath));
  }

  virtual ~CConfig()
  {
    //
  }

  BOOL Load(LPTSTR szProfile, LPTSTR szVocabulary)
  {
    TCHAR szDirectory[MAX_PATH] = _T("");
    
    if (!szProfile || !GetModuleDirectory(szDirectory, MAX_PATH))
      return FALSE;

    ::_stprintf_s(m_szProfilePath, MAX_PATH, _T("%s\\%s"), szDirectory, szProfile);
    ::_stprintf_s(m_szVocabularyPath, MAX_PATH, _T("%s\\%s"), szDirectory, szVocabulary);

    return TRUE;
  }

  DWORD GetVocabularyDefinition(LPTSTR szVocabulary)
  {
    TCHAR szLine[512] = _T("");
    DWORD dwResult = NULL;

    if (::_tfopen_s(&m_pVocabulary, m_szVocabularyPath, _T("r")) != NULL)
      return NULL;

    while (!::feof(m_pVocabulary))
    {
      if (ReadNextLine(m_pVocabulary, szLine, _countof(szLine)) < 11)
        continue;

      for (UINT i = 0; i < (UINT)::_tcslen(szLine); i++)
        if (::_istspace(szLine[i]))
          szLine[i] = _T(' ');

      LPTSTR szDirective = ::_tcstok(szLine, _T(" "));
      LPTSTR szName = ::_tcstok(NULL, _T(" "));
      LPTSTR szValue = ::_tcstok(NULL, _T(" "));

      if (!szDirective || !szName || !szValue)
        continue;

      if (!::_tcscmp(szDirective, _T("#define")) && !::_tcscmp(szVocabulary, szName))
      {
        dwResult = ::_tcstoul(szValue, NULL, NULL);

        break;
      }
    }

    ::fclose(m_pVocabulary);

    return dwResult;
  }

  BOOL LoadProfile(LPVARIABLE pVariable, DWORD dwSize)
  {
    TCHAR szLine[512] = _T("");

    if (::_tfopen_s(&m_pProfile, m_szProfilePath, _T("r")) != NULL)
      return FALSE;

    while (!::feof(m_pProfile))
    {
      TCHAR szVariable[VARIABLE_SIZE] = _T("");
      TCHAR szParameter[MAX_PARAMETERS][PARAMETER_SIZE] = {_T("")};
      TCHAR szValue[2][VALUE_SIZE] = {_T("")};

      BOOL bVariable = FALSE;

      UINT nParameter = 0;
      UINT nValue = 0;

      UINT nCursor = 0;

      ReadNextLine(m_pProfile, szLine, _countof(szLine));

      if (::_tcslen(szLine) < 2 || ::_istspace(szLine[0]) || isEscapeChar(szLine[0]) || !::_tcsncmp(szLine, _T("//"), 2))
        continue;

      for (UINT i = 0; i < (UINT)::_tcslen(szLine); i++)
      {
        if (!::_tcsncmp(&szLine[i], _T("//"), 2))
          break;

        if (!bVariable)
        {
          if (szLine[i] != _T('[') && szLine[i] != _T(':'))
          {
            if (nCursor == VARIABLE_SIZE - 1)
              return FALSE;

            szVariable[nCursor] = szLine[i];
            nCursor++;

            continue;
          }

          bVariable = TRUE;
        }

        if (szLine[i] == _T('['))
        {
          nCursor = 0;

          for (UINT j = i + 1; j < (UINT)::_tcslen(szLine); j++)
          {
            if (szLine[j] == _T(']'))
              break;

            if (nCursor == PARAMETER_SIZE - 1)
              return FALSE;

            szParameter[nParameter][nCursor] = szLine[j];
            nCursor++;
          }

          nParameter++;

          if (nParameter == MAX_PARAMETERS)
            while (++i < (UINT)::_tcslen(szLine) && szLine[i] != _T(':'));
        }

        if (szLine[i] == _T(':'))
        {
          nCursor = 0;

          while (++i < (UINT)::_tcslen(szLine) && _istspace(szLine[i]));

          for (UINT j = i; j < (UINT)::_tcslen(szLine); j++)
          {
            if (szLine[j] == _T(','))
            {
              if (nValue > 0)
                break;

              nValue++;
              nCursor = 0;

              while (++j < (UINT)::_tcslen(szLine) && _istspace(szLine[j]));
            }

            if (nCursor == VALUE_SIZE - 1)
              return FALSE;

            szValue[nValue][nCursor] = szLine[j];
            nCursor++;
          }
        }
      }

      ::_tprintf_s(_T("%s[%s][%s][%s][%s]: %s and %s\n"), szVariable, szParameter[0], szParameter[1], szParameter[2], szParameter[3], szValue[0], szValue[1]);

      for (UINT i = 0; i < dwSize; i++)
      {
        DWORD dwParameter[MAX_PARAMETERS] = {NULL};

        if (::_tcsicmp(szVariable, pVariable[i].szName) != 0)
          continue;

        for (UINT j = 0; j < MAX_PARAMETERS; j++)
        {
          if (pVariable[i].nSize[j])
          {
            dwParameter[j] = ::_tcstoul(szParameter[j], NULL, NULL);

            if (!dwParameter[j])
              dwParameter[j] = GetVocabularyDefinition(szParameter[j]);

            ::_tprintf_s(_T("\t%d\n"), dwParameter[j]);
          }
        }

        if (pVariable[i].pBuffer)
        {
          TCHAR cFirst = szValue[0][0];
          TCHAR cLast = szValue[0][::_tcslen(szValue[0]) - 1];

          if (cFirst == _T('\'') && cLast == _T('\''))
          {
            if (::_tcslen(szValue[0]) == 3)
              *(LPTSTR)pVariable[i].pBuffer = szValue[0][1];
          }
          else if (cFirst == _T('\"') && cLast == _T('\"'))
          {
            LPTSTR* szBuffer = (LPTSTR*)pVariable[i].pBuffer;
            UINT nLength = (UINT)::_tcslen(szValue[0]) - 1;

            if (*szBuffer)
              delete [] *szBuffer;

            szValue[0][::_tcslen(szValue[0]) - 1] = _T('\0');

            *szBuffer = new TCHAR[nLength];
            ::_tcscpy_s(*szBuffer, nLength, szValue[0] + 1);
          }
          else
          {
            DWORD dwValue = ::_tcstoul(szValue[0], NULL, NULL);

            if (!dwValue)
              dwValue = GetVocabularyDefinition(szValue[0]);

            if (!nParameter)
              *(LPDWORD)pVariable[i].pBuffer = dwValue;
            else
            {
              LPDWORD pCursor = (LPDWORD)pVariable[i].pBuffer;

              for (UINT j = 0; j < nParameter; j++)
              {
                DWORD dwOffset = 0;

                for (UINT k = j + 1; k < nParameter; k++)
                  dwOffset += pVariable[i].nSize[k];

                pCursor += dwOffset ? dwOffset * dwParameter[j] : dwParameter[j];
              }

              *pCursor = dwValue;
            }
          }
        }

        if (pVariable[i].iToggle)
        {
          LPTSTR szToggle = pVariable[i].pBuffer ? szValue[1] : szValue[0];

          *pVariable[i].iToggle = ::_tcstoul(szToggle, NULL, NULL) & 0xFF;

          if (!*pVariable[i].iToggle)
            *pVariable[i].iToggle = GetVocabularyDefinition(szToggle) & 0xFF;

          ::_tprintf_s(_T("\tHotkey: %d\n"), *pVariable[i].iToggle);
        }
      }
    }

    ::fclose(m_pProfile);

    return TRUE;
  }

protected:
  BOOL GetModuleDirectory(LPTSTR szBuffer, DWORD dwSize)
  {
    ::GetModuleFileName(NULL, szBuffer, dwSize);
    ::PathRemoveFileSpec(szBuffer);

    return (BOOL)::_tcslen(szBuffer);
  }

  UINT ReadNextLine(FILE* pFile, LPTSTR szLine, DWORD dwSize)
  {
    if (!pFile || !szLine)
      return NULL;

    ::memset(szLine, NULL, sizeof(TCHAR) * dwSize);
    ::_fgetts(szLine, dwSize, pFile);

    if (szLine[::_tcslen(szLine) - 1] == _T('\n'))
      szLine[::_tcslen(szLine) - 1] = _T('\0');

    return (UINT)::_tcslen(szLine);
  }

  BOOL isEscapeChar(TCHAR cChar)
  {
    if (cChar == _T('\0') || cChar == _T('\r') || cChar == _T('\n'))
      return TRUE;

    return FALSE; 
  }

private:
  FILE* m_pProfile;
  FILE* m_pVocabulary;

  TCHAR m_szProfilePath[MAX_PATH];
  TCHAR m_szVocabularyPath[MAX_PATH];
};

#endif
