/*-----------------------------------------------------------------------------
 * CStream.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh Yong Hwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * Revision History:
 * Date					Who					What
 * ----------------		----------------	----------------
 * 22/03/2007			Noh Yong Hwan		birth
**---------------------------------------------------------------------------*/
#pragma once
#include "BaseWindowsHeader.h"
#include "log.h"

//
// �޸� ��Ʈ�� Ŭ���� 
//
typedef class CMemoryStream
{
private:
	size_t _capacity; 
	char *m_pMemory;	
	size_t m_size;
	size_t m_pos;

	DWORD _page_size;

	bool _read_only;

public:
	CMemoryStream();
	CMemoryStream(size_t size, const char* const read_only_ptr);
	
	virtual ~CMemoryStream();
	
	bool Reserve(_In_ size_t size);
	void ClearStream(void);

	// ��Ʈ�� ������ �Ҵ�� ������ (������ + ��������)
	size_t GetCapacity() { return _capacity; }

	// ��Ʈ���� ������ (��Ʈ�� ���۳� �Ҵ�� �������� ũ��)
	size_t GetSize() { return m_size; };

	// ��Ʈ���� �������� �����Ѵ�.
	size_t GetPos() { return m_pos; };

	// ��Ʈ���� �������� �̵��Ѵ�. 
	bool SetPos(_In_ size_t new_pos);
		
	// ��Ʈ�� ���� �����͸� �����Ѵ�.
	const void *GetMemory() { return m_pMemory; };
	
	// ��Ʈ������ ���� �����͸� �о ���ۿ� ����.	
	size_t ReadFromStream(_Out_ void *Buffer, _In_ size_t size);

	// ���۷κ��� �����͸� �о� ��Ʈ���� ���� �����ǿ� ����.
	size_t WriteToStream(_In_ const void *Buffer, _In_ size_t size);

	/// @brief	��Ʈ�����κ��� integer type ���� �а�, ���� ���� �����Ѵ�.
	///
	///			����.
	///			������ �߻��� ���� ���� ���� 0 �� ��찡 ������ �ȵǴµ�, 
	///			��Ʈ�� �б��� ��� ������ ���ٰ� �����Ѵ�. 
	template <typename int_type> int_type ReadInt()
	{
		int_type value;
		if (sizeof(value) != ReadFromStream((void*)&value, sizeof(value)))
		{
			return 0;
		}
		else
		{
			return value;
		}
	}

	/// @brief	��Ʈ���� integer type ���� ����, ������ true �� �����Ѵ�.
	template <typename int_type> bool WriteInt(const int_type value)
	{
		if (_read_only)
		{
			log_err "Never call on read only mode stream." log_end;
			return 0;
		}

		if (sizeof(value) != WriteToStream((const void*)&value, sizeof(value)))
		{
			return false;
		}
		else
		{
			return true;
		}
	}
private:	
	size_t IncreseSize(_In_ size_t newSize);

} *PMemoryStream;




/// @brief	��Ʈ���� ����� �ڿ��� �Ҹ��Ѵ�. 
inline void CMemoryStream::ClearStream(void)
{	
	if (!_read_only && nullptr != m_pMemory)
	{
		free(m_pMemory);
	}

	_capacity = 0;
	m_pMemory = nullptr;
	m_size = 0;
	m_pos = 0;
	_read_only = false;
}

/// @brief	��Ʈ���� �������� �̵��Ѵ�. 
///			��Ʈ���� ��ȿ�ϰ�, ��û�� ��ġ�� ��Ʈ�� �������� ��� �̵��� �����ϴ�. 
inline bool CMemoryStream::SetPos(_In_ size_t new_pos)
{
	if (m_size > 0 && m_size >= new_pos)
	{
		m_pos = new_pos;
		return true;
	}
	else
	{
		return false;
	}
}
