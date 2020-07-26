/**
 * @file    
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2020/07/26 09:51 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"

template <typename T>
void func_has_template_param(
	_In_ const char* name,
	_In_ const T& value)
{
	std::cout << name << "=" << value << ", ";
	std::cout << "type=(" << typeid(value).name() << ")" << ", ";
	std::cout << "size=" << sizeof(value);
	std::cout << std::endl;	
}


bool test_template()
{
	//
	//	�˾Ƽ� ����ȯ�� �Ǵ� Ÿ�Ե��� template parameter �� ������� 
	//	�ʾƵ� ������ ��
	//	@@@ ������ �ǵ����� ���� Ÿ������ ĳ���õǰų� �ǵ����� ���� 
	//		��Ȳ�� �� �� �����Ƿ� ��������� ���°� ���� @@@
	//
	
	// OK
	//uint32_t = 32, type = (unsigned int), size = 4
	//uint32_t = 32, type = (int), size = 4	
	func_has_template_param<uint32_t>("uint32_t", 32);
	func_has_template_param("uint32_t", 32);

	
	// int64 vs int
	//uint64_t = 64, type = (unsigned __int64), size = 8
	//uint64_t = 64, type = (int), size = 4
	func_has_template_param<uint64_t>("uint64_t", 64);
	func_has_template_param("uint64_t", 64);
	
	// char* vs char[]
	//char *= char* type value, type = (char * __ptr64), size = 8
	//char *= char* type value, type = (char const[17]), size = 17
	func_has_template_param<char*>("char*", "char* type value");
	func_has_template_param("char*", "char* type value");
	
	
	// wchar_t* vs wchar_t[]
	//wchar_t *= 00007FF7677ACBA0, type = (wchar_t * __ptr64), size = 8
	//wchar_t *= 00007FF7677ACBA0, type = (wchar_t const[20]), size = 40
	func_has_template_param<wchar_t*>("wchar_t*", L"wchar_t* type value");
	func_has_template_param("wchar_t*", L"wchar_t* type value");
		
	return true;
}