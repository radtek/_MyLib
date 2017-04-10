/**
 * @file    Windows registry api wrapper
 * @brief   
 * @ref     
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011/11/11 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#pragma once

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

HKEY 
RUOpenKey(
	_In_ HKEY RootKey, 
	_In_ const wchar_t* SubKey,
	_In_ bool ReadOnly);

void
RUCloseKey(
	_In_ HKEY Key
	);

HKEY 
RUCreateKey(
	_In_ HKEY RootKey,
	_In_ const wchar_t* SubKey,
	_In_ bool ReadOnly
	);

DWORD 
RUReadDword(	
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_In_ DWORD DefaultValue
	);

bool 
RUWriteDword(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_In_ DWORD value
	);

bool 
RUReadString(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_Out_ std::wstring& value
	);

bool 
RUSetString(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_In_ const wchar_t* value
	);

bool 
RUSetExpandString(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_In_ const wchar_t* value, 
	_In_ DWORD cbValue
	);

bool 
RUSetBinaryData(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_In_ const uint8_t* value, 
	_In_ DWORD cbValue
	);

uint8_t* 
RUReadBinaryData(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name, 
	_Out_ DWORD& cbValue
	);

bool 
RUDeleteValue(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* value_name
	);

bool 
RUDeleteKey(
	_In_ HKEY key_handle, 
	_In_ const wchar_t* sub_key
	);

bool 
RUIsKeyExists(
	_In_ HKEY root_key, 
	_In_ const wchar_t* sub_key
	);

typedef
bool(*fn_key_callback)(
    _In_ uint32_t index,
    _In_ const wchar_t* sub_key_name,
    _In_ const wchar_t* class_name
    );

typedef
bool(*fn_value_callback)(
    _In_ uint32_t index,
    _In_ uint32_t value_type,
    _In_ const wchar_t* value_name,
    _In_ uint32_t value_data_size,
    _In_ const uint8_t* value_data
    );

bool
reg_enum_key_values(
    _In_ HKEY key,
    _In_ fn_key_callback key_cb,
    _In_ fn_value_callback value_cb
    );

class RegHandle
{
public:
    RegHandle(HKEY key) : m_key(key) {}
	~RegHandle()    { if (NULL != m_key) RegCloseKey(m_key);  }
    HKEY get()      { return m_key; };
protected:
private:
	HKEY m_key;
};
