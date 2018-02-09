//#include "stdafx.h"
#include "PublicFunction.h"
#include <Windows.h>
#include <AtlConv.h>

#include <fstream>
#include <sstream>
#include <memory>
#include <iomanip>
#include "md5global.h"
#include "md5.h"

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
	int iLen = WideCharToMultiByte(CP_ACP, 0, lpstr, wcslen(lpstr), 0, 0, NULL, NULL) + 1;

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


// 查询文件的某个信息，例如：文件说明，文件版本，产品名称，原始文件名等
bool PublicFunction::QueryFileValue(const std::string ValueName, const std::string ModuleName, std::string &Value)
{
	bool bRet = false;

	if (!ValueName.size() || !ModuleName.size())
	{
		return false;
	}

	BYTE *bpVersionData = NULL;
	DWORD dwLangCharset = 0;
	char *tmpstr = NULL;

	do
	{
		DWORD dwHandle = 0;
		DWORD dwDataSize = ::GetFileVersionInfoSizeA(ModuleName.c_str(), &dwHandle);
		if (dwDataSize == 0)
		{
			break;
		}

		bpVersionData = new BYTE[dwDataSize];
		if (bpVersionData == NULL) break;
		//检索信息，从支持版本标记的一波模块中获取文件版本信息
		if (!::GetFileVersionInfoA(ModuleName.c_str(), dwHandle,
			dwDataSize, bpVersionData))
		{
			break;
		}

		UINT uiQuerySize = 0;
		DWORD *pdwTransTable = NULL;
		// 设置语言
		if (!::VerQueryValueA(bpVersionData, "\\VarFileInfo\\Translation",
			(void**)&pdwTransTable, &uiQuerySize))
		{
			break;
		}
		dwLangCharset = MAKELONG(HIWORD(pdwTransTable[0]), LOWORD(pdwTransTable[0]));
		if (bpVersionData == NULL) break;

		tmpstr = new char[128];
		if (tmpstr == NULL) break;
		sprintf_s(tmpstr, 128, "\\StringFileInfo\\%08lx\\%s", dwLangCharset, ValueName.c_str());
		
		LPVOID lpData;

		// 调用此函数查询前需要先依次调用函数GetFileVersionInfoSize和GetFileVersionInfo  
		if (::VerQueryValueA((void *)bpVersionData, tmpstr, &lpData, &uiQuerySize))
		{
			Value = (char*)lpData;
			bRet = true;
		}
	} while (FALSE);

	if (bpVersionData)
	{
		delete[] bpVersionData;
		bpVersionData = NULL;
	}

	if (tmpstr)
	{
		delete[] tmpstr;
		tmpstr = NULL;
	}

	return bRet;
}


bool PublicFunction::GetFileVersion(const std::string ModuleName, std::string & Value)
{
	return QueryFileValue("FileVersion", ModuleName, Value);
}

std::string PublicFunction::GetFileMD5(const std::string& filename)
{
	if (!PathFileExistsA(filename.c_str()))
	{
		return "";
	}

	std::ifstream fin(filename.c_str(), std::ifstream::in | std::ifstream::binary);
	if (fin)
	{
		MD5_CTX context;
		MD5Init(&context);

		fin.seekg(0, fin.end);
		const auto fileLength = fin.tellg();
		fin.seekg(0, fin.beg);

		const int bufferLen = 8192;
		std::unique_ptr<unsigned char[]> buffer{ new unsigned char[bufferLen] {} };
		unsigned long long totalReadCount = 0;
		decltype(fin.gcount()) readCount = 0;
		// 读取文件内容，调用MD5Update()更新MD5值
		while (fin.read(reinterpret_cast<char*>(buffer.get()), bufferLen))
		{
			readCount = fin.gcount();
			totalReadCount += readCount;
			MD5Update(&context, buffer.get(), static_cast<unsigned int>(readCount));
		}
		// 处理最后一次读到的数据
		readCount = fin.gcount();
		if (readCount > 0)
		{
			totalReadCount += readCount;
			MD5Update(&context, buffer.get(), static_cast<unsigned int>(readCount));
		}
		fin.close();

		// 数据完整性检查
		if (totalReadCount != fileLength)
		{
			std::ostringstream oss;
			oss << "FATAL ERROR: read " << filename << " failed!" << std::ends;
			throw std::runtime_error(oss.str());
		}

		unsigned char digest[16];
		MD5Final(digest, &context);

		// 获取MD5
		std::ostringstream oss;
		for (int i = 0; i < 16; ++i)
		{
			oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(digest[i]);
		}
		oss << std::ends;

		return std::move(oss.str());
	}
	else
	{
		std::ostringstream oss;
		oss << "FATAL ERROR: " << filename << " can't be opened" << std::ends;
		throw std::runtime_error(oss.str());
	}
}

