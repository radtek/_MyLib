/**
 * @file    test_CStream.cpp
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2019/10/05 09:51 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include "_MyLib/src/CStream.h"

bool test_cstream()
{
	_mem_check_begin
	{
		CMemoryStream strm;

		// ��Ʈ���� ���� �Ҵ���� ����
		_ASSERTE(!strm.SetPos(0));

		_ASSERTE(true == strm.Reserve(1024));
		_ASSERTE(strm.GetCapacity() >= 1024);
		_ASSERTE(0 == strm.GetPos());
		_ASSERTE(strm.GetSize() == strm.GetPos());

		//
		//	���ڿ� ����
		//
		const char* test_string = "0123456789";
		size_t size = strlen(test_string) * sizeof(char);

		_ASSERTE(strm.WriteInt<size_t>(size));
		_ASSERTE(size == strm.WriteToStream(test_string, size));
		
		size_t pos = sizeof(size) + size;
		_ASSERTE(pos == strm.GetPos());
		_ASSERTE(strm.GetSize() == strm.GetPos());
		_ASSERTE(strm.GetCapacity() >= pos);

		//
		//	������ ����/�б�
		//
		_ASSERTE(strm.WriteInt<uint8_t> (0x11));
		_ASSERTE(strm.WriteInt<uint16_t>(0x1122));
		_ASSERTE(strm.WriteInt<uint32_t>(0x11223344));
		_ASSERTE(strm.WriteInt<uint64_t>(0x1122334411223344));

		// ��Ʈ�� ���� ������ �̵��� ����
		_ASSERTE(false == strm.SetPos(strm.GetPos() + 1));

		//
		//	�б� & ���� (����, �б�)
		//
		{
			_ASSERTE(strm.SetPos(0));
			size = strm.ReadInt<size_t>();

			//	��Ʈ�� ���ڿ� ����
			pos = strm.GetPos();

			const char* p = nullptr;
			_ASSERTE(size >= strm.RefFromStream(p, size));
			_ASSERTE(strm.GetPos() > pos);

			std::string s_ref((char*)p, size);
			log_info "ref from stream=%s", s_ref.c_str() log_end;

			_ASSERTE(0x11 == strm.ReadInt<uint8_t>());
			_ASSERTE(0x1122 == strm.ReadInt<uint16_t>());
			_ASSERTE(0x11223344 == strm.ReadInt<uint32_t>());
			_ASSERTE(0x1122334411223344 == strm.ReadInt<uint64_t>());
		}

		//
		//	�б� & ���� (����, �б�)
		//
		{
			_ASSERTE(strm.SetPos(0));
			size = strm.ReadInt<size_t>();

			auto ptr = std::make_unique<char[]>(size);
			_ASSERTE(ptr);
			_ASSERTE(size >= strm.ReadFromStream(ptr.get(), size));
			std::string s(ptr.get(), size);
			log_info "read from stream=%s", s.c_str() log_end;

			_ASSERTE(0x11 == strm.ReadInt<uint8_t>());
			_ASSERTE(0x1122 == strm.ReadInt<uint16_t>());
			_ASSERTE(0x11223344 == strm.ReadInt<uint32_t>());
			_ASSERTE(0x1122334411223344 == strm.ReadInt<uint64_t>());
		}

		
		

		strm.ClearStream();
		_ASSERTE(0 == strm.GetPos());
		_ASSERTE(0 == strm.GetSize());
		_ASSERTE(0 == strm.GetCapacity());
		_ASSERTE(nullptr == strm.GetMemory());
	}
	_mem_check_end;

	return true;
}



bool test_cstream_read_only()
{
	_mem_check_begin
	{
		CMemoryStream strm(1024, (const char* const)GetModuleHandleW(nullptr));

		// ��Ʈ���� �Ҵ�Ǿ����Ƿ� SetPos ����
		_ASSERTE(strm.SetPos(1024));
		_ASSERTE(!strm.SetPos(1024 + 1));
		_ASSERTE(strm.SetPos(0));

		_ASSERTE(true != strm.Reserve(1024));
		_ASSERTE(strm.GetCapacity() == 1024);
		_ASSERTE(0 == strm.GetPos());
		_ASSERTE(strm.GetSize() == 1024);

		//
		//	���ڿ� ���� ����
		//
		const char* test_string = "0123456789";
		size_t size = strlen(test_string) * sizeof(char);

		_ASSERTE(!strm.WriteInt<size_t>(size));
		_ASSERTE(size != strm.WriteToStream(test_string, size));

		//
		//	������ ����/�б� ���� 
		//
		_ASSERTE(!strm.WriteInt<uint8_t>(0x11));
		_ASSERTE(!strm.WriteInt<uint16_t>(0x1122));
		_ASSERTE(!strm.WriteInt<uint32_t>(0x11223344));
		_ASSERTE(!strm.WriteInt<uint64_t>(0x1122334411223344));

		//
		//	�б� & ����
		//
		_ASSERTE(strm.SetPos(0));

		_ASSERTE(strm.ReadInt<uint8_t>() > 0);
		_ASSERTE(strm.ReadInt<uint16_t>() > 0);
		_ASSERTE(strm.ReadInt<uint32_t>() > 0);
		_ASSERTE(strm.ReadInt<uint64_t>() > 0);


		strm.ClearStream();
		_ASSERTE(0 == strm.GetPos());
		_ASSERTE(0 == strm.GetSize());
		_ASSERTE(0 == strm.GetCapacity());
		_ASSERTE(nullptr == strm.GetMemory());
	}
	_mem_check_end;

	return true;
}