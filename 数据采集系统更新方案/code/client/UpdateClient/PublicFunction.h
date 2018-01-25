//#pragma once

#include "stdafx.h"
#include "tinyxml.h"

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

	static bool QueryFileValue(const std::string ValueName, const std::string ModuleName,
		std::string &Value);

	static bool GetFileVersion(const std::string ModuleName, std::string &Value);

	static std::string GetFileMD5(const std::string& filename);

	static void TraverseFolder(const std::string path, TiXmlElement *element, vector<string> excludeFiles);

	static bool InitFolderToXml(const std::string dirpath, const std::string xmlpath, vector<string> excludeFiles);
};

