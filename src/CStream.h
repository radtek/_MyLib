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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sal.h>
#include <stdint.h>
#include <crtdbg.h>

//
// �޸� ��Ʈ�� Ŭ���� 
//
typedef class CMemoryStream
{
private:
	char *m_pMemory;
	unsigned long m_size;
	unsigned long m_pos; 

public:
	CMemoryStream():m_pMemory(nullptr), m_size(0ll), m_pos(0ll)
	{
	}
	
	virtual ~CMemoryStream()			
	{
		if (nullptr != m_pMemory)
		{
			if (0 != SetSize(0))
			{
				_ASSERTE(!"SetSize() error");				
			}
		}
	}

	// ��Ʈ���� ����� �ڿ��� �Ҹ��Ѵ�. 
	void ClearStream(void)
	{
		if (0 != SetSize(0))
		{
			_ASSERTE(!"SetSize() error");			
		}		
	}

	// �޸� ������ ������(�Ҵ��)�� �����Ѵ�.
	unsigned long GetSize() { return m_size; };
		
	// �޸� ���� �����͸� �����Ѵ�.
	const void *GetMemory() { return m_pMemory; };
	
	// ByteToRead ��ŭ ���� �� �ִ��� �˻�
	bool CanReadFromStream(_In_ unsigned long ByteToRead)
	{
		if (m_pos + ByteToRead > m_size)
			return false;
		else
			return true;
	}

	// ��Ʈ������ ���� �����͸� �о ���ۿ� ����.
	unsigned long ReadFromStream(_Out_ void *Buffer, _In_ unsigned long Count);

	// ���۷κ��� �����͸� �о� ��Ʈ���� ���� �����ǿ� ����.
	unsigned long WriteToStream(_In_ const void *Buffer, _In_ unsigned long Count);		
private:
	
	unsigned long SetSize(_In_ unsigned long newSize);

	unsigned long GetCurrentCusor() { return m_pos; };
	unsigned long ChangeCursor(_In_ const unsigned long offset, _In_ unsigned long from);
	
	// ChangeCursor() method ���� setPos() �� ȣ���ϸ� ��� ��ȿ�� �˻�� 
	// ChangeCursor() method ������ �����Ѵ�. 
	unsigned long setPos(_In_ const unsigned long newPosition)
	{
		m_pos = newPosition;
		return m_pos;
	}
	
	// ��Ʈ�� ���۸� �̹� �Ҵ�� �޸� ���۷� �Ҵ��Ѵ�. 
	void SetStreamBuffer(_In_ void* Buffer, _In_ unsigned long BufferSize)
	{
		ClearStream();

		m_pos = 0;
		m_size = BufferSize;
		m_pMemory = (char*)Buffer;
	}

} *PMemoryStream;



