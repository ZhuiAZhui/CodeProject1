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

// ���� -- true�� ������ -- false
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

	// �ļ�ָ������Ϊ�ļ�β
	fseek(pFile, 0, SEEK_END);

	if (log.size() > fwrite(log.c_str(), 1, log.size(), pFile))
	{
		fclose(pFile);
		return false;
	}

	fclose(pFile);
	return true;
}


// ��ѯ�ļ���ĳ����Ϣ�����磺�ļ�˵�����ļ��汾����Ʒ���ƣ�ԭʼ�ļ�����
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
		//������Ϣ����֧�ְ汾��ǵ�һ��ģ���л�ȡ�ļ��汾��Ϣ
		if (!::GetFileVersionInfoA(ModuleName.c_str(), dwHandle,
			dwDataSize, bpVersionData))
		{
			break;
		}

		UINT uiQuerySize = 0;
		DWORD *pdwTransTable = NULL;
		// ��������
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

		// ���ô˺�����ѯǰ��Ҫ�����ε��ú���GetFileVersionInfoSize��GetFileVersionInfo  
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
		// ��ȡ�ļ����ݣ�����MD5Update()����MD5ֵ
		while (fin.read(reinterpret_cast<char*>(buffer.get()), bufferLen))
		{
			readCount = fin.gcount();
			totalReadCount += readCount;
			MD5Update(&context, buffer.get(), static_cast<unsigned int>(readCount));
		}
		// �������һ�ζ���������
		readCount = fin.gcount();
		if (readCount > 0)
		{
			totalReadCount += readCount;
			MD5Update(&context, buffer.get(), static_cast<unsigned int>(readCount));
		}
		fin.close();

		// ���������Լ��
		if (totalReadCount != fileLength)
		{
			std::ostringstream oss;
			oss << "FATAL ERROR: read " << filename << " failed!" << std::ends;
			throw std::runtime_error(oss.str());
		}

		unsigned char digest[16];
		MD5Final(digest, &context);

		// ��ȡMD5
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

			// �жϵ�ǰ�ļ��Ƿ��� ���ų��ļ����б��У��ǣ��������ļ�
			vector<string>::iterator findRes = find(excludeFiles.begin(), excludeFiles.end(), strFileName);
			if (findRes == excludeFiles.end())
			{
				
				TiXmlElement *fileEle = new TiXmlElement("UpdateFile");
				element->LinkEndChild(fileEle);
			
				strFilePath = path + "\\" + strFileName;
				fileEle->SetAttribute("name", strFileName.c_str());
				if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					// �ļ���
					fileEle->SetAttribute("type", "dir");
					TraverseFolder(strFilePath, fileEle, excludeFiles);
				}
				else
				{
					fileEle->SetAttribute("type", "file");
					ulLength = (data.nFileSizeHigh * (MAXDWORD + 1)) + data.nFileSizeLow;

					// �ļ���������ɼ���MD5��ʱ���� 100M���ϲ�����
					if (ulLength < 1024 * 1024 * 100)
					{
						string strPath = path + "\\" + data.cFileName;
						GetFileVersion(strPath, strVersion);
						strMD5 = GetFileMD5(strPath);

						fileEle->SetAttribute("version", strVersion.c_str());
						fileEle->SetAttribute("MD5", strMD5.c_str());
					}
				}
				// �Ƚ��ļ�ʱ��ת��Ϊ�����ļ�ʱ�䣬��ת��Ϊϵͳʱ�䣨��ֹ����ʱ�����죩
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
	// Ŀ¼������
	if (!IsDirExisted(dirpath))
	{
		return false;
	}
	// ����������Ϸ�
	if (dirpath.size() == 0 || xmlpath.size() == 0)
	{
		return false;
	}

	// ����xml
	TiXmlDocument doc;
	TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "UTF-8", "");
	TiXmlElement *element = new TiXmlElement("AutoUpdate");		// ���ڵ�
	TiXmlElement *listEle = new TiXmlElement("FileList");		// ��һ���ӽڵ�
	doc.LinkEndChild(dec);
	doc.LinkEndChild(element);

	element->LinkEndChild(listEle);

	TraverseFolder(dirpath, listEle, excludeFiles);
	
	return doc.SaveFile(xmlpath.c_str());
}

/* �Ƚ�����xml����ͬ ����true�����򷵻�false
	1��FileList�ڵ����¾���Ҫ�Ƚ�
	2���ڵ������·�������ƣ����Լ�����ֵ���ڱȽϷ�Χ�� */
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
