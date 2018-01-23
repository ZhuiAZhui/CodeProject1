//#pragma once

#include "stdafx.h"
//#include <iostream>
//using namespace std;

class PublicFunction
{
public:
	PublicFunction();
	~PublicFunction();

	static string GetCurrentRunPath();

	/* ÷¥––œµÕ≥√¸¡Ó */
	static void ExecuteSysCmd(string cmd);

	static BOOL BeginProcess(string strCurDir, string strCommand);

	static bool IsDirExisted(CString strPath);

	static bool InitSystemFileInfo(CString strSystemPath, CString strSavePath);

	static CString M2W(LPCSTR lpstr);
	static string W2M(LPCWSTR lpstr);

	static bool SaveLogToFile(const string path, const string log);

	static int DecompressPackage(const string winRarPath, const string packagePath, string destpath);
};

