/**
 * @file    _MyLib test 
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2017/08/13 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include "_MyLib/src/process_tree.h"
#include "_MyLib/src/log.h"
#include <vector>

/**
 * @brief	test for cprocess_tree class 
			
			�׽�Ʈ�� ���ؼ��� 
			cmd.exe -> procexp.exe -> procexp64.exe(�ڵ����� �������) -> notepad.exe
			������ ���μ����� ������ �ΰ� �ؾ� �Ѵ�. 
**/
bool proc_tree_callback(_In_ const process* const process_info)
{
    log_info "pid = %u, name = %ws, path = %ws", 
		process_info->pid(),
		process_info->process_name(),
		process_info->process_path()
		log_end
	return true;
}

/// @brief	
bool test_iterate_process_tree()
{
	_mem_check_begin
	{
		cprocess_tree proc_tree;
		if (!proc_tree.build_process_tree(true)) return false;

		///	�θ� ���μ����� ���� ���μ��� ����� ���� �����Ѵ�. 
		std::vector<const process*> top_level_procs;
		proc_tree.iterate_process([&](_In_ const process* const process_info)->bool
		{
			const process* p = proc_tree.get_parent(process_info);
			if (nullptr == p)
			{
				top_level_procs.push_back(process_info);
			}

			return true;
		});

		/// top_level_proces �� �� �ڽ� ���μ������� �θ�->�ڽ� ������ iterate �Ѵ�. 
		size_t count = 0;
		for (auto& top_level_proc : top_level_procs)
		{
			log_info
				"processes tree under pid=%u, name=%ws, path=%ws",
				top_level_proc->pid(),
				top_level_proc->process_name(),
				top_level_proc->process_path()
				log_end;

			proc_tree.iterate_process_tree(top_level_proc,
										   [&](_In_ const process* const process_info)->bool
			{
				log_info "    pid = %u, name = %ws, path = %ws",
					process_info->pid(),
					process_info->process_name(),
					process_info->process_path()
					log_end;

				count++;
				return true;
			});
		}
		_ASSERTE(proc_tree.size() == count);
	}
	_mem_check_end;	
	return true;
}

bool test_process_tree()
{
	_mem_check_begin
	{
		cprocess_tree proc_tree;
		if (!proc_tree.build_process_tree(true)) return false;

		// ���μ��� ���� �׽�Ʈ (by callback)
		proc_tree.iterate_process(proc_tree_callback);
		proc_tree.find_process(L"cmd.exe", 
							   [&](_In_ const process* const process_info)->bool 
		{
			proc_tree.iterate_process_tree(process_info->pid(),
										   proc_tree_callback);
			return true;
		});
		

		// ���μ��� ���� �׽�Ʈ (by lambda)
		proc_tree.iterate_process([](_In_ const process* const process_info)->bool
		{
			log_info "pid = %u, name = %ws, path = %ws",
				process_info->pid(),
				process_info->process_name(),
				process_info->process_path()
				log_end
				return true;
		});

		// ���μ��� ���� �׽�Ʈ (by boost::function, lambda with capture)
		int count = 0;
		auto callback = [&count](_In_ const process* const process_info)->bool
		{
			log_info "pid = %u, name = %ws, path = %ws",
				process_info->pid(),
				process_info->process_name(),
				process_info->process_path()
				log_end;
			++count;
			return true;
		};
		proc_tree.iterate_process(callback);
		_ASSERTE(count > 0);

		// ���μ��� ���� �׽�Ʈ (by lambda with capture local variable)
		count = 0;
		proc_tree.iterate_process([&count](_In_ const process* const process_info)->bool
		{
			log_info "pid = %u, name = %ws, path = %ws",
				process_info->pid(),
				process_info->process_name(),
				process_info->process_path()
				log_end;
			count++;
			return true;
		});
		_ASSERTE(count > 0);

		// 
		//	run notepad.exe & kill & ...
		// 
		PROCESS_INFORMATION pi = { 0 };
		STARTUPINFOW si = { 0 }; si.cb = sizeof(si);
		if (!CreateProcessW(L"c:\\windows\\system32\\notepad.exe",
							nullptr,
							nullptr,
							nullptr,
							FALSE,
							0,
							nullptr,
							nullptr,
							&si,
							&pi))
		{
			log_err "CreateProcessW() failed. gle=%u",
				GetLastError()
				log_end;
			return false;
		}
		Sleep(1000);

		log_info "kill notepad..." log_end;
		_ASSERTE(true == proc_tree.build_process_tree(false));
		proc_tree.print_process_tree(L"notepad.exe");

		proc_tree.find_process(L"notepad.exe", 
							   [&](const process* process_info)->bool 
		{
			proc_tree.kill_process_tree(process_info->pid(), false);
			return true;
		});

		////
		////	print process tree
		////
		//log_info "print explorer family..." log_end;
		//proc_tree.print_process_tree(L"explorer.exe");
	}
	_mem_check_end;
	return true;
}

/// @brief	
bool test_image_path_by_pid()
{
	_mem_check_begin
	{
		cprocess_tree proc_tree;
		if (!proc_tree.build_process_tree(true)) return false;

		proc_tree.find_process(L"explorer.exe", [&](const process* process)->bool
		{
			std::wstring win32_path;
			std::wstring native_path;
			if (!image_path_by_pid(process->pid(), true, win32_path)) return false;
			if (!image_path_by_pid(process->pid(), false, native_path)) return false;

			log_info "pid=%u, explorer.exe, \nwin32_path=%ws\nnative_path=%ws",
				process->pid(),
				win32_path.c_str(),
				native_path.c_str()
				log_end;

			return false; // No all process, just one for test!
		});

		
	}
	_mem_check_end;

	return true;
}

/// @brief	
bool test_get_process_creation_time()
{
	_mem_check_begin
	{
		cprocess_tree proc_tree;
		if (!proc_tree.build_process_tree(true)) return false;

		proc_tree.find_process(L"explorer.exe", [](const process* process) 
		{
			FILETIME creation_time;
			if (!get_process_creation_time(process->pid(), 
										   &creation_time)) return false;

			log_info "pid=%u, explorer.exe, creation_time=%s",
				process->pid(),
				file_time_to_str(&creation_time, true, true).c_str()
				log_end;

			return false; // No all process, just one for test!
		});

		
	}
	_mem_check_end;


	return true;
}