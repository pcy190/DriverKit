#include <ntddk.h>
#include <windef.h>


typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT PathLength;
	CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;


typedef NTSTATUS(*NTQUERYSYSTEMINFORMATION) (
	IN ULONG SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG_PTR SystemInformationLength,
	OUT PULONG_PTR ReturnLength OPTIONAL
	);

VOID getcpuid();
BOOLEAN CheckDriverModule();

UNICODE_STRING symLinkName = { 0 };
PDEVICE_OBJECT pDevice;
PETHREAD pThreadObject = NULL;
BOOLEAN boom = FALSE;

//eax为CPU型号
//若为虚拟机，则ecx最高位为1;如果是物理机，则最高位为0.
//code in cpuid.asm
DWORD a = 0, b = 0, c_var = 0, d = 0;


VOID Unload() {//PDRIVER_OBJECT DriverObject
	if (pThreadObject) {
		boom = TRUE;
		KeWaitForSingleObject(pThreadObject, Executive, KernelMode, FALSE, NULL);
	}
	if (pDevice) {
		IoStopTimer(pDevice);
		IoDeleteSymbolicLink(&symLinkName);
		IoDeleteDevice(pDevice);
		pDevice = NULL;
	}
	DbgPrint("Unload --------");
}

VOID TimerRoutine(PDEVICE_OBJECT DeviceObject, PVOID context) {
	DbgPrint("In TimerRoutine\n");
	//Begin normal check
	getcpuid();
	c_var = c_var >> 31;
	if (c_var){
		DbgPrint("Running in the virtual machine 2. Find VMWare by CPUID!!!!\n");
	}
}

BOOLEAN CheckDriverModule() {
	BOOLEAN bRet = FALSE;
	NTQUERYSYSTEMINFORMATION m_NtQuerySystemInformation = NULL;
	UNICODE_STRING NtQuerySystemInformation_Name = { 0 };
	PSYSTEM_MODULE_INFORMATION ModuleEntry = NULL;
	ULONG_PTR RetLength = 0, BaseAddr = 0, EndAddr = 0;
	ULONG ModuleNums = 0, Index = 0;
	NTSTATUS status = STATUS_SUCCESS;
	PVOID Buffer = NULL;
	RtlInitUnicodeString(&NtQuerySystemInformation_Name, L"NtQuerySystemInformation");
	do
	{
		m_NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)MmGetSystemRoutineAddress(&NtQuerySystemInformation_Name);
		if (m_NtQuerySystemInformation == NULL) {
			bRet = TRUE;
			break;
		}
		status = m_NtQuerySystemInformation(0xb, NULL, 0, &RetLength);
		if (status < 0 && status != STATUS_INFO_LENGTH_MISMATCH) {
			bRet = TRUE;
			break;
		}
		Buffer = ExAllocatePoolWithTag(PagedPool, RetLength, "ytz");
		if (Buffer == NULL) {
			bRet = TRUE;
			break;
		}
		RtlZeroMemory(Buffer, RetLength);
		status = m_NtQuerySystemInformation(0xb, Buffer, RetLength, &RetLength);
		if (status < 0) {
			bRet = TRUE;
			break;
		}
		ModuleNums = *(ULONG*)Buffer;
		ModuleEntry = (PSYSTEM_MODULE_INFORMATION)((ULONG_PTR)Buffer + 8);
		for (Index = 0; Index < ModuleNums; Index++) {
			if (strstr(ModuleEntry->ImageName, "vmmouse.sys") ||
				strstr(ModuleEntry->ImageName, "vmrawdsk.sys") ||
				strstr(ModuleEntry->ImageName, "vmusbmouse.sys")) {

				//strstr(ModuleEntry->ImageName, "vmci.sys") 
				//strstr(ModuleEntry->ImageName, "vsock.sys") 

				DbgPrint("The Module Name is %s\n", ModuleEntry->ImageName);
				bRet = TRUE;
				break;
			}
			ModuleEntry++;
		}
		break; //Amuse
	} while (TRUE);
	if (Buffer)
	{
		ExFreePool(Buffer);
		Buffer = NULL;
	}
	return bRet;

}

VOID CheckVmWare(PVOID context) {
	LARGE_INTEGER sleeptime = { 0 };
	sleeptime.QuadPart = -20000000;
	while (1)
	{
		if (boom)
			break;
		if (CheckDriverModule()) {
			DbgPrint("Running in the virtual machine 1. Find VMWare!!!!\n");
		}
		DbgPrint("Thread is working");
		KeDelayExecutionThread(KernelMode, 0, &sleeptime);
	}
	PsTerminateSystemThread(STATUS_SUCCESS);

}

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING RegPath) {
	HANDLE hThread = NULL;
	NTSTATUS status = NULL;
	UNICODE_STRING DeviceName = { 0 };
	driverObject->DriverUnload = Unload;

	status = PsCreateSystemThread(&hThread, 0, NULL, NULL, NULL, CheckVmWare, NULL);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Create Thread Failed!\n");
		return STATUS_UNSUCCESSFUL;
	}

	status = ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, *PsThreadType, KernelMode, &pThreadObject, NULL);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Cannot reference");
		ObDereferenceObject(pThreadObject);
	}

	RtlInitUnicodeString(&DeviceName, L"\\Device\\MyDevices");
	status = IoCreateDevice(driverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevice);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Create Device Failed!");
		return STATUS_UNSUCCESSFUL;
	}
	RtlInitUnicodeString(&symLinkName, L"\\?\\SybLinkName");
	status = IoCreateSymbolicLink(&symLinkName, &DeviceName);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Create SymLink Failed!");
		//return STATUS_UNSUCCESSFUL;
	}
	IoInitializeTimer(pDevice, (PIO_TIMER_ROUTINE)TimerRoutine, NULL);
	IoStartTimer(pDevice);

	ZwClose(hThread);
	return STATUS_SUCCESS;
}