void PublicFunction::TraverseFolder(const std::string path, TiXmlElement *element, vector<string> excludeFiles)
{
	if (path.size() == 0 || element == NULL || !IsDirExisted(path))
	{
		return;
	}

	WIN32_FIND_DATAA data;
	char szFind[MAX_PATH] = { 0 };

	string strFileName = "";
	string strLastDate = "";
	ULONGLONG ulLength = 0;

	strcpy_s(szFind, MAX_PATH, path.c_str());
	strcat(szFind, "\\*.*");

	HANDLE handle = ::FindFirstFileA(szFind, &data);
	while(1)
	{
		FILETIME ft = data.ftLastWriteTime;
		SYSTEMTIME st;
		FILETIME localTime;
		string strVersion = "";
		string strMD5 = "";
		string strFilePath = "";
		char szBuffer[MAX_PATH] = { 0 };
		
		if (data.cFileName[0] != _T('.'))
		{
			strFileName = data.cFileName;

			// 判断当前文件是否在 ‘排除文件’列表中，是，跳过此文件
			vector<string>::iterator findRes = find(excludeFiles.begin(), excludeFiles.end(), strFileName);
			if (findRes == excludeFiles.end())
			{
				
				TiXmlElement *fileEle = new TiXmlElement("UpdateFile");
				element->LinkEndChild(fileEle);
			
				strFilePath = path + "\\" + strFileName;
				fileEle->SetAttribute("name", strFileName.c_str());
				if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					// 文件夹
					fileEle->SetAttribute("type", "dir");
					TraverseFolder(strFilePath, fileEle, excludeFiles);
				}
				else
				{
					fileEle->SetAttribute("type", "file");
					ulLength = (data.nFileSizeHigh * (MAXDWORD + 1)) + data.nFileSizeLow;

					// 文件过大，易造成计算MD5耗时过多 100M以上不计算
					if (ulLength < 1024 * 1024 * 100)
					{
						string strPath = path + "\\" + data.cFileName;
						GetFileVersion(strPath, strVersion);
						strMD5 = GetFileMD5(strPath);

						fileEle->SetAttribute("version", strVersion.c_str());
						fileEle->SetAttribute("MD5", strMD5.c_str());
					}
				}
				// 先将文件时间转换为本地文件时间，再转换为系统时间（防止出现时区差异）
				FileTimeToLocalFileTime(&ft, &localTime);
				FileTimeToSystemTime(&localTime, &st);

				sprintf_s(szBuffer, MAX_PATH, "%04d-%02d-%02d %02d:%02d", st.wYear, st.wMonth,
					st.wDay, st.wHour, st.wMinute);
				strLastDate = szBuffer;
				fileEle->SetAttribute("lastwritetime", strLastDate.c_str());
			}
		}

		if (::FindNextFileA(handle, &data) == 0 )
		{
			break;
		}
	}
	::FindClose(handle);
}

bool PublicFunction::InitFolderToXml(const std::string dirpath, const std::string xmlpath, vector<string> excludeFiles)
{
	// 目录不存在
	if (!IsDirExisted(dirpath))
	{
		return false;
	}
	// 输入参数不合法
	if (dirpath.size() == 0 || xmlpath.size() == 0)
	{
		return false;
	}

	// 创建xml
	TiXmlDocument doc;
	TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "UTF-8", "");
	TiXmlElement *element = new TiXmlElement("AutoUpdate");		// 根节点
	TiXmlElement *listEle = new TiXmlElement("FileList");		// 第一个子节点
	doc.LinkEndChild(dec);
	doc.LinkEndChild(element);

	element->LinkEndChild(listEle);

	TraverseFolder(dirpath, listEle, excludeFiles);
	
	return doc.SaveFile(xmlpath.c_str());
}

/* 比较两个xml：相同 返回true；否则返回false
	1、FileList节点以下均需要比较
	2、节点个数，路径，名称，属性及属性值均在比较范围内 */
bool PublicFunction::CompareXMLs(const std::string xmlfirst, const std::string xmlsecond)
{
	if (xmlfirst.size() == 0 || xmlsecond.size() == 0)
	{
		return false;
	}

	// load xml file
	TiXmlDocument docFirst(xmlfirst.c_str()), docSecond(xmlsecond.c_str());
	if (docFirst.LoadFile() || docSecond.LoadFile())
	{
		return false;
	}


	return false;
}

DWORD PublicFunction::ReserveIP(const std::string ip)
{
	in_addr addr;
	addr.S_un.S_addr = inet_addr(ip.c_str());
	//memcpy(&addr, inet_addr(ip.c_str()), sizeof(in_addr));
	DWORD dwIP = MAKEIPADDRESS(addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2,
		addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);
	return dwIP;
}
