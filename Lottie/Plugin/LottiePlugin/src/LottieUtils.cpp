#include "Utils.h"

#ifdef _WINDOWS
#ifdef USE_HTTP_SERVER
    #include <WinSock.h>
#endif
    #include "Windows.h"
    #include "ShellApi.h"
#endif

#ifdef __APPLE__
    #include "CoreFoundation/CoreFoundation.h"
    #include <dlfcn.h>
#ifdef USE_HTTP_SERVER    
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif 
    #include <sys/stat.h>    

    #include <copyfile.h>
#endif

#include <iomanip>
#include <algorithm>
#include <sstream>

#include "IFCMStringUtils.h"

#include <string>
#include <cstring>
#include <stdlib.h>
#include "Application/Service/IOutputConsoleService.h"
#include "Application/Service/IFlashApplicationService.h"
#include "FlashFCMPublicIDs.h"

#ifndef _WINDOWS
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <ftw.h>
#include <unistd.h>
#endif // !_WINDOWS

/* -------------------------------------------------- Constants */

#ifdef _WINDOWS
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

namespace LottieExporter
{
    static std::string comma = ",";
}


/* -------------------------------------------------- Static Functions */


/* -------------------------------------------------- Utils */

namespace LottieExporter
{
    FCM::AutoPtr<FCM::IFCMStringUtils> Utils::GetStringUtilsService(FCM::PIFCMCallback pCallback)
    {
        FCM::AutoPtr<FCM::IFCMUnknown> pIFCMStringUtilsUnknown;
        FCM::Result res = pCallback->GetService(FCM::SRVCID_Core_StringUtils, pIFCMStringUtilsUnknown.m_Ptr);
        if (FCM_FAILURE_CODE(res))
        {
            return NULL;
        }
        FCM::AutoPtr<FCM::IFCMStringUtils> pIFCMStringUtils = pIFCMStringUtilsUnknown;
        return pIFCMStringUtils;
    }
    

    FCM::AutoPtr<FCM::IFCMCalloc> Utils::GetCallocService(FCM::PIFCMCallback pCallback)
    {
        FCM::AutoPtr<FCM::IFCMUnknown> pIFCMCallocUnknown;
        FCM::Result res = pCallback->GetService(FCM::SRVCID_Core_Memory, pIFCMCallocUnknown.m_Ptr);
        if (FCM_FAILURE_CODE(res))
        {
            return NULL;
        }
        FCM::AutoPtr<FCM::IFCMCalloc> pIFCMCalloc = pIFCMCallocUnknown;
        return pIFCMCalloc;
    }
    

    void Utils::GetLanguageCode(FCM::PIFCMCallback pCallback, std::string& langCode)
    {
        FCM::StringRep8 pLanguageCode;
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        FCM::AutoPtr<Application::Service::IFlashApplicationService> pAppService;
        FCM::Result res;
        
        res = pCallback->GetService(Application::Service::FLASHAPP_SERVICE, pUnk.m_Ptr);
        pAppService = pUnk;

        if (pAppService)
        {
            res = pAppService->GetLanguageCode(&pLanguageCode);
            if (FCM_SUCCESS_CODE(res))
            {
                langCode = ToString(pLanguageCode);

                FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = GetCallocService(pCallback);
                pCalloc->Free(pLanguageCode);
            }
        }
    }

    void Utils::GetAppVersion(FCM::PIFCMCallback pCallback, FCM::U_Int32& version)
    {
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        FCM::AutoPtr<Application::Service::IFlashApplicationService> pAppService;
        FCM::Result res;
        
        version = 0;

        res = pCallback->GetService(Application::Service::FLASHAPP_SERVICE, pUnk.m_Ptr);
        pAppService = pUnk;

        if (pAppService)
        {
            res = pAppService->GetVersion(version);
            ASSERT(FCM_SUCCESS_CODE(res))
        }
    }

