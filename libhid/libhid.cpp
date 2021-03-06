// libhid.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <string>
#include <SetupAPI.h>
#include "libhid.h"
#include <iostream>

#pragma comment(lib, "setupapi.lib")

void LoadHid();
void UnloadHid();
std::string GetHidDevice(unsigned short uVID, unsigned short  uPID);

/* HID Declared  */
typedef struct _HIDD_ATTRIBUTES {
	ULONG   Size; // = sizeof (struct _HIDD_ATTRIBUTES)

				  //
				  // Vendor ids of this hid device
				  //
	USHORT  VendorID;
	USHORT  ProductID;
	USHORT  VersionNumber;

	//
	// Additional fields will be added to the end of this structure.
	//
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

typedef void (__stdcall *fHidD_GetHidGuid)(_Out_  LPGUID HidGuid);
typedef BOOLEAN (__stdcall *fHidD_GetAttributes)(IN  HANDLE HidDeviceObject, OUT PHIDD_ATTRIBUTES Attributes);

fHidD_GetHidGuid HidD_GetHidGuid = NULL;
fHidD_GetAttributes HidD_GetAttributes = NULL;

/* HID Declared  */


HANDLE DeviceHandle;

std::string GetHidDevice(unsigned short uVID, unsigned short  uPID)
{
	DWORD Requred = 0;
	DWORD MemberIndex = 0;
	DWORD NeedLength = 0;
	GUID HidGuid;

	HDEVINFO hDevInfo;
	SP_DEVICE_INTERFACE_DATA devInfoData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detailData;

	if ((HidD_GetAttributes == NULL) || (HidD_GetAttributes == NULL))
	{
		return std::string();
	}

	HidD_GetHidGuid(&HidGuid);
	hDevInfo = SetupDiGetClassDevs(&HidGuid, NULL, NULL,
		DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		return std::string();
	}

	devInfoData.cbSize = sizeof(devInfoData);

	BOOL Result;

	while (1)
	{
		Result = SetupDiEnumDeviceInterfaces
		(hDevInfo, NULL, &HidGuid, MemberIndex, &devInfoData);
		if (!Result)
		{
			SetupDiDestroyDeviceInfoList(hDevInfo);
			return std::string();
		}

		MemberIndex++;

		Result = SetupDiGetDeviceInterfaceDetail
		(hDevInfo, &devInfoData, NULL, 0, &Requred, NULL);
		if (!Result)
		{
			if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
			{
				SetupDiDestroyDeviceInfoList(hDevInfo);
				return std::string();
			}

		}

		NeedLength = Requred;
		detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(NeedLength);
		detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		Result = SetupDiGetDeviceInterfaceDetail
		(hDevInfo, &devInfoData, detailData, NeedLength, &Requred, NULL);
		if (!Result)
		{
			free(detailData);
			SetupDiDestroyDeviceInfoList(hDevInfo);
			return FALSE;
		}

		DeviceHandle = CreateFile(detailData->DevicePath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (DeviceHandle == INVALID_HANDLE_VALUE)
		{
			free(detailData);
			CloseHandle(DeviceHandle);
			continue;
		}

		HIDD_ATTRIBUTES Attributes;
		HidD_GetAttributes(DeviceHandle, &Attributes);
		if ((Attributes.VendorID == uVID) && (Attributes.ProductID == uPID))
		{
			std::string path(detailData->DevicePath);
			free(detailData);
			return path;
		}
	}
}

HMODULE g_hHid = NULL;

void LoadHid()
{
	g_hHid = LoadLibraryA("hid.dll");
	if (g_hHid == NULL)
	{
		std::cerr << "Failed to load hid.dll!" << std::endl;
		return;
	}
	HidD_GetHidGuid = (fHidD_GetHidGuid)GetProcAddress(g_hHid, "HidD_GetHidGuid");
	if (!HidD_GetHidGuid)
	{
		std::cerr << "Failed to load HidD_GetHidGuid!" << std::endl;
		return;
	}
	HidD_GetAttributes = (fHidD_GetAttributes)GetProcAddress(g_hHid, "HidD_GetAttributes");
	if (!HidD_GetAttributes)
	{
		std::cerr << "Failed to load HidD_GetAttributes!" << std::endl;
		return;
	}
}

void UnloadHid()
{
	if (g_hHid)
	{
		FreeLibrary(g_hHid);
		g_hHid = NULL;
	}
}

void* __stdcall open_dev(unsigned short vid, unsigned short pid)
{
	std::string path = GetHidDevice(vid, pid);
	HANDLE hDev = CreateFile(path.c_str(), 
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (INVALID_HANDLE_VALUE == hDev)
	{
		std::cerr << "Failed to open device! error: " << GetLastError() << std::endl;
		return nullptr;
	}
	return hDev;
}

void __stdcall close_dev(void* h)
{
	if (nullptr != h)
	{
		CloseHandle(h);
		h = nullptr;
	}
}

int __stdcall write_bytes(void* h, unsigned char * bytes, int count)
{
	if (nullptr == h)
	{
		std::cerr << "Please open device first!" << std::endl;
		return -1;
	}

	unsigned char write_buf[65] = { 0 };
	memcpy(write_buf + 1, bytes, min(count, 64));

	DWORD numBytesWriten;
	if (!WriteFile(h, write_buf, 65, &numBytesWriten, NULL))
	{
		std::cerr << "Failed to write data! error: " << GetLastError() << std::endl;
		return -1;
	}

	return min(numBytesWriten - 1, count);
}

int __stdcall read_bytes(void* h, unsigned char * bytes, int size)
{
	if (nullptr == h)
	{
		std::cerr << "Please open device first!" << std::endl;
		return -1;
	}

	unsigned char read_buf[65] = { 0 };

	DWORD dwReadBytes;
	if (!ReadFile(h, read_buf, 65, &dwReadBytes, NULL))
	{
		std::cerr << "Failed to read data! error: " << GetLastError() << std::endl;
		return -1;
	}

	memcpy(bytes, &read_buf[1], min(64, size));

	return min(64, size);
}
