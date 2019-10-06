/*-----------------------------------------------------------------------------
 * CStream.cpp
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
#include "stdafx.h"
#include "CStream.h"
#include <strsafe.h>

/// @brief	Constructor
CMemoryStream::CMemoryStream() 
	:
	_capacity(0),
	m_pMemory(nullptr), 
	m_size(0), 
	m_pos(0), 
	_page_size(0)
{
	SYSTEM_INFO si = { 0 };
	GetSystemInfo(&si);
	_page_size = si.dwPageSize;

	_ASSERTE(_page_size > 0);
	if (_page_size == 0)
	{
		_page_size = 4096;
	}
}

/// @brief	�޸� ���۸� size ��ŭ �̸� �Ҵ��صд�.
///			�޸� ���۰� �̹� �Ҵ�Ǿ���, ������� ��Ʈ���� ���ؼ��� 
///			Reserve() �� ȣ�� �� �� ����. 
bool CMemoryStream::Reserve(_In_ size_t size)
{
	_ASSERTE(0 == _capacity);
	if (_capacity > 0 || m_size > 0 || m_pos > 0 || nullptr != m_pMemory)
	{
		return false;
	}

	return (IncreseSize(size) >= size ? true : false);
}

/// @brief	Destructor
CMemoryStream::~CMemoryStream()
{
	ClearStream();
}

/// @brief	���� ������ �� ũ�� newSize �� Ű���. 
///			������ ������ ������(newSize)�� �����ϰ�, 
///			���н� 0 �� �����ϰ�, ���� ���� ���۴� �״�� �д�.
size_t 
CMemoryStream::IncreseSize(
	_In_ size_t newSize
)
{
	if (0 == newSize)
	{
		if (nullptr == m_pMemory)
		{
			free(m_pMemory); 
			m_pMemory = nullptr;
		}

		_capacity = 0;
		m_size = 0;
		m_pos = 0;
	}
	else
	{
		//
		//	��Ʈ���� ����� ���̴� ���� �Ұ�����
		//
		if (m_pos > newSize)
		{
			return 0;
		}

		//
		//	��û�� ����� ������ ������� �ø��Ѵ�.
		//
		size_t new_size = newSize;
		_ASSERTE(_page_size > 0);			
		uint32_t remain = newSize % _page_size;
		if (0 != remain)
		{
			new_size += (_page_size - remain);
		}		
		
		char *ptr = (char *) realloc(m_pMemory, new_size);
		if (nullptr == ptr)
		{
			// �޸� ����, m_pMemory �� ������� ����. 
			char log[512]={0};
			StringCbPrintfA(log, 
							sizeof(log), 
							"%s(), can not reallocate memory, new memory size=%u bytes", 
							__FUNCTION__, 
							newSize);
			OutputDebugStringA(log);
			return 0;
		}
		else
		{
			_capacity = new_size;
			m_pMemory = ptr;			
		}
	}
	
	return _capacity;
}

/// @brief	��Ʈ������ ���� �����͸� �о ���ۿ� ����.
///			������ ���� ����Ʈ �� ���� (��Ʈ���� size ���� ���� ��� ����)
///			���н� 0 ����
size_t 
CMemoryStream::ReadFromStream(
	_Out_ void *Buffer, 
	_In_ size_t size
)
{
	_ASSERTE(nullptr != Buffer);
	if (nullptr == Buffer) return 0;

	if ( (m_pos >= 0) && (size >= 0) )	
	{
		size_t cb_read = m_size - m_pos;
		if (cb_read > 0)
		{
			if (cb_read > size) cb_read = size;
		
			RtlCopyMemory(Buffer, (char *)(DWORD_PTR(m_pMemory) + m_pos), cb_read);
			m_pos += cb_read;
			return cb_read;
		}
	}

	return 0;
}

/// @brief	���۷κ��� �����͸� �о� ��Ʈ���� ���� �����ǿ� ����.
///			������ Write �� ����Ʈ ��
///			���н� 0
size_t 
CMemoryStream::WriteToStream(
	const void *Buffer, 
	size_t size
)
{
	_ASSERTE(nullptr != Buffer);
	_ASSERTE(size > 0);
	if (nullptr == Buffer || size == 0 || (size_t)(-1) == size)
	{
		return 0;
	}

	if ( (m_pos >= 0) && (size >= 0) )
	{
		size_t pos = m_pos + size;
		if (pos > 0)
		{
			if (pos > m_size)
			{
				if (0 == IncreseSize(pos))
				{
					return 0;
				}
			}

			RtlCopyMemory(&m_pMemory[m_pos], Buffer, size);

			m_pos = pos;
			m_size = m_pos;
			return size;
		}	  
	}

	return 0;	// write �� ����Ʈ�� 0 �̹Ƿ�
}

