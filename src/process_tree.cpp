/**----------------------------------------------------------------------------
 * process_tree.cpp
 *-----------------------------------------------------------------------------
 * module that manage running process
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com)
 *-----------------------------------------------------------------------------
 * 2014:6:15 22:23 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "process_tree.h"

/// @brief	Constructor
process::process()
	:
	_process_name(L""),
	_ppid(0),
	_pid(0),
	_creation_time(0),
	_is_wow64(false),
	_full_path(L""),
	_killed(false)
{
}

/// @brief	Destructor
process::process(
	_In_ const wchar_t* process_name,
	_In_ DWORD ppid,
	_In_ DWORD pid,
	_In_ uint64_t creation_time,
	_In_ bool is_wow64,
	_In_ std::wstring& full_path,
	_In_ bool killed)
	:
	_process_name(process_name),
	_ppid(ppid),
	_pid(pid),
	_creation_time(creation_time),
	_is_wow64(is_wow64),
	_full_path(full_path),
	_killed(killed)
{
	_ASSERTE(nullptr != process_name);
	if (nullptr == process_name || 0 == wcslen(process_name))
	{
		_process_name = L"N/A";
	}
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool process::kill(_In_ DWORD exit_code, _In_ bool enable_debug_priv)
{
	_ASSERTE(true != _killed);
	if (true == _killed) return true;

	//
	//	first try without set privilege
	//
	HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, _pid);
	if (NULL == h && true == enable_debug_priv)
	{
		DWORD gle = GetLastError();
		if (true != set_privilege(SE_DEBUG_NAME, true))
		{
			log_err
				"OpenProcess() failed and no debug privilege), pid = %u, gle = %u",
				_pid,
				gle
				log_end;
			return false;
		}

		//
		//	re-try with debug privilege
		//	
		h = OpenProcess(PROCESS_TERMINATE, FALSE, _pid);
	}

	if (NULL == h)
	{
		log_err
			"OpenProcess() failed, pid = %u, gle = %u",
			_pid,
			GetLastError()
			log_end;
		return false;
	}

	if (!TerminateProcess(h, exit_code))
	{
		log_err
			"TerminateProcess() failed, pid = %u, gle = %u",
			_pid,
			GetLastError()
			log_end;
	}
	else
	{
		_killed = true;
		log_dbg "pid = %u, %ws terminated", _pid, _process_name.c_str() log_end;
	}

	_ASSERTE(NULL != h);
	CloseHandle(h); // TerminateProcess() is asynchronous, so must call CloseHandle()
	return (true == _killed) ? true : false;
}













/// @brief	Destructor
cprocess_tree::~cprocess_tree()
{
	clear_process_tree();
}

/// @brief	
void cprocess_tree::clear_process_tree()
{
	for (auto& entry : _proc_map)
	{
		_ASSERTE(entry.second);
		if (nullptr != entry.second)
		{
			delete entry.second;
		}
	}
	_proc_map.clear();
}


/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool
cprocess_tree::build_process_tree(_In_ bool enable_debug_priv)
{
	//
	// ����(SeDebugPrivilege)�� ���� ��� Ư�� ���μ���(csrss, WmiPrvSE ��)��
	// ���� �ҷ��� �ϴ� ��� ���и� �ϱ� ������ ����� ���� ������ �ʿ��ϴ�.
	// `set_debug_privilege` ���� true�� ��� ����� ������ ������ �ϰ� �ƴ� ��� ����
	// ���� �ʴ´�.
	//
	if (true == enable_debug_priv)
	{
		if (true != set_privilege(SE_DEBUG_NAME, true))
		{
			log_err "set_privilege(SE_DEBUG_NAME) failed."
				log_end;
			//
			// `SeDebugPrivilege`�� Ȱ��ȭ ���� ���� ��� ���и� ��ȯ
			// �ϰ� �Լ��� ���� ������ �ʴ� ������ `SeDebugPrivilege`
			// ������ ���ٰ� �ؼ� ���μ��� ������ ���� ���ϴ°� �ƴϱ� �����̴�.
			//

		}
	}

	clear_process_tree();

	PROCESSENTRY32W proc_entry = { 0 };
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE)
	{
		log_err "CreateToolhelp32Snapshot() failed, gle = %u", GetLastError() log_end;
		return false;
	}

#pragma warning(disable: 4127)
	do
	{
		proc_entry.dwSize = sizeof(PROCESSENTRY32W);
		if (!Process32First(snap, &proc_entry))
		{
			log_err "CreateToolhelp32Snapshot() failed, gle = %u", GetLastError() log_end;
			break;
		}

		//
		// system, idle ���μ����� ���� �ð��� ȹ�� �� �� �����Ƿ�
		// ���� �ð��� ���μ��� ���� �ð����� �����Ѵ�.
		//
		FILETIME now;
		GetSystemTimeAsFileTime(&now);

		do
		{
			BOOL IsWow64 = FALSE;
			FILETIME create_time(now);
			std::wstring full_path(_null_stringw);

			//
			// System Idle Process, System Process
			//
			if (0 == proc_entry.th32ProcessID || 4 == proc_entry.th32ProcessID)
			{
				// already initialized with default values.
				// 
				//full_path = _null_stringw;
				//create_time = now;
			}
			else
			{
				HANDLE process_handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
													FALSE,
													proc_entry.th32ProcessID);
				if (NULL == process_handle)
				{
					// too much logs.
					//log_err 
					//	"OpenProcess() failed, pid = %u, proc = %s, gle = %u", 
					//	proc_entry.th32ProcessID, 
					//	WcsToMbsEx(proc_entry.szExeFile).c_str(),
					//	GetLastError() 
					//log_end;					
				}
				else
				{
					if (!get_process_image_full_path(process_handle, full_path))
					{
						log_err "get_process_image_full_path() failed. pid=%u, process=%ws",
							proc_entry.th32ProcessID,
							proc_entry.szExeFile
							log_end;
					}

					FILETIME dummy_time;
					if (!GetProcessTimes(process_handle,
										 &create_time,
										 &dummy_time,
										 &dummy_time,
										 &dummy_time))
					{
						log_err "GetProcessTimes() failed, pid=%u, process=%ws, gle = %u",
							proc_entry.th32ProcessID,
							proc_entry.szExeFile,
							GetLastError()
							log_end;
					}

#ifdef _WIN64
					//
					// Is WoW64 process?
					//					
					if (!IsWow64Process(process_handle, &IsWow64))
					{
						log_err "IsWow64Process() failed. pid=%u, process=%ws, gle = %u",
							proc_entry.th32ProcessID,
							proc_entry.szExeFile,
							GetLastError()
							log_end;
						// assume process is not WoW64
					}
#endif
					CloseHandle(process_handle); process_handle = NULL;
				}
			}

			add_process(proc_entry.th32ParentProcessID,
						proc_entry.th32ProcessID,
						create_time,
						IsWow64,
						proc_entry.szExeFile,
						full_path);
		} while (Process32Next(snap, &proc_entry));

	} while (false);
#pragma warning(default: 4127)

	CloseHandle(snap);
	return true;
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
DWORD cprocess_tree::find_process(_In_ const wchar_t* process_name)
{
	_ASSERTE(NULL != process_name);
	if (NULL == process_name) return false;

	for (const auto& process : _proc_map)
	{
		if (rstrnicmp(process.second->process_name(), process_name))
		{
			// found
			return process.second->pid();
		}
	}

	return 0;
}

const process* cprocess_tree::get_process(_In_ DWORD pid)
{
	auto p = _proc_map.find(pid);
	if (_proc_map.end() == p)
		return nullptr;
	else
		return p->second;
}

const wchar_t* cprocess_tree::get_process_name(_In_ DWORD pid)
{
	const process* p = get_process(pid);
	if (nullptr != p)
	{
		return p->process_name();
	}
	else
	{
		return nullptr;
	}
}

const wchar_t* cprocess_tree::get_process_path(_In_ DWORD pid)
{
	const process* p = get_process(pid);
	if (nullptr != p)
	{
		return p->process_path();
	}
	else
	{
		return nullptr;
	}
}

uint64_t cprocess_tree::get_process_time(DWORD pid)
{
	const process* p = get_process(pid);
	if (nullptr != p)
	{
		return p->creation_time();
	}
	else
	{
		return 0;
	}
}

/// @brief	�θ� ���μ��� ��ü�� �����Ѵ�.
const 
process* 
cprocess_tree::get_parent(
	_In_ const process* const process
)
{
	if (process->pid() == _idle_proc_pid || process->pid() == _system_proc_pid)
	{
		return nullptr;
	}

	auto p = _proc_map.find(process->ppid());
	if (p == _proc_map.end()) return nullptr;

	if (p->second->creation_time() <= process->creation_time())
	{
		return p->second;
	}
	else
	{
		return nullptr;
	}
}

const process* cprocess_tree::get_parent(_In_ DWORD pid) 
{
	const process* me = get_process(pid);
	if (nullptr == me) return nullptr;

	return get_parent(me);
}

/// @brief
DWORD cprocess_tree::get_parent_pid(_In_ DWORD pid)
{
	const process* p = get_parent(pid);
	if (nullptr == p) return 0xffffffff;

	return p->ppid();
}

/// @brief 
const wchar_t* cprocess_tree::get_parent_name(_In_ DWORD pid)
{
	if (0 == pid || 4 == pid) return nullptr;

	const process* p = get_parent(pid);
	if (nullptr == p) return nullptr;

	return p->process_name();
}

/// @brief	��� process �� iterate �Ѵ�. 
///			callback ���� false �� �����ϸ� iterate �� �����Ѵ�.
void
cprocess_tree::iterate_process(
	_In_ on_proc_walk callback
)
{
	_ASSERTE(NULL != callback);
	if (NULL == callback) return;

	for (auto& process : _proc_map)
	{
		if (true != callback(process.second))
		{
			return;
		}
	}

}

/// @brief	������ process tree �� iterate �Ѵ�. 
///			callback ���� false �� ����(iterate ����/���) �ϰų�,
///			������ ���μ����� ã�� �� ���ų�,
///			��ȿ���� �ʴ� �Ķ���� �Է½� iterate �� �����Ѵ�.
void
cprocess_tree::iterate_process_tree(
	_In_ DWORD root_pid,
	_In_ on_proc_walk callback
)
{
	_ASSERTE(NULL != callback);
	if (NULL == callback) return;

	process_map::const_iterator it = _proc_map.find(root_pid);
	if (it == _proc_map.end()) return;

	const pprocess root = it->second;
	return iterate_process_tree(root, callback);
}

/// @brief	process map �� ��ȸ�ϸ鼭 �ݹ��Լ��� ȣ���Ѵ�. 
///			�ݹ��Լ��� false �� �����ϸ� ��ȸ�� ��� �����.
void
cprocess_tree::iterate_process_tree(
	_In_ const process* const root,
	_In_ on_proc_walk callback
)
{
	// parent first
	if (true != callback(root)) return;

	//
	//	pid == 0 �� ���μ������ recursive call �� ���� �ʴ´�. 
	//	win10 ���� toolhelp �� �̿��� ��� 
	// 
	//	`[System Process]` : 
	//	`System` 
	// 
	//	�̷��� �ΰ��� ���μ��� ������ ���ϵǴµ�, [System Process] �� ���
	//	pid, ppid, create time ���� ��� 0 �̴�. 
	//	[System Process] �� �ڽ� ���μ����� �����Ƿ� recursive call �� ���� �ʴ´�.
	//	
	if (0 == root->pid())
	{
		return;
	}

	// childs
	process_map::const_iterator its = _proc_map.begin();
	process_map::const_iterator ite = _proc_map.end();
	for (; its != ite; ++its)
	{
		//	ppid �� ���� ���������� ppid ���μ����� �̹� ����ǰ�, ���ο� ���μ����� �����ǰ�, 
		//	ppid �� �Ҵ���� ��찡 �߻��� �� �ִ�. 
		//	���� ppid ���� ������ ��� ppid �� ���� ���μ����� ���� �ð��� pid �� �����ð� ���� 
		//	�� Ŀ�� �Ѵ�.
		// 
		//	����: Jang, Hyowon (jang.hw73@gmail.com)
		//	�����ð��� ������ ��� print���� �ʴ� ���μ����� �����ϱ� ������(ex. creation_time == 0) 
		//	�����ð��� ���� �� ũ�ų� ���� ������ �ؾ� �Ѵ�.
		if (its->second->ppid() == root->pid() &&
			its->second->creation_time() >= root->creation_time())
		{
			iterate_process_tree(its->second, callback);
		}
	}
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
void cprocess_tree::print_process_tree(_In_ DWORD root_pid)
{
	process_map::iterator it = _proc_map.find(root_pid);
	if (it != _proc_map.end())
	{
		DWORD depth = 0;
		print_process_tree(it->second, depth);
		_ASSERTE(0 == depth);
	}
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
void cprocess_tree::print_process_tree(_In_ const wchar_t* root_process_name)
{
	_ASSERTE(NULL != root_process_name);
	if (NULL == root_process_name) { return; }

	DWORD pid = find_process(root_process_name);
	if (0 != pid)
	{
		print_process_tree(pid);
	}
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
void
cprocess_tree::print_process_tree(
	_In_ const process* const p,
	_In_ DWORD& depth
)
{
	std::stringstream prefix;
	for (DWORD i = 0; i < depth; ++i)
	{
		prefix << "+";
	}

	log_info
		"%spid = %u (ppid = %u), %ws ",
		prefix.str().c_str(),
		p->pid(),
		p->ppid(),
		p->process_name()
		log_end;

	// p._pid �� ppid �� ���� item �� ã��
	process_map::const_iterator it = _proc_map.begin();
	process_map::const_iterator ite = _proc_map.end();
	for (; it != ite; ++it)
	{
		// ppid �� ���� ���������� ppid ���μ����� �̹� ����ǰ�, ���ο� ���μ����� �����ǰ�, ppid �� �Ҵ���� ��찡 
		// �߻��� �� �ִ�. ���� ppid ���� ������ ��� ppid �� ���� ���μ����� ���� �ð��� pid �� �����ð� ���� �� Ŀ�� �Ѵ�.
		// ����: Jang, Hyowon (jang.hw73@gmail.com)
		// �����ð��� ������ ��� print���� �ʴ� ���μ����� �����ϱ� ������(ex. creation_time == 0) �����ð��� ���� �� ũ�ų� ���� ������ �ؾ� �Ѵ�.
		if (it->second->ppid() == p->pid() &&
			(uint64_t)it->second->creation_time() >= (uint64_t)p->creation_time())
		{
			print_process_tree(it->second, ++depth);
			--depth;
		}
	}
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool cprocess_tree::kill_process(_In_ DWORD pid, _In_ bool enable_debug_priv)
{
	if (pid == 0 || pid == 4) return false;

	process_map::iterator it = _proc_map.find(pid);
	if (it == _proc_map.end()) return true;
	process* const prcs = it->second;
	return prcs->kill(0, enable_debug_priv);
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool cprocess_tree::kill_process(_In_ const wchar_t* process_name, _In_ bool enable_debug_priv)
{
	_ASSERTE(NULL != process_name);
	if (NULL == process_name) return false;

	DWORD pid = find_process(process_name);
	return kill_process(pid, enable_debug_priv);
}

/**
 * @brief	root_pid �� �� �ڽ� ���μ����� ��� �����Ѵ�.
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
bool
cprocess_tree::kill_process_tree(
	_In_ DWORD root_pid,
	_In_ bool enable_debug_priv
)
{
	if (root_pid == 0 || root_pid == 4) return false;

	process_map::iterator it = _proc_map.find(root_pid);
	if (it == _proc_map.end()) return true;
	process* const root = it->second;

	// check process is already killed.
	if (true == root->killed())
	{
		log_info
			"already killed. pid = %u, %ws",
			root->pid(),
			root->process_name()
			log_end;
		return true;
	}

	// kill process tree include root.
	kill_process_tree(root, enable_debug_priv);
	return true;
}

/**
 * @brief
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
void
cprocess_tree::add_process(
	_In_ DWORD ppid,
	_In_ DWORD pid,
	_In_ FILETIME& creation_time,
	_In_ BOOL is_wow64,
	_In_ const wchar_t* process_name,
	_In_ std::wstring& full_path
)
{
	pprocess p = new process(process_name,
							 ppid,
							 pid,
							 *(uint64_t*)&creation_time,
							 (is_wow64 ? true : false),
							 full_path,
							 false);
	_proc_map.insert(std::make_pair(pid, p));
}

/**
 * @brief	�ڽ��� �ڽ��� �ڽı���... ��������� ���̰�, ���� �״´�. :-)

			�߰��� ���� ��������� ���� ������ �ִ� ���μ����� ���� ���� �ִ�.
			�����̴� ���� ���ΰ�, ���� �� �ִ� ���� �� ���̷���, ���ϰ��� void �� ����
 * @param
 * @see
 * @remarks
 * @code
 * @endcode
 * @return
**/
void
cprocess_tree::kill_process_tree(
	_In_ process* const root,
	_In_ bool enable_debug_priv
)
{
	// terminate child processes first if exists.
	process_map::const_iterator its = _proc_map.begin();
	process_map::const_iterator ite = _proc_map.end();
	for (; its != ite; ++its)
	{
		// ppid �� ���� ���������� ppid ���μ����� �̹� ����ǰ�, ���ο� ���μ����� �����ǰ�, ppid �� �Ҵ���� ��찡 
		// �߻��� �� �ִ�. ���� ppid ���� ������ ��� ppid �� ���� ���μ����� ���� �ð��� pid �� �����ð� ���� �� Ŀ�� �Ѵ�.
		// ����: Jang, Hyowon (jang.hw73@gmail.com)
		// �����ð��� ������ ��� print���� �ʴ� ���μ����� �����ϱ� ������(ex. creation_time == 0) �����ð��� ���� �� ũ�ų� ���� ������ �ؾ� �Ѵ�.
		if (its->second->ppid() == root->pid() &&
			its->second->creation_time() >= root->creation_time())
		{
			kill_process_tree(its->second, enable_debug_priv);
		}
	}

	// terminate parent process
	root->kill(0, enable_debug_priv);
}

