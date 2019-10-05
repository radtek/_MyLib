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

/// @brief	
unsigned long 
CMemoryStream::ChangeCursor(
	_In_ const unsigned long offset, 
	_In_ unsigned long from
)
{
	_ASSERTE(0 <= offset);
	_ASSERTE(0 <= from);
	_ASSERTE(m_pos >= from);
	_ASSERTE(_max_ulong >= from); 
	_ASSERTE(_max_ulong >= offset); 
	_ASSERTE(_max_ulong >= from + offset);

	if (	
			!(0 <= offset) ||
			!(0 <= from) ||
			!(m_pos >= from) ||
			!(_max_ulong >= from) ||
			!(_max_ulong >= offset) || 
			!(_max_ulong >= from + offset)
		)
	{
		return _max_ulong;
	}

	unsigned long newPosition = from + offset;		

	// position �� ��Ʈ���� ũ�⺸�� ũ�� ���������� Ŀ���� �ű��. 
	//
	if (newPosition > m_size)
	{
		newPosition = m_size;		
	}

	setPos(newPosition);		
	return newPosition;
};


/// @brief	memory stream �� ũ�⸦ �����Ѵ�.
unsigned long CMemoryStream::SetSize(_In_ unsigned long newSize)
{
	unsigned long oldPosition = GetCurrentCusor();
	char *ptr=NULL;

	if (0 == newSize)
	{
		free(m_pMemory); m_pMemory=NULL;		
	}
	else
	{
		ptr = (char *) realloc(m_pMemory, newSize);
		if (NULL == ptr)
		{
			// �޸𸮰� ������.
			// m_pMemory �� ������� ����. 
			// 
			char log[512]={0};
			StringCbPrintfA(log, 
							sizeof(log), 
							"%s(), can not reallocate memory, new memory size=%u bytes", 
							__FUNCTION__, 
							newSize);
			OutputDebugStringA(log);
			return _max_ulong;
		}
		
		m_pMemory = ptr;
	}
	
	m_size = newSize;
	if (oldPosition > newSize) ChangeCursor(0, newSize);
	
	return newSize;
}



 
/**	-----------------------------------------------------------------------
	\brief	��Ʈ������ ���� �����͸� �о ���ۿ� ����.

	\param	
	\return			
			������ ���� ����Ʈ �� ���� 
			(��Ʈ���� Count ���� ���� ��� ����)
			���н� -1 ����
	\code
	
	\endcode		
-------------------------------------------------------------------------*/
unsigned long CMemoryStream::ReadFromStream(_Out_ void *Buffer, _In_ unsigned long Count)
{
	_ASSERTE(nullptr != Buffer);
	if (nullptr == Buffer) return 0;

	if ( (m_pos >= 0) && (Count >= 0) )	
	{
		unsigned long ret = m_size - m_pos;
		if (0 < ret)
		{
			if (ret > Count) ret = Count;

			RtlCopyMemory(Buffer, (char *)(DWORD_PTR(m_pMemory) + m_pos), ret);
			ChangeCursor(ret, m_pos);
			return ret;
		}
	}

	return 0;
}

/**	-----------------------------------------------------------------------
	\brief	���۷κ��� �����͸� �о� ��Ʈ���� ���� �����ǿ� ����.

	\param	
	\return			
			������ Write �� ����Ʈ ��
			���н� -1
	\code
	
	\endcode		
-------------------------------------------------------------------------*/
unsigned long CMemoryStream::WriteToStream(const void *Buffer, unsigned long Count)
{
	if ( (m_pos >= 0) && (Count >= 0) )
	{
		unsigned long pos = m_pos + Count;
		if (pos > 0)
		{
			if (pos > m_size)
			{
				if (_max_ulong == SetSize(pos))
				{
					return _max_ulong;
				}
			}

			RtlCopyMemory(&m_pMemory[m_pos], Buffer, Count);
			this->m_pos = pos;
			return Count;
		}	  
	}

	return 0;	// write �� ����Ʈ�� 0 �̹Ƿ�
}



///// @brief	
//unsigned long CMemoryStream::ReadUint16FromStream(_Out_ uint16_t& value)
//{
//	return ReadFromStream((void*)&value, sizeof(uint16_t));
//}
//
///// @brief	
//unsigned long CMemoryStream::WriteUint16ToStream(_In_ uint16_t value)
//{
//	return WriteToStream(&value, sizeof(uint16_t));
//}
//
//
///// @brief	
//unsigned long CMemoryStream::ReadUint32FromStream(_Out_ uint32_t& value)
//{
//	return ReadFromStream((void*)&value, sizeof(uint32_t));
//}
//
///// @brief	
//unsigned long CMemoryStream::WriteUint32ToStream(_In_ uint32_t value)
//{
//	return WriteToStream(&value, sizeof(uint32_t));
//}
//


