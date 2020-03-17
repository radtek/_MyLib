#include "stdafx.h"
#include "_MyLib/src/AirCrypto.h"
#include "_MyLib/src/log.h"
#include "_MyLib/src/Win32Utils.h"

/**
* @brief	test_aes256()���� ������ ��ο� ���� ������ �����Ѵ�.
*/
bool create_test_sample(_In_ const std::wstring& target_file_path)
{
	_ASSERTE(nullptr != target_file_path.c_str());
	if (nullptr == target_file_path.c_str())
	{
		return false;
	}

	std::wstring directory;
	if (!extract_last_tokenW(const_cast<std::wstring&>(target_file_path),
		L"\\",
		directory,
		true,
		false))
	{
		log_err "extract_last_tokenW(path=%ws) failed.", target_file_path.c_str() log_end;
		return false;
	}

	//�׽�Ʈ�� ���丮 Ȯ��
	if (!is_dir(directory.c_str()))
	{
		log_info "create directory(%ws)!!", directory.c_str() log_end;
		if (!CreateDirectoryW(directory.c_str(), NULL))
		{
			log_err "CreateDirectoryW failed. gle=%u", GetLastError() log_end;
			return false;
		}
	}
	
	//������ �����ϸ� ������ �ʿ䰡 ����
	if (is_file_existsW(target_file_path.c_str()))
	{
		log_info "%ws file exists!!", target_file_path.c_str() log_end;
		return true;
	}

	//�׽�Ʈ ���� ����
	HANDLE create_file = CreateFileW(target_file_path.c_str(),
									 GENERIC_WRITE,
									 NULL,
									 NULL,
									 CREATE_ALWAYS,
									 FILE_ATTRIBUTE_NORMAL,
									 NULL);
	if (INVALID_HANDLE_VALUE == create_file)
	{
		log_err "CreateFileW(%ws) failded, gle = %u", target_file_path.c_str(), GetLastError() log_end;
		return false;
	}

	char buf[] = "test_code";
	DWORD write_length;

	if (!WriteFile(create_file, 
				   buf, 
				   (uint32_t)strlen(buf),
				   &write_length, 
				   NULL))
	{
		log_err "WriteFile(%ws) failed, gle = %u", target_file_path.c_str(), GetLastError() log_end;
		CloseHandle(create_file);
		return false;
	}

	CloseHandle(create_file);
	return true;
}

/**
* @brief	target_file_path, encrypt_file_path, decrypt_file_path �� ���� ��η� �����Ѵ�.
			C:\\_test_aes256 �� ������ ������ �����ϸ�, ����� C:\\_test_aes256�� ������ �ȴ�.
*/

bool test_aes256()
{
	std::wstring target_file_path = L"C:\\_test_aes256\\ase256_test_before.conf";
	std::wstring encrypt_file_path = L"C:\\_test_aes256\\ase256_test.crypto";
	std::wstring decrypt_file_path = L"C:\\_test_aes256\\ase256_test_after.conf";
	unsigned char origin_key[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890*!=&?&/";

	//�׽�Ʈ ���� ����
	if (!create_test_sample(target_file_path))
	{
		log_err "create_test_sample failed." log_end;
		DeleteFileW(target_file_path.c_str());
		RemoveDirectoryW(L"C:\\_test_aes256");
		return false;
	}

	//�̹� ��ȣȭ ������ �����ϸ� ����
	if (is_file_existsW(encrypt_file_path.c_str()))
	{
		log_err "encrypt file exits, encrypt file delete!!" log_end;
		::DeleteFileW(encrypt_file_path.c_str());
	}

	//aes256 ��ȣȭ
	if (!aes256_encrypt(origin_key, 
						target_file_path, 
						encrypt_file_path))
	{
		log_err "aes256_encrypt() err" log_end;
		return false;
	}

	//aes256 ��ȣȭ ���� �����ϸ� ����
	if (is_file_existsW(decrypt_file_path.c_str()))
	{
		log_err "decrypt file exits, decrypt file delete!!" log_end;
		::DeleteFileW(decrypt_file_path.c_str());
	}

	//aes256 ��ȣȭ
	if (!aes256_decrypt(origin_key, 
						encrypt_file_path, 
						decrypt_file_path))
	{
		log_err "aes256_encrypt() err" log_end;
		return false;
	}
	
	log_info "[result] aes256 test folder : C:\\_test_aes256" log_end;

	return true;
}
