// libhid.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <string>
extern "C"
{
#include "Setupapi.h"
#include "Hidsdi.h"
};

std::string GetHidDevice(unsigned short uVID, unsigned short  uPID);

std::string GetHidDevice(unsigned short uVID, unsigned short  uPID)
{
	DWORD Requred = 0;
	DWORD MemberIndex = 0;
	DWORD NeedLength = 0;
	LPGUID HidGuid;

	HidD_GetHidGuid(&HidGuid);
	hDevInfo = SetupDiGetClassDevs(&HidGuid, NULL, NULL,
		DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		//MessageBox(L"Get Class Devices Error!");
		return FALSE;
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
			return FALSE;
		}

		MemberIndex++;

		Result = SetupDiGetDeviceInterfaceDetail
		(hDevInfo, &devInfoData, NULL, 0, &Requred, NULL);
		if (!Result)
		{
			if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
			{
				SetupDiDestroyDeviceInfoList(hDevInfo);
				return FALSE;
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
			DevicePath = CString(detailData->DevicePath);
			free(detailData);
			return TRUE;
		}
	}
}