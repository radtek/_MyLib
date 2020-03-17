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


/// @brief	Constructor
CMemoryStream::CMemoryStream()
	:
	_capacity(0),
	m_pMemory(nullptr),
	m_size(0),
	m_pos(0),
	_page_size(0),
	_read_only(false)
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

CMemoryStream::CMemoryStream(size_t size, const char* const read_only_ptr)
	:
	_capacity(size),
	m_pMemory(const_cast<char*>(read_only_ptr)),
	m_size(size),
	m_pos(0),
	_page_size(4096),// read_only ��忡���� �ʿ���, �⺻������ ����
	_read_only(true)
{
}

/// @brief	Destructor
CMemoryStream::~CMemoryStream()
{
	ClearStream();
}

/// @brief	�޸� ���۸� size ��ŭ �̸� �Ҵ��صд�.
///			�޸� ���۰� �̹� �Ҵ�Ǿ���, ������� ��Ʈ���� ���ؼ��� 
///			Reserve() �� ȣ�� �� �� ����. 
bool CMemoryStream::Reserve(_In_ size_t size)
{
	if (_read_only)
	{
		log_err "Never call on read only mode stream." log_end;
		return 0;
	}
	
	_ASSERTE(0 == _capacity);
	if (_capacity > 0 || m_size > 0 || m_pos > 0 || nullptr != m_pMemory)
	{
		return false;
	}

	return (IncreseSize(size) >= size ? true : false);
}

