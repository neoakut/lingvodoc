#define UNICODE

#ifdef __linux__ 
#include <wchar.h>
#include <string.h>
#include <wctype.h>
#include <stdlib.h>
#include "unicode.h"
#include "strings.h"
//typedef LPTSTR string;
#else
#include "default.h"
//#include <wchar.h> пока не выходит
#include "windows.h"
#include "winnt.h"
#include <seh.h>
#include "gui_win32.h"
#endif

#include "stringfunctions.h"

#include "infotree.h"
#include "dictionary.h"
#include "cognates.h"

//летит
//TCHAR IPA[100] = TEXT("a, b");

#ifdef __linux__ 
extern "C" {int
#else
int __declspec(dllexport)
#endif
PhonemicAnalysis_GetAllOutput(LPTSTR bufIn, int nRows, LPTSTR bufOut, int)
{
	int szOutput;

	if (nRows == -1)
		szOutput = lstrlen(bufIn) * 50 + 100000;
	else
		szOutput = nRows * 250 + 100000;

	if (!bufIn)
		return szOutput;

	try
	{
		InfoTree trOut(L"ФОНЕТИЧЕСКИЙ АНАЛИЗ");
		Dictionary dic;

		dic.AddWordList(bufIn, nRows - 1);

		InfoTree trIPAVowels, trIPAConsonants, trIPANotFound;

		dic.BuildIPATable(FT_VOWEL, &trIPAVowels);
		dic.BuildIPATable(FT_CONSONANT, &trIPAConsonants);
		dic.BuildIPATable(FT_UNKNOWNSOUND, &trIPANotFound);

		trOut.AddSubtree(&trIPAConsonants, L"Согласные звуки", IT_COLUMN | IT_EMPTYLINEBEFORE);//, IT_TAB);
		trOut.AddSubtree(&trIPAVowels, L"Гласные звуки", IT_COLUMN | IT_EMPTYLINEBEFORE);//, IT_TAB);
		trOut.AddSubtree(&trIPANotFound, L"Неопознанные знаки", IT_COLUMN | IT_EMPTYLINEBEFORE, 0);

		InfoNode* ndDistr[FT_NSOUNDCLASSES];
		ndDistr[FT_VOWEL] = trOut.Add(L"Списки по гласному первого слога",/*NULL,*/ IT_COLUMN | IT_EMPTYLINEBEFORE);
		ndDistr[FT_CONSONANT] = trOut.Add(L"Списки по согласному перед гласным первого слога", /*NULL,*/ IT_COLUMN | IT_EMPTYLINEBEFORE);
		dic.BuildDistributionLists(ndDistr, &trOut);

		OutputString output(szOutput, 45); //Dictionary::OutputString катит!!!
		output.Build(trOut.ndRoot);

		lstrcpy(bufOut, output.bufOut);
		return 1;
	}
	catch (...)
	{
		return 2;
	}
}
#ifdef __linux__ 
}
#endif

#ifdef __linux__ 
extern "C" {int
#else
int __declspec(dllexport)
#endif
CognateAnalysis_GetAllOutput(LPTSTR bufIn, int nCols, int nRows, LPTSTR bufOut, int flags)
{
	if (nCols < 1 || nCols > 1000)
		return -1;

	int szOutput = nRows * nCols * 60 + 100000;

	if (!bufIn)
		return szOutput;

	try
	{
		bool isBinary = flags == 2;
		nRows -= 1;// потому что там ещё и заголовок

		Comparison cmp(nRows, nCols);
		LPTSTR title;
		if (!isBinary) title = L"ЭТИМОЛОГИЧЕСКИЙ АНАЛИЗ"; else title = NULL;
		InfoTree trOut(title);

		cmp.AddCognateList(bufIn);

		Query qry;
		qry.AddCondition(L"Г", L"#", NULL, 0, L"Соответствия по начальному гласному");
		qry.AddCondition(L"Г", L"С", NULL, QF_OBJECTONLYONCE, L"Соответствия по гласному после первого согласного");
		qry.AddCondition(L"С", L"#", NULL, 0, L"Соответствия по начальному согласному");

		if (!isBinary)
			cmp.OutputLanguageNames(&trOut);

		for (Condition* cnd = qry.FirstCondition(); cnd; cnd = qry.NextCondition())
		{
			cmp.Process(cnd);
			cmp.OutputCorresponcesWithMaterial(cnd, &trOut);
		}

		OutputString output(szOutput, 20, nCols * 2, isBinary);
		output.Build(trOut.ndRoot);

		if (isBinary)
			output.OutputTableSizes(bufOut);
		output.OutputData(bufOut);

		return output.OutputSize;
	}
	catch (...)
	{
		return -2;
	}
}
#ifdef __linux__ 
}
#endif


#ifdef __linux__ 
extern "C" {int
#else
int __declspec(dllexport)
#endif
CognateAcousticAnalysis_GetAllOutput(LPTSTR bufIn, int nCols, int nRows, LPTSTR bufOut, int flags)
{
	if (nCols < 1 || nCols > 1000)
		return -1;

	int szOutput = nRows * nCols * 60 + 100000;

	if (!bufIn)
		return szOutput;

	try
	{
		bool isBinary = flags == 2;
		nRows -= 1;// потому что там ещё и заголовок

		Comparison cmp(nRows, nCols);
		LPTSTR title;
		if (!isBinary) title = L"АКУСТИЧЕСКИЙ АНАЛИЗ ОТКЛОНЕНИЙ"; else title = NULL;
		InfoTree trOut(title);

		cmp.AddCognateList(bufIn);

		Query qry;
		qry.AddCondition(L"Г", L"#", NULL, 0, L"Отклонения по начальному гласному");
		qry.AddCondition(L"Г", L"С", NULL, QF_OBJECTONLYONCE, L"Отклонения по гласному после первого согласного");

		if (!isBinary)
			cmp.OutputLanguageNames(&trOut);

		for (Condition* cnd = qry.FirstCondition(); cnd; cnd = qry.NextCondition())
		{
			cmp.Process(cnd);
			cmp.OutputDeviationsWithMaterial(cnd, &trOut);
		}

		OutputString output(szOutput, 20, nCols * 2, isBinary);
		output.Build(trOut.ndRoot);

		if (isBinary)
			output.OutputTableSizes(bufOut);
		output.OutputData(bufOut);

		return output.OutputSize;
	}
	catch (...)
	{
		return -2;
	}
}
#ifdef __linux__ 
}
#endif






#ifdef __linux__ 
extern "C" {int
#else
int __declspec(dllexport)
#endif
Retranscribe(LPTSTR bufIn, LPTSTR bufOut, LPTSTR langIn, LPTSTR langOut, int flags)
{
	try
	{
		Dictionary dic;

		dic.ReplaceSymbols(bufIn, bufOut, dic.GuessReplacer(bufIn));


		//lstrcpy(bufOut, bufIn);
		return 1;
	}
	catch (...)
	{
		return -2;
	}
}
#ifdef __linux__ 
}
#endif




/*
BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
//	MsgBox(ipaVowels);
}
*/
