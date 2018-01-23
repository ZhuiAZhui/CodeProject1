//#include "stdafx.h"
#include "PublicFunction.h"
#include <Windows.h>
#include <AtlConv.h>

PublicFunction::PublicFunction()
{
}


PublicFunction::~PublicFunction()
{
}

string PublicFunction::GetCurrentRunPath()
{
	string strPath = "";
	char szPath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szPath, MAX_PATH);

	if (FALSE != PathRemoveFileSpecA(szPath))
	{
		return string(szPath);
	}

	return string("");
}

void PublicFunction::ExecuteSysCmd(string cmd)
{
	if (cmd == "")
	{
		return;
	}
	system(cmd.c_str());
}

BOOL PublicFunction::BeginProcess(string strCommand)
{
	PROCESS_INFORMATION pi;
	STARTUPINFOA si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW;
	BOOL bRet = CreateProcessA(strCommand.c_str(), NULL, NULL, FALSE, NULL,
		NULL, NULL, NULL, &si, &pi);
	return bRet;
}

// 存在 -- true； 不存在 -- false
bool PublicFunction::IsDirExisted(string strPath)
{
	DWORD dwAttr = GetFileAttributesA(strPath.c_str());
	if (INVALID_FILE_ATTRIBUTES == dwAttr ||  !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		return false;
	}
	return true;
}

CDuiString PublicFunction::M2W(LPCSTR lpstr)
{
	int iLen = MultiByteToWideChar(CP_ACP, 0, lpstr, -1, NULL, 0) + 1 ;
	WCHAR *pUnicode = NULL;
	pUnicode = new WCHAR[iLen];

	memset(pUnicode, 0, (iLen)*sizeof(WCHAR));

	MultiByteToWideChar(CP_ACP, CP_ACP, lpstr, -1, (LPWSTR)pUnicode, iLen);

	CDuiString return_string = pUnicode;

	if (pUnicode != NULL)
	{
		delete[] pUnicode;
		pUnicode = NULL;
	}
	return return_string;
	//USES_CONVERSION;
	//return A2W(lpstr);
}

string PublicFunction::W2M(LPCWSTR lpstr)
{
#ifdef _UNICODE
	int iLen = WideCharToMultiByte(CP_ACP, 0, lpstr, -1, 0, 0, NULL, NULL) + 1;

	char* pStr = new char[iLen];
	memset((void*)pStr, 0, sizeof(char) * iLen);

	WideCharToMultiByte(CP_ACP, 0, lpstr, -1, pStr, iLen, NULL, NULL);


	std::string return_string(pStr);

	if (pStr != NULL)
	{
		delete[] pStr;
		pStr = NULL;
	}
	return return_string;
#else
	return std::string(str);
#endif
	//USES_CONVERSION;
	//return W2A(lpstr);
}

std::string PublicFunction::StringFromLPCTSTR(LPCTSTR str) 
{
#ifdef _UNICODE
	int size_str = WideCharToMultiByte(CP_ACP, 0, str, -1, 0, 0, NULL, NULL);

	char* point_new_array = new char[size_str+1];
	memset((void*)point_new_array, 0, sizeof(char) * (size_str + 1));

	WideCharToMultiByte(CP_ACP, 0, str, -1, point_new_array, size_str, NULL, NULL);
	point_new_array[size_str] = '\0';
	std::string return_string(point_new_array);
	delete[] point_new_array;
	point_new_array = NULL;
	return return_string;
#else
	return std::string(str);
#endif
}

bool PublicFunction::SaveLogToFile(const string path, const string log)
{
	if (path.size() == 0 || log.size() == 0)
	{
		return false;
	}

	FILE *pFile = NULL;
	errno_t err = fopen_s(&pFile, path.c_str(), "ab");
	if (err != 0 || pFile == NULL)
	{
		return false;
	}

	// 文件指针设置为文件尾
	fseek(pFile, 0, SEEK_END);

	if (log.size() > fwrite(log.c_str(), 1, log.size(), pFile))
	{
		fclose(pFile);
		return false;
	}

	fclose(pFile);
	return true;
}