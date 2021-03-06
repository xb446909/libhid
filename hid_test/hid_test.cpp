// hid_test.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../libhid/libhid.h"
#include <Windows.h>
#include <iostream>

#ifdef _DEBUG
#pragma comment(lib, "../Debug/libhid.lib")
#else
#pragma comment(lib, "../Release/libhid.lib")
#endif // _DEBUG


#define VID	0x0483
#define PID 0x5750

int main()
{
	char buf[30] = { 0 };
	
	void* h = open_dev(VID, PID);
	if (h)
	{
		while (true)
		{
			for (size_t i = 0; i < sizeof(buf); i++)
			{
				buf[i] = i + 1;
			}
			std::cout << "Write " << write_bytes(h, (unsigned char*)buf, sizeof(buf)) << " bytes." << std::endl;
			Sleep(1000);
			std::cout << "Read " << read_bytes(h, (unsigned char*)buf, sizeof(buf)) << " bytes." << std::endl;
			Sleep(1000);
		}
	}
    return 0;
}

