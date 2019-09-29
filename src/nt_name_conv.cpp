/**
 * @file    nt path name -> win32 name converter
 * @brief   
 * @ref     https://msdn.microsoft.com/ko-kr/library/windows/desktop/aa385453(v=vs.85).aspx
 *          https://msdn.microsoft.com/windows/hardware/drivers/ifs/support-for-unc-naming-and-mup
 *          http://pdh11.blogspot.kr/2009/05/pathcanonicalize-versus-what-it-says-on.html
 *          https://github.com/processhacker2/processhacker2/blob/master/phlib/native.c
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2016.06.14 15:36 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "nt_name_conv.h"
#include "Singleton.h"


/// @brief  
bool NameConverter::load(_In_ bool force_reload)
{
	boost::lock_guard<boost::mutex> lock(_lock);
	if (true == _loaded && true != force_reload) return true;
		
	if (true != load_env_values())
	{
		log_err "load_env_values() failed. " log_end;
		return false;
	}

	if (true != update_dos_device_prefixes())
	{
		log_err "update_dos_device_prefixes() failed." log_end;
		return false;
	}

	if (true != update_mup_device_prefixes())
	{
		log_err "update_mup_device_prefixes() failed." log_end;
		return false;
	}
    return true;
}

/// @brief  nt path name �� dos path name ���� ��ȯ�Ѵ�. 
///
///			\??\c:\windows\system32\abc.exe->c:\windows\system32\abc.exe
///			\Systemroot\abc.exe->C:\WINDOWS\abc.exe
///			system32\abc.exe->C:\WINDOWS\system32\abc.exe
///			\windows\system32\abc.exe->C:\WINDOWS\system32\abc.exe
///			x:\->x:\		(network mapped drive)
///			\Program Files\ -> c:\ProgramFiles
///			\Program Files (x86) -> c:\Program Files (x86)
///
///			\Device\Mup\1.1.1.1\abc.exe -> \\1.1.1.1\abc.exe
///			\Device\Unknown\aaaa.exe -> \Device\Unknown\aaaa.exe
///			\device\lanmanredirector\;x:000000000008112d\192.168.153.144\sfr022\ -> \\192.168.153.144\sfr022\
///			\Device\Mup\192.168.153.144\sfr022\ -> \\192.168.153.144\sfr022\
///			\Device\WebDavRedirector\192.168.0.1\DavWWWRoot\ -> \\192.168.0.1\davwwwroot\
///			\Device\Mup\192.168.0.1\ -> \\192.168.0.1\
///			\Device\Mup\192.168.0.1\temp.*\ -> \\192.168.0.1\temp.*\
///			\Device\Mup\192.168.59.134\ADMIN$\PSEXESVC.EXE -> \\192.168.59.134\admin$\psexesvc.exe
///			\Device\Mup\; Csc\.\.\ -> \Device\csc\.\.\
///			\Device\Mup\;          WebDavRedirector\ -> \Device\webdavredirector\
///			\Device\Mup\; WebDavRedirector\192.168.0.1\DavWWWRoot\ -> \Device\webdavredirector\192.168.0.1\davwwwroot\
///			\Device\Mup\; RdpDr\; :1\192.168.59.134\IPC$\ -> \\192.168.59.134\ipc$\
///			\Device\Floppy0\temp\123.txt -> \Device\Floppy0\temp\123.txt
///			\Device\CdRom0\temp\123.txt -> \Device\CdRom0\temp\123.txt
///		
///			\Device\HarddiskVolumeShadowCopy222 -> \Device\HarddiskVolumeShadowCopy222
///
///			���� ���� �Ǵ� ����̽� ������ ���� root ��� ������ ��� %SystemDrive% �� �ٿ��ش�. 
///			(���� ���ϴ� �����̰�, `\` �� �����ϴ� ��θ��� ���)
///				\Users\ -> c:\users
///				\UnknownPath -> c:\UnknownPath
///				\Program Files -> c:\Program Files
///				\Program Files (x86) -> c:\Program Files (x86)
///				\windows -> c:\windows
bool
NameConverter::get_canon_name(
	_In_ const wchar_t* file_name, 
	_Out_ std::wstring& canonical_file_name
	)
{
    _ASSERTE(NULL != file_name);
	if (NULL == file_name) return false;

    uint32_t cch_file_name = (uint32_t)wcslen(file_name);
    uint32_t cch_canon_file = 0;
    
	//
	//	\Device\HardDiskVolume1\... -> c:\...
	//
	if (true == lstrnicmp(file_name, L"\\device"))
	{
		return resolve_device_prefix(file_name, canonical_file_name);
	}

	//
    //	"\??\" refers to \GLOBAL??\. Just remove it.
	//
    else if (true == lstrnicmp(file_name, L"\\??\\"))
    {
        cch_canon_file = cch_file_name - 4;
		wchar_ptr canon_file(
			(wchar_t*)malloc((cch_canon_file + 1) * sizeof(wchar_t)), 
			[](wchar_t* p) {
				if (nullptr != p)
				{
					free(p);
				}
		});
        if (nullptr == canon_file.get())
		{
			log_err "Not enough memory. size=%u", 
				(cch_canon_file + 1) * sizeof(wchar_t)
				log_end;
			return false;
		}

        RtlCopyMemory(canon_file.get(), &file_name[4], (cch_canon_file * sizeof(wchar_t)));
        canon_file.get()[cch_canon_file] = 0x0000;
		canonical_file_name = canon_file.get();
		return true;
    }
	
	//
    //	"\SystemRoot" means "C:\Windows".    
	//
    else if (true == lstrnicmp(file_name, L"\\SystemRoot"))
    {
        cch_canon_file = (cch_file_name - 11) + (uint32_t)_system_root.size();        
		wchar_ptr canon_file(
			(wchar_t*)malloc((cch_canon_file + 1) * sizeof(wchar_t)),
			[](wchar_t* p) 
		{		
			if (nullptr != p)
			{
				free(p);
			}
		});
		if (nullptr == canon_file.get())
		{
			log_err "Not enough memory. size=%u",
				(cch_canon_file + 1) * sizeof(wchar_t)
				log_end;
			return false;
		}

        RtlCopyMemory(canon_file.get(), _system_root.c_str(), (_system_root.size() * sizeof(wchar_t)));
        RtlCopyMemory(&canon_file.get()[_system_root.size()], &file_name[11], (cch_file_name - 11) * sizeof(wchar_t));
        canon_file.get()[cch_canon_file] = 0x0000;
		canonical_file_name = canon_file.get();
		return true;
    }

	//
    //	"system32\" means "C:\Windows\system32\"
	//
    else if (true == lstrnicmp(file_name, L"system32\\"))
    {
		std::wstringstream strm;
		strm << _system_root << L"\\" << file_name;
		canonical_file_name = strm.str();
		return true;
    }

	//
	//	�������� �Ǵ� ����̽� ���� ���� root ��� ������
	//	%SystemDrive% �� �ٿ��ش�. 
	//
	//	\windows -> c:\windows
	//	\Program Files, \Program Files (x86) -> c:\Program Files...
	//	\Users -> c:\Users
	//	...
	//
	else if (file_name[0] == L'\\')
	{
		std::wstringstream strm;
		strm << _system_drive << file_name;
		canonical_file_name = strm.str();
		return true;
	}
	else
    {
        // unknown
		return false;
    }
}

/// @brief  nt_name( ex. \Device\HarddiskVolume4\Windows\...\MicrosoftEdge.exe ) 
///         �� removeable drive �̸� true �� �����Ѵ�.
///			���� nt_name �� �˼� ���� �̸��̶��  false �� �����Ѵ�. 
bool
NameConverter::is_removable_drive(
	_In_ const wchar_t* nt_name
	)
{
	_ASSERTE(NULL != nt_name);
	if (NULL == nt_name) return false;

	//
	//	���� ���ڿ� ���̸� ���
	//	input: \Device\HarddiskVolume4\
	//        ^      ^               ^  : `\` �� 3�� ������������ ���̸� ���Ѵ�. (`\` ����)
	//                                    `\' �� ���� Volume44 == Volume4 �� ���������� �νĵ� �� ����
	//
	//	ex) \\Device\\HarddiskVolume4\\Windows -> \\Device\\HarddiskVolume4\\
	//      \\Device\\HarddiskVolume4          -> \\Device\\HarddiskVolume4
	//      \\Device\\HarddiskVolume455\\xyz   -> \\Device\\HarddiskVolume455\\
	//
	uint32_t cmp_count = 0;
	uint32_t met_count = 0;
	uint32_t max_count = (uint32_t)wcslen(nt_name);
	for (cmp_count = 0; cmp_count <= max_count; ++cmp_count)
	{
		if (met_count == 3) break;
		if (nt_name[cmp_count] == L'\\')
		{
			++met_count;
		}
	}

	boost::lock_guard<boost::mutex> lock(_lock);
	for (const auto& drive_info : _dos_devices)
	{
		if (0 != _wcsnicmp(drive_info._device_name.c_str(),
						   nt_name,
						   cmp_count))
		{
			continue;
		}

		return DRIVE_REMOVABLE == drive_info._drive_type ? true : false;
	}

	return false;
}

/// @brief  nt_name( ex. \\Device\\Mup\\192.168.153.144\\sfr022\\ ) 
///         �� network �̸� true �� �����Ѵ�.
///			���� nt_name �� �˼� ���� �̸��̶��  false �� �����Ѵ�. 
bool
NameConverter::is_network_path(
	_In_ const wchar_t* nt_name
)
{
	_ASSERTE(NULL != nt_name);
	if (NULL == nt_name) return false;

	boost::lock_guard<boost::mutex> lock(_lock);
	for (const auto& mup_device : _mup_devices)
	{
		if (0 != _wcsnicmp(mup_device.c_str(),
						   nt_name,
						   mup_device.length()))
		{
			continue;
		}

		return true;
	}
	
	return false;
}

/// @brief	DosDevice �� iterate �Ѵ�.
///			callback �Լ��� false �� �����ϸ� iterate �� �����ϰ�
///			�����Ѵ�. 
bool 
NameConverter::iterate_dos_devices(
	_In_ iterate_dos_device_callback callback,
	_In_ ULONG_PTR tag
	)
{
	_ASSERTE(nullptr != callback);
	if (nullptr == callback) return false;

	boost::lock_guard<boost::mutex> lock(_lock);
	for (const auto& ddi : _dos_devices)
	{
		if (!callback(&ddi, tag))
		{
			log_dbg "Iterate canceled." log_end;
			return true;
		}
	}
	return true;
}

/// @brief	c:\windows\system32\test.txt -> \Device\HarddiskVolume1\windows\test.txt �� ��ȯ
///
///			[warning]
///			c: (drive letter) -> \Device\HardDiskVolume1 (Device name) ��ȯ�� 
///			get_device_name_by_drive_name() �޼ҵ� �̿�
bool
NameConverter::get_nt_path_by_dos_path(
	_In_ const wchar_t* dos_path, 
	_Out_ std::wstring& nt_device_path
	)
{
	_ASSERTE(dos_path);
	if (nullptr == dos_path) return false;

	// 
	// dos_path �� �ּ��� `c:\` �����̾�� �Ѵ�. 
	// 
	if (3 > wcslen(dos_path) || dos_path[2] != L'\\')
	{
		log_err "Invalid dos_path format. dos_path=%ws",
			dos_path
			log_end;
		return false;
	}

	bool found = false;
	std::wstringstream out_path;
	{
		boost::lock_guard<boost::mutex> lock(_lock);
		for (const auto& ddi : _dos_devices)
		{
			//
			//	`c:` �� ���ڸ� ��
			// 
			if (0 == _wcsnicmp(ddi._logical_drive.c_str(), dos_path, 2))
			{
				out_path << ddi._device_name;
				found = true;
				break;
			}
		}
	}
	
	if (true != found)
	{
		log_err "Can not find device name. dos_path=%ws",
			dos_path
			log_end;
		return false;
	}

	// 
	//	dos_path =                      c:\windows\system32\test.txt
	//	out_path = \device\harddiskvolume1\
	// 
	out_path << &dos_path[3];
	nt_device_path = out_path.str();
	return true;
}

/// @brief	c: (drive letter) -> \Device\HardDiskVolume1 (Device name) ��ȯ
bool 
NameConverter::get_device_name_by_drive_letter(
	_In_ const wchar_t* drive_letter,
	_Out_ std::wstring& device_name
	)
{
	_ASSERTE(nullptr != drive_letter);
	if (nullptr == drive_letter) return false;
	
	// 
	// drive_letter �� �ݵ�� `c:` �����̾�� �Ѵ�. 
	// 
	if (2 != wcslen(drive_letter) || drive_letter[1] != L':')
	{
		log_err "Invalid drive letter. drive_letter=%ws",
			drive_letter
			log_end;
		return false;
	}

	bool found = false;	
	{
		boost::lock_guard<boost::mutex> lock(_lock);
		for (const auto& ddi : _dos_devices)
		{
			//
			//	`c:` �� ���ڸ� ��
			// 
			if (0 == _wcsnicmp(ddi._logical_drive.c_str(), drive_letter, 2))
			{
				//
				//	ddi._device_name �� �׻� `\` ����Ǳ� ������ ������ `\` ���ڸ�
				//	�����ؼ� �����Ѵ�. 
				//
				device_name = ddi._device_name.substr(0, ddi._device_name.size() - 1);
				found = true;
				break;
			}
		}
	}
	
	if (true != found)
	{
		log_err "Can not find device name. drive letter=%ws",
			drive_letter
			log_end;
		return false;
	}

	return true;
}

/// @brief	nt device name ���ڿ��� �޾Ƽ� drive letter �� �����Ѵ�. 
///	@param	device_name		
///				1) \Device\HardDiskVolume1\windows\system32\calc.exe
///				2) \Device\HardDiskVolume1\
///				3) \Device\HardDiskVolume1
///				������ �Է��� ��� �����Ѵ�. 
///				1), 2) ���� �Է��� ��� `\Device\HardDiskVolume1\` �κи� ����Ѵ�. 
/// @prarm	device_name �� ��Ī�Ǵ� `c:` ������ ����̺� ���͸� �����Ѵ�.
bool 
NameConverter::get_drive_letter_by_device_name(
	_In_ const wchar_t* device_name,
	_Out_ std::wstring& drive_letter
	)
{
	_ASSERTE(nullptr != device_name);
	if (nullptr == device_name) return false;
	
	// 
	//	device name �� ����ȭ�Ѵ�.
	// 
	//  - \Device, Device �� '\' �� 2�� �̸��̸� invalid input
	//	- \Device\HarddiskVolume4 ��� \Device\HarddiskVolume4\ �� ��ȯ
	//  - \Device\HarddiskVolume4\, \Device\HarddiskVolume4\aaa ��� 
	//	  \Device\HarddiskVolume4\ ������ ���
	//
	int32_t back_slash_count = 0;
	int32_t compare_count = 0;
	int32_t max_count = (int32_t)wcslen(device_name);
	for (compare_count = 0; compare_count <= max_count; ++compare_count)
	{
		if (back_slash_count == 3) break;
		if (device_name[compare_count] == L'\\')
		{
			++back_slash_count;
		}
	}
	compare_count = min(max_count, compare_count);

 	if (back_slash_count < 2 || compare_count < 0)
	{
		log_err "Invalid device name format. device_name=%ws", device_name log_end;
		return false;
	}

	bool found = false;
	boost::lock_guard<boost::mutex> lock(_lock);
	for (const auto& ddi : _dos_devices)
	{
		if (0 != _wcsnicmp(ddi._device_name.c_str(), device_name, compare_count))
		{
			continue;
		}

		// 
		//	compare_count ��ŭ�� ���ϱ� ������ \Device\HarddiskVolume4 ������ �Է��� ���
		//	\Device\HarddiskVolume4\ �� \Device\HarddiskVolume44\ �� �� ��� ��Ī�� �� �ִ�.
		//	���� ���� ��Ī�� DosDeviceInfo::_device_name[compare_count+1] �� \ ���� Ȯ���ؾ� �Ѵ�.
		// 
		if (back_slash_count == 2)
		{
			if (ddi._device_name[compare_count+1] != L'\\')
			{
				found = true;
				drive_letter = ddi._logical_drive;
				break;
			}
		}
		else
		{
			found = true;
			drive_letter = ddi._logical_drive;
			break;
		}
	}

	if (true != found)
	{
		log_err "Can not find drive letter, device_name=%ws",
			device_name
			log_end;
		return false;
	}

	return true;
}

/// @brief  device prefix �� �ν�, ������ ��ȯ�ؼ� �����Ѵ�. 
/// @remark dos_device prefix �� ���
///             \device\harddiskvolume1\abc.exe -> c:\abc.exe
///
///          local drive �� ���ε� ��� (DRIVE_REMOTE Ÿ�� ����̺��� ���)
///             x: -> \\192.168.153.144\share 
///             
///         mup_device �� ���
///             \device\mup\1.1.1.1\share\abc.exe -> \\1.1.1.1\share\abc.exe
bool 
NameConverter::resolve_device_prefix(
	_In_ const wchar_t* file_name,
	_Out_ std::wstring& resolved_file_name
)
{
	_ASSERTE(NULL != file_name);
	if (NULL == file_name || file_name[0] != L'\\') return false;


	// file_name ���ڿ� ����ȭ 
	//
	//  \\Device, Device �� '\' �� 2�� �̸��̸� invalid input
	//  \\Device\\HarddiskVolume4    ��� \\Device\\HarddiskVolume4\\ �� ��ȯ
	//  \\Device\\HarddiskVolume4\\, \\Device\\HarddiskVolume4\\aaa ��� �״�� ���
	uint32_t cmp_count = 0;
	uint32_t met_count = 0;
	uint32_t max_count = (uint32_t)wcslen(file_name);
	for (cmp_count = 0; cmp_count <= max_count; ++cmp_count)
	{
		if (met_count == 3) break;
		if (file_name[cmp_count] == L'\\')
		{
			++met_count;
		}
	}

	std::wstringstream strm;
	switch (met_count)
	{
	case 0:
	case 1:
		log_err "invalid nt_name, nt_name=%ws", file_name log_end;
		return false;
	case 2:
		// '\' �ϳ� �� �ٿ��ش�.            
		strm << file_name << L"\\";
		break;
	case 3:
		strm << file_name;
		break;
	}

	std::wstring revised_file_name = strm.str();
	std::wstring small_rfn = revised_file_name;
	to_lower_string(small_rfn);

	//
	// #0, \Device\HarddiskVolumeShadowCopy ��� �״�� �����Ѵ�.
	//
	if (lstrnicmp(L"\\Device\\HarddiskVolumeShadowCopy", small_rfn.c_str(), true))
	{
		resolved_file_name = file_name;
		return true;
	}

	boost::lock_guard<boost::mutex> lock(this->_lock);

	//
	//	#1, is this a dos device?
	// 	
    for (const auto& dos_device : _dos_devices)
    {
		// 
        //	\Device\HarddiskVolume1, \Device\HarddiskVolume11 �� ��Ī���� �ʵ���
        //	dos_device._device_name �ʵ�� `\Device\HarddiskVolume1\` ó�� `\` �� ������.
		//

		std::wstring smal_dn = dos_device._device_name;
		to_lower_string(smal_dn);

        size_t pos = small_rfn.find(smal_dn);
        if (pos == 0)
        {
            if (DRIVE_REMOTE == dos_device._drive_type)
            {
                resolved_file_name = dos_device._network_name;
                resolved_file_name += L"\\";
                resolved_file_name += revised_file_name.substr(dos_device._device_name.size(),
															   revised_file_name.size());

            }
            else
            {
                resolved_file_name = dos_device._logical_drive;
                resolved_file_name += L"\\";
                resolved_file_name += revised_file_name.substr(dos_device._device_name.size(),
															   revised_file_name.size());
            }

            return true;
        }
    }

	//
	//	#2. is this a mup device? (network providers)
	//  ';' �� 0���� ��� mup prefix�� ����
	//				ex)	\Device\Mup\192.168.0.1\						->\\192.168.0.1\
	//		   1���� ��� �տ� \\Device prefix �߰�
	//				ex)	\Device\Mup\; WebDavRedirector\					-> \Device\WebDavRedirector\
	//				ex)	\Device\Mup\;      WebDavRedirector\			-> \Device\WebDavRedirector\
	//		   2���� ��� �տ� \\Device prefix �߰�
	//				ex)	\Device\Mup\; RdpDr\; :1\192.168.59.134\IPC$\	-> \\192.168.59.134\IPC$\
	//				ex)	\Device\Mup\; RdpDr\; :xxxx\192.168.59.134\IPC$\-> \\192.168.59.134\IPC$\
	//
	for (const auto& mup_device : _mup_devices)
	{
		if (0 != revised_file_name.find(mup_device))
		{
			continue;
		}

		std::vector<std::wstring> tokens;
		if (!split_stringw(small_rfn.c_str(), L";", tokens))
		{
			log_err "split_stringw( %ws ) failed.", small_rfn.c_str() log_end;
			return false;
		}

		std::wstring mup_path;
		size_t pos = 0;
		switch (tokens.size())
		{
			case 1:
			{
				mup_path = tokens[0];
				pos = (uint32_t)wcslen(mup_device.c_str());

				if (std::string::npos == mup_path.find(_device_path))
				{
					resolved_file_name = L"\\";
				}
				break;
			}
			case 2:
			{
				mup_path = tokens[1];
				pos = mup_path.find_first_not_of(L" ");

				if (std::string::npos == mup_path.find(_device_path))
				{
					resolved_file_name = _device_path;
				}
				break;
			}
			case 3:
			{
				mup_path = tokens[2];
				pos = mup_path.find(L'\\');

				if (std::string::npos == mup_path.find(_device_path))
				{
					resolved_file_name = L"\\";
				}
				break;
			}
			default:
			{
				return false;
			}
		}

		resolved_file_name += mup_path.substr(pos, mup_path.size());
		return true;
	}

    // #3, no match    
    return false;
}

/// @brief 
bool NameConverter::load_env_values()
{
	std::wstring value;

	//
	//	%SystemDrive% ȯ�溯�� �б�
	// 
	if (get_environment_value(L"%SystemDrive%", value))
	{
		to_lower_string(value);
		_system_drive = value;
	}

	//
	//	%SystemRoot%
	//
	if (get_environment_value(L"%SystemRoot%", value))
	{
		to_lower_string(value);
		_system_root = value;
	}


	return true;
}


/// @brief  
bool NameConverter::update_dos_device_prefixes()
{
	this->_dos_devices.clear();

	// �ý��ۿ� ���εǾ��ִ� ����̺� ����� ���Ѵ�. 
	// 
	// 0               4               8               12
	// +---+---+---+---+---+---+---+---+---+---+---+---+
	// | A | : | \ |NUL| C | : | \ |NUL| D | : | \ |NUL|
	//  "A:\"           "C:\"           "D:\"
	// 
	// ���ε� ����̺긦 ��Ÿ���� ���ڿ� ���۴� 
	// 26 (���ĺ�) * 4 (����̺� ��) = 104 ����Ʈ�� �����
	// �����ڵ��� ��� 208����Ʈ 	
    wchar_t drive_string[128 + 1] = { 0 };
    DWORD length = GetLogicalDriveStringsW(128, drive_string);
    if (0 == length)
    {
        log_err "GetLogicalDriveStringsW(), gle = %u", GetLastError() log_end;
        return false;
    }

	//
    // ���ε� �� ����̺��� device name �� ���Ѵ�. 
	// 
    for (DWORD i = 0; i < length / 4; ++i)
    {
        wchar_t* dos_device_name = &(drive_string[i * 4]);
        dos_device_name[2] = 0x0000;	// `c:` ���·� ����
                                        // floppy �� �����Ѵ�. �ȱ׷��� �޼��� �ڽ� ���� 
										// �� �غ� �ȵǾ����� ��¼���� ��.
        if (dos_device_name[0] == 'A' || dos_device_name[0] == 'a') { continue; }

		std::wstring nt_device;
        if (true != query_dos_device(dos_device_name, nt_device))
        {
            log_err
                "query_dos_device( dos_device_name = %s )",
                dos_device_name
                log_end;
            return false;
        }
        nt_device += L"\\";

        // drive type �� ���Ѵ�. 
        std::wstring network_name;
        std::wstring drive_root = dos_device_name;
        drive_root += L"\\";
        uint32_t drive_type = GetDriveTypeW(drive_root.c_str());
        if (DRIVE_REMOTE == drive_type)
        {
            DWORD cch_rmt = 0;
            wchar_t* rmt = NULL;

            DWORD ret = WNetGetConnection(dos_device_name, NULL, &cch_rmt);
            if (ERROR_MORE_DATA == ret)
            {
                rmt = (wchar_t*)malloc((cch_rmt + 1) * sizeof(wchar_t));
                if (NULL != rmt)
                {
                    if (NO_ERROR == WNetGetConnection(dos_device_name, rmt, &cch_rmt))
                    {
                        network_name = rmt;
						free(rmt);
                    }
                }
            }
        }

        // _drive_table �� ����Ѵ�. 
        DosDeviceInfo ddi(dos_device_name, nt_device.c_str(), drive_type, network_name.c_str());
        _dos_devices.push_back(ddi);
    }

    return true;
}

/// @brief  ��ϵ� MUP device �̸����� �о�´�. 
///         https://msdn.microsoft.com/windows/hardware/drivers/ifs/support-for-unc-naming-and-mup
bool NameConverter::update_mup_device_prefixes()
{
    // #1, HKLM\System\CurrentControlSet\Control\NetworkProvider\Order :: ProviderOrder (REG_SZ) ���� �д´�. 
    std::wstring provider_order;
	HKEY key_handle = RUOpenKey(HKEY_LOCAL_MACHINE,
								L"System\\CurrentControlSet\\Control\\NetworkProvider\\Order",
								true);
	if (nullptr == key_handle)
	{
		log_err "RUOpenKey() failed. key=%ws",
			L"System\\CurrentControlSet\\Control\\NetworkProvider\\Order"
			log_end;
		return false;
	}
	
    if (!RUReadString(key_handle, 
					  L"ProviderOrder",
					  provider_order))
    {
        log_err "RUReadString( value=ProviderOrder ) failed." log_end;

		RUCloseKey(key_handle);
        return false;
    }
	RUCloseKey(key_handle);



    _mup_devices.push_back(L"\\Device\\Mup");       // default mup device

    // #2, provider �� device �̸��� �����´�. 
    // 
    //  provider_order = RDPNP, LanmanWorkstation, webclient
    // 
    //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\[provider]\NetworkProvider :: DeviceName (REG_SZ)
    // 
    std::vector<std::wstring> providers;
    if (!split_stringw(provider_order.c_str(), L",", providers))
    {
        log_err "split_stringw( provider_order=%ws ) failed.", provider_order.c_str() log_end;
        return false;
    }

    for (const auto& provider : providers)
    {
        std::wstring device_name;
        std::wstringstream key;
        key << L"SYSTEM\\CurrentControlSet\\Services\\" << provider << L"\\NetworkProvider";

		key_handle = RUOpenKey(HKEY_LOCAL_MACHINE,
							   key.str().c_str(),
							   true);
		if (nullptr == key_handle)
		{
			log_err "RUOpenKey() failed. key=%ws",
				key.str().c_str()				
				log_end;
			continue;
		}

        if (!RUReadString(key_handle, 
						  L"DeviceName",
						  device_name))
        {
            log_err "RUReadString( DeviceName ) failed." log_end;

			RUCloseKey(key_handle);
            continue;
        }

        this->_mup_devices.push_back(device_name);
    }
	
    return true;
}

/// @brief	NameConverter �ν��Ͻ��� ref count �� ������Ų��.
bool nc_initialize()
{
	NameConverter* nc = Singleton<NameConverter>::GetInstancePointer();
	if (true != nc->load(false))
	{
		return false;
	}
	return true;
}

/// @brief	NameConverter �ν��Ͻ��� ref count �� ���ҽ�Ų��.
void nc_finalize()
{
	Singleton<NameConverter>::ReleaseInstance();
}

/// @brief	NameConver wrappera api
bool nc_reload()
{
	std::unique_ptr<NameConverter, void(*)(_In_ NameConverter*)> nc(
		Singleton<NameConverter>::GetInstancePointer(),
		[](_In_ NameConverter*) 
	{
		Singleton<NameConverter>::ReleaseInstance();
	});
	return nc.get()->load(true);
}

/// @brief	NameConver wrappera api
bool 
nc_get_canon_name(
	_In_ const wchar_t* file_name, 
	_Out_ std::wstring& canonical_file_name
	)
{
	std::unique_ptr<NameConverter, void(*)(_In_ NameConverter*)> nc(
		Singleton<NameConverter>::GetInstancePointer(),
		[](_In_ NameConverter*)
	{
		Singleton<NameConverter>::ReleaseInstance();
	}); 
	return nc.get()->get_canon_name(file_name, canonical_file_name);
}

/// @brief	NameConver wrappera api
bool 
nc_get_nt_path_by_dos_path(
	_In_ const wchar_t* dos_path,
	_Out_ std::wstring& nt_device_path
	)
{
	std::unique_ptr<NameConverter, void(*)(_In_ NameConverter*)> nc(
		Singleton<NameConverter>::GetInstancePointer(),
		[](_In_ NameConverter*)
	{
		Singleton<NameConverter>::ReleaseInstance();
	});
	return nc.get()->get_nt_path_by_dos_path(dos_path, nt_device_path);
}

/// @brief	NameConver wrappera api
bool 
nc_get_device_name_by_drive_letter(
	_In_ const wchar_t* drive_letter,
	_Out_ std::wstring& device_name
	)
{
	std::unique_ptr<NameConverter, void(*)(_In_ NameConverter*)> nc(
		Singleton<NameConverter>::GetInstancePointer(),
		[](_In_ NameConverter*)
	{
		Singleton<NameConverter>::ReleaseInstance();
	});
	return nc.get()->get_device_name_by_drive_letter(drive_letter, device_name);
}

/// @brief	NameConver wrappera api
bool 
nc_get_drive_letter_by_device_name(
	_In_ const wchar_t* device_name,
	_Out_ std::wstring& drive_letter
	)
{
	std::unique_ptr<NameConverter, void(*)(_In_ NameConverter*)> nc(
		Singleton<NameConverter>::GetInstancePointer(),
		[](_In_ NameConverter*)
	{
		Singleton<NameConverter>::ReleaseInstance();
	});
	return nc.get()->get_drive_letter_by_device_name(device_name, drive_letter);
}

/// @brief	NameConver wrappera api
bool nc_is_removable_drive(_In_ const wchar_t* nt_name)
{
	std::unique_ptr<NameConverter, void(*)(_In_ NameConverter*)> nc(
		Singleton<NameConverter>::GetInstancePointer(),
		[](_In_ NameConverter*)
	{
		Singleton<NameConverter>::ReleaseInstance();
	});
	return nc.get()->is_removable_drive(nt_name);
}

/// @brief	NameConver wrappera api
bool nc_is_network_path(_In_ const wchar_t* nt_name)
{
	std::unique_ptr<NameConverter, void(*)(_In_ NameConverter*)> nc(
		Singleton<NameConverter>::GetInstancePointer(),
		[](_In_ NameConverter*)
	{
		Singleton<NameConverter>::ReleaseInstance();
	});
	return nc.get()->is_network_path(nt_name);
}

/// @brief	NameConver wrappera api
bool 
nc_iterate_dos_devices(
	_In_ iterate_dos_device_callback callback,
	_In_ ULONG_PTR tag
	)
{
	std::unique_ptr<NameConverter, void(*)(_In_ NameConverter*)> nc(
		Singleton<NameConverter>::GetInstancePointer(),
		[](_In_ NameConverter*)
	{
		Singleton<NameConverter>::ReleaseInstance();
	});
	return nc.get()->iterate_dos_devices(callback, tag);
}