    std::string Utils::ToString(const FCM::FCMGUID& in)
    {
        std::ostringstream result;
        unsigned i;

        result.fill('0');

        result << std::hex;
        result  << std::setw(8) << (in.Data1);
        result << "-";
        result  << std::setw(4) << (in.Data2);
        result << "-";
        result  << std::setw(4) << (in.Data3);
        result << "-";

        for (i = 0; i < 2 ; ++i)
        {
            result << std::setw(2) << (unsigned int) (in.Data4[i]);
        }
        result << "-";

        for (; i < 8 ; ++i)
        {
            result << std::setw(2) << (unsigned int) (in.Data4[i]);
        }

        std::string guid_str = result.str();

        std::transform(guid_str.begin(), guid_str.end(), guid_str.begin(), ::toupper);

        return guid_str;
    }

    
    std::string Utils::ToString(FCM::CStringRep16 pStr16, FCM::PIFCMCallback pCallback)
    {
        FCM::StringRep8 pStr8 = NULL;
        FCM::AutoPtr<FCM::IFCMStringUtils> pStrUtils = GetStringUtilsService(pCallback);
        pStrUtils->ConvertStringRep16to8(pStr16, pStr8);
        
        std::string string = (const char*)pStr8;
        
        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = GetCallocService(pCallback);
        pCalloc->Free(pStr8);
        
        return string;
    }

    std::string Utils::ToString(FCM::CStringRep8 pStr8)
    {
        std::string string = (const char*)pStr8;
        return string;
    }
    
    std::string Utils::ToString(const double& in)
    {
        char buffer[32];
        sprintf(buffer,"%.6f", in);
        std::string str(buffer);
        return str;
    }
    
    std::string Utils::ToString(const float& in)
    {
        char buffer[32];
        sprintf(buffer,"%.6f", in);
        std::string str(buffer);
        return str;
    }
    
    std::string Utils::ToString(const FCM::U_Int32& in)
    {
        char buffer[32];
        sprintf(buffer,"%u", in);
        std::string str(buffer);
        return str;
    }
    
    std::string Utils::ToString(const FCM::S_Int32& in)
    {
        char buffer[32];
        sprintf(buffer,"%d", in);
        std::string str(buffer);
        return str;
    }
    
    std::string Utils::ToString(const DOM::Utils::MATRIX2D& matrix)
    {
        std::string matrixString = "";

        matrixString.append(ToString(matrix.a));
        matrixString.append(comma);
        matrixString.append(ToString(matrix.b));
        matrixString.append(comma);
        matrixString.append(ToString(matrix.c));
        matrixString.append(comma);
        matrixString.append(ToString(matrix.d));
        matrixString.append(comma);
        matrixString.append(ToString(matrix.tx));
        matrixString.append(comma);
        matrixString.append(ToString(matrix.ty));

        return matrixString;
    }

    std::string Utils::ToString(const DOM::Utils::CapType& capType)
    {
        std::string str;

        switch (capType)
        {
            case DOM::Utils::NO_CAP:
                str = "butt";
                break;

            case DOM::Utils::ROUND_CAP:
                str = "round";
                break;

            case DOM::Utils::SQUARE_CAP:
                str = "square";
                break;
        }

        return str;
    }

    std::string Utils::ToString(const DOM::Utils::JoinType& joinType)
    {
        std::string str;

        switch (joinType)
        {
            case DOM::Utils::MITER_JOIN:
                str = "miter";
                break;

            case DOM::Utils::ROUND_JOIN:
                str = "round";
                break;

            case DOM::Utils::BEVEL_JOIN:
                str = "bevel";
                break;
        }

        return str;
    }

    
    FCM::StringRep16 Utils::ToString16(const std::string& str, FCM::PIFCMCallback pCallback)
    {
        FCM::StringRep16 pStrFeatureName = NULL;
        FCM::AutoPtr<FCM::IFCMStringUtils> pStrUtils = GetStringUtilsService(pCallback);
        pStrUtils->ConvertStringRep8to16(str.c_str(), pStrFeatureName);
        
        return pStrFeatureName;
    }


    std::string Utils::ToString(const DOM::FillStyle::GradientSpread& spread)
    {
        std::string res;

        switch (spread)
        {
            case DOM::FillStyle::GRADIENT_SPREAD_EXTEND:
                res = "pad";
                break;

            case DOM::FillStyle::GRADIENT_SPREAD_REFLECT:
                res = "reflect";
                break;

            case DOM::FillStyle::GRADIENT_SPREAD_REPEAT:
                res = "repeat";
                break;

            default:
                res = "none";
                break;
        }

        return res;
    }


    std::string Utils::ToString(const DOM::Utils::COLOR& color)
    {
        char cstr[5];
        std::string colorStr;

        colorStr.append("#");
        sprintf(cstr, "%02x", color.red);
        colorStr.append(cstr);
        sprintf(cstr, "%02x", color.green);
        colorStr.append(cstr);
        sprintf(cstr, "%02x", color.blue);
        colorStr.append(cstr);

        return colorStr;
    }

