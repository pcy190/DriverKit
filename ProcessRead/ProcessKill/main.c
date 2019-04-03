#include <ntddk.h>
#include <windef.h>
NTSTATUS PsLookupProcessByProcessId(
	HANDLE    ProcessId,
	PEPROCESS *Process
);
PVOID __stdcall PsGetProcessSectionBaseAddress(PEPROCESS Process);



typedef struct _KAPC_STATE {
	LIST_ENTRY ApcListHead[MaximumMode];       //线程的apc链表 只有两个 内核态和用户态
	struct _KPROCESS *Process;               //当前线程的进程体   PsGetCurrentProcess()
	BOOLEAN KernelApcInProgress;              //内核APC正在执行
	BOOLEAN KernelApcPending;                 //内核APC正在等待执行
	BOOLEAN UserApcPending;                  //用户APC正在等待执行
} KAPC_STATE, *PKAPC_STATE, *PRKAPC_STATE;

void KeStackAttachProcess(
	PRKPROCESS   PROCESS,
	PRKAPC_STATE ApcState
);
void KeUnstackDetachProcess(
	PRKAPC_STATE ApcState
);

BYTE readcode[4];	//cpory to readcode,you can change readcode size to change size to read.
ULONG offset = 0x10;//offset to Program Base Address

VOID MyProcessNotify(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create) {
	if (Create&&ProcessId!=4&&ProcessId!=0)
	{
		PEPROCESS tempEp = NULL;
		NTSTATUS status = STATUS_SUCCESS;
		PUCHAR processBaseAddr = NULL;
		KAPC_STATE temp_stack = { 0 };
		status = PsLookupProcessByProcessId(ProcessId, &tempEp);
		if (tempEp)
		{

			ObDereferenceObject(tempEp);
			processBaseAddr = (PUCHAR)PsGetProcessSectionBaseAddress(tempEp);
			if (!processBaseAddr) {
				DbgPrint("Get ProcessBaseAddr Fail!");
				return;
			}
			RtlZeroMemory(readcode, sizeof(readcode));
			KeStackAttachProcess(tempEp, &temp_stack);
			__try {

				ProbeForRead((processBaseAddr + offset), sizeof(readcode), 1);

				RtlCopyMemory((PVOID)readcode, (PVOID)(processBaseAddr + offset), sizeof(readcode));

			}
			__except (1) {
				DbgPrint("Memory is error while reading!\n");
			}
			KeUnstackDetachProcess(&temp_stack);
			for (int i = 0; i < sizeof(readcode); i++)
			{
				DbgPrint("Read %x", readcode[i]);
			}


		}


	}

}

VOID Unload(PDRIVER_OBJECT driver) {
	PsSetCreateProcessNotifyRoutine(MyProcessNotify, TRUE);

}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING RegPath) {

	NTSTATUS status = STATUS_SUCCESS;

	status = PsSetCreateProcessNotifyRoutine(MyProcessNotify, FALSE);

	if (!NT_SUCCESS(status)) {
		DbgPrint("Create Process Notify Fail!");
	}


	driver->DriverUnload = Unload;
	return STATUS_SUCCESS;
}