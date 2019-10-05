/**
 * @file    test_unique_ptr.cpp
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2017/10/18 09:51 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"

typedef class Foo
{
public:
	Foo(const char* str_ptr) : foo_str(str_ptr)
	{
	}

	~Foo()
	{
		log_info "destructor, %s", foo_str.c_str() log_end;
	}

	std::string foo_str;
} *PFoo;


bool test_unique_ptr()
{
	_mem_check_begin
	{
		std::map<std::string, std::unique_ptr<Foo> > fooz;

		char buf[128];
		for (int i = 0; i < 10; ++i)
		{
			StringCbPrintfA(buf, sizeof(buf), "index=%d", i);

			fooz.insert(std::make_pair(std::string(buf),
									   std::make_unique<Foo>(buf)));
		}

		//
		// for (auto entry : fooz)  ó�� entry �� �����ڷ� ���� ������
		// `attempting to reference a deleted function` ������ ���� �߻�
		// unique_ptr �� ���簡 ���� �ʱ� �����ε�
		//
		for (auto& entry : fooz)
		{
			log_info "%s", entry.second.get()->foo_str.c_str() log_end;
		}
	}
	_mem_check_end;

	return true;
}


bool test_unique_ptr_assign()
{
	_mem_check_begin
	{
		std::unique_ptr<char[]> y = std::make_unique<char[]>(1024);
		std::unique_ptr<char[]> x = std::make_unique<char[]>(512);

		// unique_ptr �� ���簡 �ȵ�
		// std::move �� �ؾ� ��
		y = std::move(x);
			// y �� �̹� �Ҵ�Ǿ��ִ� �޸𸮴� x �� �̵��Ҷ� �Ҹ� ��
			// x �����ʹ� y �� �̵��Ǿ���, y �Ҹ�� ���� ��

		
		//
		//	������ �׽�Ʈ............
		//	�Ҹ��� ȣ���� ���������� �Ǵ��� Ȯ��
		//

		std::unique_ptr<Foo> yyyy = std::make_unique<Foo>("yyyy");
		std::unique_ptr<Foo> xxxx = std::make_unique<Foo>("xxxx");

		// unique_ptr �� ���簡 �ȵ�
		// std::move �� �ؾ� ��
		yyyy = std::move(xxxx);
		// y �� �̹� �Ҵ�Ǿ��ִ� �޸𸮴� x �� �̵��Ҷ� �Ҹ� ��
		// x �����ʹ� y �� �̵��Ǿ���, y �Ҹ�� ���� ��
	}
	_mem_check_end;

	return true;	
}