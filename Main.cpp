#include <Windows.h>

#include <tchar.h>

#include "Config.h"

DWORD dwVariable = NULL;
DWORD dwTest = NULL;
DWORD dwSmallTest[3] = {NULL};
DWORD dwBigTest[5][4] = {{NULL}};
TCHAR cCharacter = _T('\0');
LPTSTR szString = NULL;

BYTE iTestKey = NULL;
BYTE iNoobKey = NULL;

VARIABLE Variables[] = {
  {_T("Variable"), &dwVariable, NULL, {NULL}},
  {_T("Test"), &dwTest, &iTestKey, {NULL}},
  {_T("Noob"), NULL, &iNoobKey, {NULL}},
  {_T("Small Test"), &dwSmallTest, NULL, {3}},
  {_T("Big Test"), &dwBigTest, NULL, {5, 4}},
  {_T("Character"), &cCharacter, NULL, {NULL}},
  {_T("String"), &szString, NULL, {NULL}}
};

BOOL _tmain(INT argc, LPTSTR* argv, LPTSTR* envp)
{
  CConfig Config;

  Config.Load(_T("Config.ini"), _T("Vocabulary.ini"));
  Config.LoadProfile(Variables, _countof(Variables));

  ::_tprintf_s(_T("\n\n"));

  ::_tprintf_s(_T("TEST1: %d\n"), dwVariable);
  ::_tprintf_s(_T("TEST1: %d\n"), dwSmallTest[1]);
  ::_tprintf_s(_T("TEST2: %d\n"), dwBigTest[1][2]);
  ::_tprintf_s(_T("TEST3: %c\n"), cCharacter);
  ::_tprintf_s(_T("TEST4: %s\n"), szString);

  return EXIT_SUCCESS;
}
