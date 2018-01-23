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

	static BOOL BeginProcess(string strCommand);

	static bool IsDirExisted(string strPath);

	static CDuiString M2W(LPCSTR lpstr);
	static string W2M(LPCWSTR lpstr);

	static std::string StringFromLPCTSTR(LPCTSTR str);

	static bool SaveLogToFile(const string path, const string log);
};