	std::string Utils::removeSpecialChars(std::string str)
	{
		std::string result;
		for (size_t i = 0; i < str.length(); i++)
		{
			if (isalnum(str[i]))
				result.append(1, str[i]);
		}

		return result;
	}
    
    void Utils::replaceAll(std::string& str, const std::string& from, const std::string& to) {
        if(from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }
    
    std::string Utils::replaceSpecialChars(std::string str, std::string rep) {
        std::string result;
        for (size_t i = 0; i < str.length(); i++)
        {
            if (isalnum(str[i]))
                result.append(1, str[i]);
            else
                result.append(rep);
        }
        
        return result;
    }

	std::string Utils::cleanScript(std::string& str)
	{
		str = removeComments(str);
		std::string::size_type pos = 0; // Must initialize
		while ((pos = str.find("\n", pos)) != std::string::npos)
		{
			str.erase(pos, 1);
		}
		pos = 0;
		while ((pos = str.find("\t", pos)) != std::string::npos)
		{
			str.erase(pos, 1);
		}
		return str;
	}

	std::string Utils::removeComments(std::string& prgm)
	{
		int n = prgm.length();
		std::string res;

		bool isSingleLineComment = false;
		bool isMultiLineComment = false;

		for (int i = 0; i<n; i++)
		{
			if (isSingleLineComment == true && prgm[i] == '\n')
				isSingleLineComment = false;

			else if (isMultiLineComment == true && prgm[i] == '*' && prgm[i + 1] == '/')
				isMultiLineComment = false, i++;

			else if (isSingleLineComment || isMultiLineComment)
				continue;

			else if (prgm[i] == '/' && prgm[i + 1] == '/')
				isSingleLineComment = true, i++;
			else if (prgm[i] == '/' && prgm[i + 1] == '*')
				isMultiLineComment = true, i++;

			else  res += prgm[i];
		}
		return res;
	}

    void Utils::TransformPoint(
            const DOM::Utils::MATRIX2D& matrix, 
            DOM::Utils::POINT2D& inPoint,
            DOM::Utils::POINT2D& outPoint)
    {
        DOM::Utils::POINT2D loc;

        loc.x = inPoint.x * matrix.a + inPoint.y * matrix.c + matrix.tx;
        loc.y = inPoint.x * matrix.b + inPoint.y * matrix.d + matrix.ty;

        outPoint = loc;
    }

    void Utils::GetParent(const std::string& path, std::string& parent)
    {
        size_t index = path.find_last_of("/\\");
        if((index+1) == path.length())
        {
            parent = path.substr(0, index);
            index = parent.find_last_of("/\\");
        }
        parent = path.substr(0, index + 1);
    }

    void Utils::GetFileName(const std::string& path, std::string& fileName)
    {
        size_t index = path.find_last_of("/\\");
        fileName = path.substr(index + 1, path.length() - index - 1);
    }

    void Utils::GetFileNameWithoutExtension(const std::string& path, std::string& fileName)
    {
        GetFileName(path, fileName);

        // Remove the extension (if any)
        size_t index = fileName.find_last_of(".");
        if (index != std::string::npos)
        {
            fileName = fileName.substr(0, index);
        }
    }

    void Utils::GetFileExtension(const std::string& path, std::string& extension)
    {
        size_t index = path.find_last_of(".");
        extension = "";
        if (index != std::string::npos)
        {
            extension = path.substr(index + 1, path.length());
        }
    }

    void Utils::GetModuleFilePath(std::string& path, FCM::PIFCMCallback pCallback)
    {
#ifdef _WINDOWS

        std::string fullPath;
        FCM::U_Int16* pFilePath = new FCM::U_Int16[MAX_PATH];

        ASSERT(pFilePath);

        ::GetModuleFileName((HINSTANCE)&__ImageBase, pFilePath, MAX_PATH);
        
        fullPath = Utils::ToString(pFilePath, pCallback);

        GetParent(fullPath, path);

        delete[] pFilePath;
        
#else
        Dl_info info;
        if (dladdr((void*)(GetModuleFilePath), &info)) {
            std::string fullPath(info.dli_fname);
            GetParent(fullPath, path);
            GetParent(path, fullPath);
            GetParent(fullPath, path);
            GetParent(path, fullPath);
            path = fullPath;
        }
        else{
            ASSERT(0);
        }
#endif
    }


    // Creates a directory. If the directory already exists or is successfully created, success
    // is returned; otherwise an error code is returned.
    FCM::Result Utils::CreateDir(const std::string& path, FCM::PIFCMCallback pCallback)
    {
#ifdef _WINDOWS

        FCM::Result res = FCM_SUCCESS;
        BOOL ret;
        FCM::StringRep16 pFullPath;

        pFullPath = Utils::ToString16(path, pCallback);
        ASSERT(pFullPath);

        ret = ::CreateDirectory(pFullPath, NULL);
        if (ret == FALSE)
        {
            DWORD err = GetLastError();
            if (err != ERROR_ALREADY_EXISTS)
            {
                res = FCM_GENERAL_ERROR;
            }
        }

        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = Utils::GetCallocService(pCallback);
        ASSERT(pCalloc.m_Ptr != NULL);  
        pCalloc->Free(pFullPath);

        return res;

#else
        struct stat sb;
        
        // Does the directory exist?
        if (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
        {
            return FCM_SUCCESS;
        }
        
        int err = mkdir(path.c_str(), 0777);
        if ((err == 0) || (err == EEXIST))
        {

            return FCM_SUCCESS;
        }

        return FCM_GENERAL_ERROR;
#endif
    }
    void Utils::OpenFStream(const std::string& outputFileName, std::fstream &file, std::ios_base::openmode mode, FCM::PIFCMCallback pCallback)
    {
 
#ifdef _WINDOWS
        FCM::StringRep16 pFilePath = Utils::ToString16(outputFileName, pCallback);

        file.open(pFilePath,mode);

        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = Utils::GetCallocService(pCallback);
        ASSERT(pCalloc.m_Ptr != NULL);  
        pCalloc->Free(pFilePath);
#else
       file.open(outputFileName.c_str(),mode);
#endif
    }

    void Utils::Trace(FCM::PIFCMCallback pCallback, const char* fmt, ...)
    {
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        FCM::AutoPtr<Application::Service::IOutputConsoleService> outputConsoleService;
        FCM::Result tempRes = pCallback->GetService(Application::Service::FLASHAPP_OUTPUT_CONSOLE_SERVICE, pUnk.m_Ptr);
        outputConsoleService = pUnk;
        pUnk.Reset();

        if (outputConsoleService)
        {
            va_list args;
            char buffer[1024];

            va_start(args, fmt);
            vsnprintf(buffer, 1024, fmt, args);
            va_end(args);

            FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = LottieExporter::Utils::GetCallocService(pCallback);
            ASSERT(pCalloc.m_Ptr != NULL);

            FCM::StringRep16 outputString = Utils::ToString16(std::string(buffer), pCallback);
            outputConsoleService->Trace(outputString);
            pCalloc->Free(outputString);
        }
    }

    void Utils::Log(const char* fmt, ...)
    {
        va_list args;
        char buffer[1024];

        va_start(args, fmt);
        vsnprintf(buffer, 1024, fmt, args);
        va_end(args);

        //printf(buffer);
    }

#ifndef _WINDOWS
	int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
	{
		int rv = remove(fpath);

		if (rv)
			perror(fpath);

		return rv;
	}

	int rmrf(const char *path)
	{
		return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
	}
#endif // !_WINDOWS
    

    // Removes the folder all its contents
    FCM::Result Utils::Remove(const std::string& folder, FCM::PIFCMCallback pCallback)
    {
        FCM::Result result = FCM_SUCCESS;

#ifdef _WINDOWS

        SHFILEOPSTRUCT sf;
        std::wstring wstr;

        memset(&sf, 0, sizeof(sf));

        sf.hwnd = NULL;
        sf.wFunc = FO_DELETE;
        sf.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;
        FCM::StringRep16 folderStr = Utils::ToString16(folder, pCallback);;
        wstr = folderStr;
        wstr.append(1, '\0');
        sf.pFrom = wstr.c_str();
        sf.pTo = NULL;

        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = GetCallocService(pCallback);
        int n = SHFileOperation(&sf);
        if (n != 0)
        {
            pCalloc->Free(folderStr);
            return FCM_GENERAL_ERROR;
        }

        pCalloc->Free(folderStr);

#else
        if(rmrf(folder.c_str()) != 0)
        {
            result = FCM_GENERAL_ERROR;
        }
#endif

        return result;
    }


    // Copies a source folder to a destination folder. In other words, dstFolder contains
    // the srcFolder after the operation.
    FCM::Result Utils::CopyDir(const std::string& srcFolder, const std::string& dstFolder, FCM::PIFCMCallback pCallback)
    {
#ifdef _WINDOWS

        SHFILEOPSTRUCT sf;
        std::wstring srcWstr;
        std::wstring dstWstr;

        memset(&sf, 0, sizeof(sf));

        sf.hwnd = NULL;
        sf.wFunc = FO_COPY;
        sf.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;

        FCM::StringRep16 srcFolderStr = Utils::ToString16(srcFolder, pCallback);
        srcWstr = srcFolderStr;
        srcWstr.append(1, '\0');
        sf.pFrom = srcWstr.c_str();
        FCM::StringRep16 dstFolderStr = Utils::ToString16(dstFolder, pCallback);
        dstWstr = dstFolderStr;
        dstWstr.append(1, '\0');
        sf.pFrom = srcWstr.c_str();
        sf.pTo = dstWstr.c_str();

        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = GetCallocService(pCallback);
        int n = SHFileOperation(&sf);
        if (n != 0)
        {
            pCalloc->Free(srcFolderStr);
            pCalloc->Free(dstFolderStr);
            return FCM_GENERAL_ERROR;
        }

        pCalloc->Free(srcFolderStr);
        pCalloc->Free(dstFolderStr);
#else

        copyfile(srcFolder.c_str(), dstFolder.c_str(), NULL, COPYFILE_ALL | COPYFILE_RECURSIVE);
#endif
        return FCM_SUCCESS;
    }
    
    void Utils::FreeString16(FCM::StringRep16 str, FCM::PIFCMCallback pCallback){
        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = Utils::GetCallocService(pCallback);
        ASSERT(pCalloc.m_Ptr != NULL);
        pCalloc->Free(str);
    }


#ifdef USE_HTTP_SERVER

    void Utils::LaunchBrowser(const std::string& outputFileName, int port, FCM::PIFCMCallback pCallback)
    {

#ifdef _WINDOWS

        std::wstring output = L"http://localhost:";
		FCM::StringRep16 tail = Utils::ToString16(outputFileName, pCallback);
        FCM::StringRep16 portStr = Utils::ToString16(Utils::ToString(port), pCallback);
        output += portStr;
        output += L"/";
        output += tail;
        ShellExecute(NULL, L"open", output.c_str(), NULL, NULL, SW_SHOWNORMAL);

        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = GetCallocService(pCallback);
        pCalloc->Free(portStr);
		pCalloc->Free(tail);
#else

        std::string output = "http://localhost:";
        output += Utils::ToString(port);
        output += "/";
        std::string encodedFileName;
        GeneralUtils::encodeUrl(outputFileName.c_str(), encodedFileName);
        output += encodedFileName;
        std::string str = "/usr/bin/open " + output;
        popen(str.c_str(), "r");
        
#endif // _WINDOWS

    }

    int Utils::GetUnusedLocalPort()
    {
        sockaddr_in client;

        InitSockAddr(&client);

        SOCKET sock = socket(PF_INET, SOCK_STREAM, 0);
        
        // Look for a port in the private port range
        int minPortNumber = 49152;
        int maxPortNumber = 65535;
        int defaultPortNumber = 50000;
        
        int port = defaultPortNumber;
        
        int nTries = 0;
        int maxTries = 10;
        srand ((int)time(NULL));

        // Try connect
        while (nTries++ <= maxTries) 
        {
            client.sin_port = htons(port);

            int result = connect(sock, (struct sockaddr *) &client, sizeof(client));

            CLOSE_SOCKET(sock);

            // Connect unsuccessful, port is available.
            if (result != 0) 
            {
                break;  
            }

            // Retry at a random port number in the valid range
            port = minPortNumber + rand() % (maxPortNumber - minPortNumber - 1);
        }
        
        if (nTries > maxTries) 
        {
            port = -1;
        }
        
        return port;
    }

    void Utils::InitSockAddr(sockaddr_in* sockAddr)
    {
        ASSERT(sockAddr);

        memset(sockAddr, 0, sizeof(struct sockaddr_in));
        sockAddr->sin_family = AF_INET;
        char *ipAddressStr = (char*)"127.0.0.1";

#ifdef _MAC
        sockAddr->sin_addr.s_addr = inet_addr(ipAddressStr);
#endif
        
#ifdef _WINDOWS
        sockAddr->sin_addr.S_un.S_addr = inet_addr(ipAddressStr);
#endif

    }

#endif // USE_HTTP_SERVER
}