/// @brief	���� ������ �� ũ�� newSize �� Ű���. 
///			������ ������ ������(newSize)�� �����ϰ�, 
///			���н� 0 �� �����ϰ�, ���� ���� ���۴� �״�� �д�.
size_t 
CMemoryStream::IncreseSize(
	_In_ size_t newSize
)
{
	if (_read_only)
	{
		log_err "Never call on read only mode stream." log_end;
		return 0;
	}

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
			log_err
				"No resources for stream. req size=%u",
				new_size
				log_end;
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

/// @brief	`size` ��ŭ `Buffer` �� �����ϰ�, ��Ʈ�� �������� size ��ŭ �̵�
/// @return	������ `size` �� �����ϰ�, ��Ʈ���� �б� ������ ������ `size` ���� 
///			������ 0 (����)�� ����
size_t 
CMemoryStream::ReadFromStream(
	_Out_ char* const Buffer, 
	_In_ const size_t size
)
{
	_ASSERTE(size > 0);
	_ASSERTE(size <= m_size);
	if (size == 0 || size > m_size)
	{
		log_err
			"Invalid parameter. =%zu, m_size=%zu",
			size,
			m_size
			log_end;
		return 0;
	}
	
	size_t cb_can_read = m_size - m_pos;
	if (size > cb_can_read)
	{
		log_err
			"Invalid request. available=%zu, size=%zu",
			cb_can_read,
			size
			log_end;
		return 0;
	}
		
	RtlCopyMemory(Buffer, 
		          (char *)(DWORD_PTR(m_pMemory) + m_pos), 
				  size);
	m_pos += size;
	return size;
}

/// @brief	`Buffer` �� ���� ��Ʈ���� �����͸� �����ϰ�, `size` ��ŭ ��Ʈ�� 
///			�������� �̵�
/// @return	������ `size` �� �����ϰ�, ��Ʈ���� �б� ������ ������ `size` ���� 
///			������ 0 (����)�� ����
size_t
CMemoryStream::RefFromStream(
	_Out_ const char*& Buffer,
	_In_ const size_t size
)
{
	_ASSERTE(size > 0);
	_ASSERTE(size <= m_size);
	if (size == 0 || size > m_size)
	{
		log_err
			"Invalid parameter. =%zu, m_size=%zu",
			size,
			m_size
			log_end;
		return 0;
	}

	size_t cb_can_read = m_size - m_pos;
	if (size > cb_can_read)
	{
		log_err
			"Invalid request. available=%zu, size=%zu",
			cb_can_read,
			size
			log_end;
		return 0;
	}

	Buffer = &m_pMemory[m_pos];
	m_pos += size;
	return size;
}


/// @brief	���۷κ��� �����͸� �о� ��Ʈ���� ���� �����ǿ� ����.
///			������ Write �� ����Ʈ ��
///			���н� 0
size_t 
CMemoryStream::WriteToStream(
	const char* Buffer, 
	size_t size
)
{
	if (_read_only)
	{
		log_err "Never call on read only mode stream." log_end;
		return 0;
	}

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

/// @brief	��Ʈ���� std::string �� ����.
bool CMemoryStream::WriteString(_In_ const std::string& str)
{
	//
	//	�� string ��ü��� ������(0) ���� ��Ʈ���� ����.
	//
	if (str.empty())
	{
		return WriteInt<size_t>(0);
	}

	//
	//	��Ʈ���� string �� ����Ʈ ���� ����.
	//	
	if (true != WriteInt<size_t>(str.size() * sizeof(char)))
	{
		return false;
	}

	//
	//	Writes contents of the input string.
	//
	return WriteToStream((char*)str.c_str(), str.size()*sizeof(char));
}

/// @brief	
bool CMemoryStream::WriteWstring(_In_ const std::wstring& wstr)
{
	//
	//	�� string ��ü��� ������(0) ���� ��Ʈ���� ����.
	//
	if (wstr.empty())
	{
		return WriteInt<size_t>(0);
	}

	//
	//	��Ʈ���� string �� ����Ʈ ���� ����.
	//	
	if (true != WriteInt<size_t>(wstr.size() * sizeof(wchar_t)))
	{
		return false;
	}

	//
	//	Writes contents of the input string.
	//
	return WriteToStream((char*)wstr.c_str(), wstr.size() * sizeof(wchar_t));
}

/// @brief	
bool CMemoryStream::WriteString(_In_ const char* const str)
{	
	size_t cc = strlen(str);	
	if (nullptr == str || 0 == cc)
	{
		return WriteInt<size_t>(0);
	}

	//
	//	��Ʈ���� string �� ����Ʈ ���� ����.
	//		
	if (true != WriteInt<size_t>(cc * sizeof(char)))
	{
		return false;
	}

	//
	//	Writes contents of the input string.
	//
	return WriteToStream((char*)str, cc * sizeof(char));
}

/// @brief	
bool CMemoryStream::WriteWstring(_In_ const wchar_t* const wstr)
{
	size_t wcc = wcslen(wstr);
	if (nullptr == wstr || 0 == wcc)
	{
		return WriteInt<size_t>(0);
	}

	//
	//	��Ʈ���� string �� ����Ʈ ���� ����.
	//		
	if (true != WriteInt<size_t>(wcc * sizeof(wchar_t)))
	{
		return false;
	}

	//
	//	Writes contents of the input string.
	//
	return WriteToStream((char*)wstr, wcc * sizeof(wchar_t));
}

/// @brief	Reads std::string from stream.
std::string CMemoryStream::ReadString()
{
	size_t size = ReadInt<size_t>();
	if (size > 0)
	{
		const char* p = nullptr;
		if (size != RefFromStream(p, size))
		{
			return _null_stringa;
		}

		return std::string(p, size/sizeof(char));
	}
	else
	{
		return _null_stringa;
	}
}

/// @brief	
std::wstring CMemoryStream::ReadWstring()
{
	size_t size = ReadInt<size_t>();
	if (size > 0)
	{
		_ASSERTE(0 == size % sizeof(wchar_t));
		if (0 != size % sizeof(wchar_t)) return _null_stringw;

		const char* p = nullptr;
		if (size != RefFromStream(p, size))
		{
			return _null_stringw;
		}

		return std::wstring((wchar_t*)p, size / sizeof(wchar_t));
	}
	else
	{
		return _null_stringw;
	}
}
