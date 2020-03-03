/**
 * @file    curl_client.h
 * @brief   
 *
 * @author  Jungil, Yang (chaserhunter@somma.kr)
 * @date    2017/09/07 16:30 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/

#pragma once
#include <Windows.h>
#include "CStream.h"
#include "curl/curl.h"
#include "_MyLib/src/curl_client_support.h"


//
//	���� ����
// 
//	cURL ���̺귯���� ����ϱ� ���� curl_global_init() �Լ��� ȣ���ϰ�,
//	���� �� curl_global_cleanup() �Լ��� ȣ���� �־�� �Ѵ�. 
//	curl_global_init() �Լ��� ��� curl_easy_init() ���� �ڵ����� ȣ�� ��
//
//	������ curl_global_init() ȣ�� �� curl_global_cleanup() �� ȣ���ص�
//	_CrtMemDumpAllObjectsSince() ���� �Լ����� memory leak �� �����ȴ�. 
//	���ͳݿ� ���õ� �������� ���� ������ OpenSSL ���� �޸� ���̶�� �� ������
//	������ �ذ�å�� ����. 
// 
//	cURL ���̺귯�� ��� �� �޸𸮰� �����ϴ� ������ ���� ������ ũ�� ���������� 
//	�ʴ´�. ������ _Crtxxx() ���� �Լ����� �������� ���� �ϱ� ���ؼ��� 
//	_CrtMemCheckpoint() ȣ�� ������ curl_global_init() �� ȣ���ϰ�, 
//	_CrtMemDumpAllObjectsSince() ���Ŀ� curl_global_cleanup() �� ȣ���ؾ� �Ѵ�. 
// 
//	���� curl_global_init(), curl_global_cleanup() �Լ��� thread safe ���� 
//	�ʱ� ������ curl_client �� ����/�Ҹ�� ȣ���ϴ� ���� �����ϰ�, main �Լ����
//	ȣ���ϴ� ���� �����ϴ�. 
// 
//
//		int main()
//		{
//			curl_global_init(CURL_GLOBAL_ALL);
//		
//			_CrtMemState memoryState = { 0 };
//			_CrtMemCheckpoint(&memoryState);
//			_CrtSetBreakAlloc(860);
//
//			//....................
//
//			_CrtMemDumpAllObjectsSince(&memoryState);
//			curl_global_cleanup();
//			rturn 0;
//		}
//

static const char* _null_http_header_string = "";


typedef class curl_client
{
public:
	curl_client();
	~curl_client();

public:
	bool initialize();

	bool http_get(
		_In_z_ const char* url,
		_Out_ long& http_response_code,
		_Out_ CMemoryStream& stream);

	bool http_get(
		_In_z_ const char* url,
		_Out_ long& http_response_code,
		_Out_ std::string& response);

	bool http_download_file(
		_In_ const http_download_ctx* ctx,
		_Out_ long& http_response_code);

	bool http_post(
		_In_z_ const char* url,
		_In_z_ const char* data,
		_Out_  long& http_response_code,
		_Out_  CMemoryStream& stream);

	bool http_post(
		_In_z_ const char* url,
		_In_z_ const char* data,
		_Out_  long& http_response_code,
		_Out_  std::string& response);

	//
	// http_file_upload �Լ��� ����ϸ�, ���� �̸��� ������ ���۵ȴ�.
	// ����, �߰������� ������ �����Ͱ� �ִٸ� forms�� ����Ѵ�.
	//
#pragma todo("Forms ��ü�� �Ķ���ͷ� ���� �ʰ�. add_form() ���·� �����丵")
	typedef std::map<std::string, std::string> Forms;
	bool http_file_upload(
		_In_z_ const char* url,
		_In_z_ const wchar_t* file_path,
		_In_   Forms& forms,
		_Out_  long& http_response_code,
		_Out_  CMemoryStream& stream);
	bool http_file_upload(
		_In_z_ const char* url,
		_In_z_ const wchar_t* file_path,
		_In_   Forms& forms,
		_Out_  long& http_response_code,
		_Out_  std::string& response);

private:
	//
	//	CURL object
	//
	CURL* _curl;

private:
	//
	//	Options that applied on every I/O perform
	//
	std::string _url;
	long _conn_timeout;
	long _read_timeout;
	bool _follow_location;
	bool _ssl_verify_peer;
	bool _ssl_verify_host;
	bool _verbose;

	typedef std::map<std::string, std::string> Fields;
	Fields _header_fields;
public:
	bool enable_auth(_In_ const char* id, _In_ const char* password);
	void append_header(_In_z_ const char* key, _In_z_ const char* value);

	void set_url(const char* const url) { _url = url; }
	void set_connection_timeout(const long value) { _conn_timeout = value; }
	void set_read_timeout(const long value) { _read_timeout = value; }
	void set_follow_location(const bool value) { _follow_location = value; }
	void set_ssl_verify_peer(const bool value) { _ssl_verify_peer = value; }
	void set_ssl_verify_host(const bool value) { _ssl_verify_host = value; }
	void set_verbose(const bool value) { _verbose = value; }


private:
	bool prepare_perform(_In_ const char* const url);

	bool perform(_Out_ long& http_response_code);

	// multipart/form type�� request body data�� ������ �� �����ϴ� �Լ�
	bool perform(
		_In_ const char* file_path, 
		_In_ Forms& forms, 
		_Out_ long& http_response_code);

	void finalize();



} *pcurl_client;
