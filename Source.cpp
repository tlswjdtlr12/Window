#include "stdafx.h"
#include <Windows.h>
#include <locale.h>
#include <direct.h> //mkdir
#include <wchar.h> //wchar
#include <stdint.h>  //uint
#include <strsafe.h> //succeeded
#include <crtdbg.h> //asserte

//http://stackoverflow.com/questions/1373463/handling-special-characters-in-c-utf-8-encoding
//utf-8관련 문서...
bool read_file_using_memory_map();
bool create_copy_file();
bool read_file();
int _tmain(int argc, _TCHAR * argv[]) { //_tmain은 유니코드 지원이 필요한 프로그램이면 wmain,아니면 main
	setlocale(LC_ALL, "");//시스템 지역 설정을 따르게 해서 한글 파일 입출력 가능케함
	//현재 디렉토리\bob.txt 파일을 생성
//	FILE * bob=fopen("./bob.txt","w");
	//wchar_t string[100]={0,};

	//getcwd(string,100);
	//buffer = getcwd(string,260); //정상:0, 에러시:-1
	//wchar_t string[]=L"노용환멘토님 만세~ I can give my word.\n";
	
	create_copy_file();

	//fprintf(bob,"노용환멘토님 만세~ I can give my word.\n");
	//setlocale(LC_ALL, "");
	//fclose(bob); //여기서 fclose를 해줘야 정상적으로 bob2에 값이 들어감
	

	//fclose(bob2);
	


//WriteFile(hFile, wdata, strlen(wdata), &dwWrite, NULL);
	//ReadFile(hFile, rdata, dwWrite, &dwRead, NULL);


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@여기까지 readfile, 밑으로는 MMIO
	read_file();
	read_file_using_memory_map();
	
	remove("./bob.txt");
	remove("./bob2.txt");


    return 0;
}
bool read_file() //readfile
{
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("error");
		free(buf);
		return false;
	}
	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob2.txt",
		buf)))
	{
		printf("error");
		free(buf);
		return false;
	}
	free(buf); buf = NULL;

	HANDLE file_handle = CreateFileW(
		(LPCWSTR)file_name,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	wchar_t* bstrstr = NULL;
	char multibyteBuffer[1024];
	char* str = NULL;
	DWORD numberOfByteRead;

	memset(multibyteBuffer, 0, sizeof(multibyteBuffer));

	if (TRUE != ReadFile(file_handle, multibyteBuffer, 1023, &numberOfByteRead, NULL))
	{
		printf("error");
		CloseHandle(file_handle);
		return false;
	}
	int Len1 = MultiByteToWideChar(CP_UTF8, 0, multibyteBuffer, -1, bstrstr, 0);
	bstrstr = (wchar_t*)malloc(sizeof(wchar_t)*(Len1+1));
	memset(bstrstr, 0, sizeof(bstrstr));
	MultiByteToWideChar(CP_UTF8, 0, multibyteBuffer, -1, bstrstr, Len1);

	int Len2 = WideCharToMultiByte(CP_ACP, 0, bstrstr, -1, str, 0, NULL, NULL);
	str = (char*)malloc(Len2 + 1);
	memset(str, 0, sizeof(str));
	WideCharToMultiByte(CP_ACP, 0, bstrstr, -1, str, Len2, NULL, NULL);
	
	printf("ReadFile : ");
	printf("%s\n", str+1);


	
	
	free(bstrstr);
	free(str);
	CloseHandle(file_handle);

	return true;
}
bool read_file_using_memory_map() //mmio
{
	wchar_t *buf=NULL;
    uint32_t buflen = 0;
    buflen = GetCurrentDirectoryW(buflen, buf);
	

	buf = (PWSTR) malloc(sizeof(WCHAR) * buflen);
	if(0==GetCurrentDirectoryW(buflen,buf)){
		printf("error");
		free(buf);
		return false;
	}
    wchar_t file_name[260];
    if (!SUCCEEDED(StringCbPrintfW(
                            file_name, 
                            sizeof(file_name), 
                            L"%ws\\bob2.txt", 
                            buf)))
    {  
        printf("error");
        free(buf);
        return false;
    }
    free(buf); buf = NULL;

    HANDLE file_handle = CreateFileW((LPCWSTR)file_name, GENERIC_READ, NULL,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == file_handle)
	{
		printf("error");
		return false;
	}

    LARGE_INTEGER fileSize;
    if (TRUE != GetFileSizeEx(file_handle, &fileSize))
    {
        printf("error");
        CloseHandle(file_handle);
        return false;
    }

    _ASSERTE(fileSize.HighPart == 0);

    DWORD file_size = (DWORD)fileSize.QuadPart;
    HANDLE file_map = CreateFileMapping(file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (NULL == file_map)
    {
        printf("error");
        CloseHandle(file_handle);
        return false;
    }

    PCHAR file_view = (PCHAR) MapViewOfFile(file_map,FILE_MAP_READ,0,0,0);
    if(file_view == NULL)
    {
        printf("error");
        
        CloseHandle(file_map);
        CloseHandle(file_handle);
        return false;
    }  
	//아랫부분은 민군 트랙의 장효원 형에게 도움을 얻었습니다.
	wchar_t *bstrstr = NULL;
	char* str = NULL;
	int Len1 = MultiByteToWideChar(CP_UTF8, 0, file_view, -1, bstrstr, 0);
	//여기에서 Len1 : 38, 한글은 2바이트로 인식
	bstrstr = (wchar_t*)malloc(sizeof(wchar_t)*(Len1 + 1));
	memset(bstrstr, 0, sizeof(bstrstr));
	MultiByteToWideChar(CP_UTF8, 0, file_view, -1, bstrstr, Len1);

	int Len2 = WideCharToMultiByte(CP_ACP, 0, bstrstr, -1, str, 0, NULL, NULL);
	str = (char*)malloc(Len2);
	memset(str, 0, sizeof(str));
	WideCharToMultiByte(CP_ACP, 0, bstrstr, -1, str, Len2, NULL, NULL);
	
	printf("\nMMIO : ");
	printf("%s\n", str+1);

    // close
    UnmapViewOfFile(file_view);
    CloseHandle(file_map);
    CloseHandle(file_handle);
    return true;
}
bool create_copy_file()
{
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);

	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("error");
		free(buf);
		return false;
	}

	wchar_t file_name[260];
	wchar_t file_name2[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob.txt",
		buf)))
	{
		printf("error");
		free(buf);
		return false;
	}
	if (!SUCCEEDED(StringCbPrintfW(
		file_name2,
		sizeof(file_name2),
		L"%ws\\bob2.txt",
		buf)))
	{
		printf("error");
		free(buf);
		return false;
	}
	free(buf); buf = NULL;

	HANDLE file_handle = CreateFileW(
		(LPCWSTR)file_name,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (INVALID_HANDLE_VALUE == file_handle)
	{
		printf("error");
		return false;
	}

	wchar_t str[260];// "노용환멘토님 만세~ I can give my word." 문자열 생성
	if (!SUCCEEDED(StringCbPrintfW( 
		str,
		sizeof(str),
		L"노용환멘토님 만세~ I can give my word.")))
	{
		printf("error");
		return false;
	}

	//문자열을 UTF-8로 인코딩
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, str, wcslen(str), NULL, 0, NULL, NULL);
	char* multibyteBuffer;
	multibyteBuffer = (char*)malloc(sizeof(char)*size_needed);
	
	WideCharToMultiByte(CP_UTF8, 0, str, size_needed, multibyteBuffer, size_needed, NULL, NULL);
	
	unsigned char mark[3]; //UTF-8의 헤더값
	mark[0] = 0xEF;
	mark[1] = 0xBB;
	mark[2] = 0xBF;
	DWORD numberOfByteWritten;

	if (TRUE != WriteFile(file_handle, &mark, sizeof(mark), &numberOfByteWritten, NULL))
	{
		printf("error");
		CloseHandle(file_handle);
		return false;
	}
	if (TRUE != WriteFile(file_handle, multibyteBuffer, size_needed, &numberOfByteWritten, NULL)) // "노용환멘토님 만세~ I can give my word." 문자열 write
	{
		printf("error");
		CloseHandle(file_handle);
		return false;
	}

	free(multibyteBuffer); //malloc 해제
	CloseHandle(file_handle);
	
	if (TRUE != CopyFile((LPCWSTR)file_name, (LPCWSTR)file_name2, false)) //파일 copy
	{
		printf("error");
		return false;
	}
}