//#include "stdafx.h"
#include "PublicFunction.h"
#include <Windows.h>
#include <AtlConv.h>
#include <fstream>


struct FILEINFO
{
	char szFileName[100];
	char szFileType[20];
	char szFileTime[30];
	ULONGLONG ulLength;
};


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

BOOL PublicFunction::BeginProcess(string strCurDir, string strCommand)
{
	PROCESS_INFORMATION pi;
	STARTUPINFOA si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW;
	BOOL bRet = CreateProcessA(strCommand.c_str(), NULL, NULL, FALSE, NULL,
		NULL, NULL, strCurDir.c_str(), &si, &pi);
	return bRet;
}

// 存在 -- true； 不存在 -- false
bool PublicFunction::IsDirExisted(CString strPath)
{
	DWORD dwAttr = GetFileAttributes(strPath);
	if (INVALID_FILE_ATTRIBUTES == dwAttr ||  !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		return false;
	}
	return true;
}



bool PublicFunction::InitSystemFileInfo(CString strSystemPath, CString strSavePath)
{
	WIN32_FIND_DATA data;
	TCHAR szFind[MAX_PATH] = { 0 };

	_tcscpy_s(szFind, MAX_PATH, strSystemPath);
	_tcscat_s(szFind, _T("\\*.*"));

	FILEINFO *fileinfo = NULL;
	fileinfo = (struct FILEINFO*)malloc(sizeof(FILEINFO));
	if (fileinfo == NULL)
	{
		return false;
	}

	try
	{
		memset(fileinfo, 0, sizeof(FILEINFO));

		HANDLE handle = ::FindFirstFile(szFind, &data);
		if (handle == INVALID_HANDLE_VALUE)
		{
			free(fileinfo);
			return false;
		}

		FILE *pFile = NULL;
		pFile = fopen(W2M(strSavePath).c_str(), "wb");
		if (pFile == NULL)
		{
			free(fileinfo);
			return false;
		}

		do
		{
			string strFileType = "文件";
			CString strFileName = _T("");
			CString strLastDate = _T("");
			ULONGLONG ulLength = 0;

			//FileInfo fileinfo;
			if (data.cFileName[0] != _T('.'))
			{
				strFileName = data.cFileName;
				//_tprintf(_T("%s\\%s\n"), )
				if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					// 文件夹
					strFileType = "文件夹";
				}
				else
				{
					strFileType = "文件";
					ulLength = (data.nFileSizeHigh * (MAXDWORD + 1)) + data.nFileSizeLow;
					fileinfo->ulLength = ulLength;
				}
				FILETIME ft = data.ftLastWriteTime;
				SYSTEMTIME st;
				FileTimeToSystemTime(&ft, &st);
				strLastDate.Format(_T("%04d-%02d-%02d %02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

				strcpy_s(fileinfo->szFileName, W2M(strFileName).c_str());
				strcpy_s(fileinfo->szFileType, strFileType.c_str());
				strcpy_s(fileinfo->szFileTime, W2M(strLastDate).c_str());

				// write to file
				fwrite(fileinfo, sizeof(FILEINFO), 1, pFile);
			}

		} while (::FindNextFile(handle, &data) != 0 && ::GetLastError() != ERROR_NO_MORE_FILES);
		FindClose(handle);
		fclose(pFile);
	}
	catch (CException *e)
	{
		free(fileinfo);
		e->ReportError();
		return false;
	}

	free(fileinfo);
	return true;
}

CString PublicFunction::M2W(LPCSTR lpstr)
{
	USES_CONVERSION;
	return A2W(lpstr);
}

string PublicFunction::W2M(LPCWSTR lpstr)
{
	USES_CONVERSION;
	return W2A(lpstr);
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

/* 解压压缩包至指定目录
	winRarPath	--- 解压工具的路径
	packagePath --- 压缩包的路径
	destpath	--- 解压至的指定路径 
	解压命令：x -ibck -o+ -inul*/
int PublicFunction::DecompressPackage(const string winRarPath, const string packagePath, 
	string destpath)
{
	if (winRarPath.size() == 0 || packagePath.size() == 0
		|| destpath.size() == 0)
	{
		return -1;
	}

	// 解压存放目录，必须加'\'
	if (destpath.find_last_of('\\') != destpath.size() - 1)
	{
		destpath.append("\\");
	}

	string strCommand = "";
	strCommand = winRarPath + " x -ep -ibck -o+ -inul " + packagePath + " " + destpath;

	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	BOOL bRet = CreateProcessA(NULL, (char*)strCommand.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	DWORD dwExit = -1;
	if (bRet)
	{
		//这个地方将导致该函数为阻塞状态  
		WaitForSingleObject(pi.hProcess, INFINITE);
		::GetExitCodeProcess(pi.hProcess, &dwExit);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	return dwExit;
}
