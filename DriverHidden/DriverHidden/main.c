#include<ntifs.h>
#include<ntddk.h>


////x86
//typedef struct _LDR_DATA_TABLE_ENTRY
//{
//	LIST_ENTRY InLoadOrderLinks;
//	LIST_ENTRY InMemoryOrderLinks;
//	LIST_ENTRY InInitializationOrderLinks;
//	PVOID DllBase;
//	PVOID EntryPoint;
//	ULONG32 SizeOfImage;
//	UNICODE_STRING FullDllName;
//	UNICODE_STRING BaseDllName;
//	UINT32 Unknow[17];
//}LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;


//x64
typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG32 SizeOfImage;
	UINT8 Unknow0[0x4];
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
}LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

NTSTATUS ObReferenceObjectByName(
	__in PUNICODE_STRING ObjectName,
	__in ULONG Attributes,
	__in_opt PACCESS_STATE AccessState,
	__in_opt ACCESS_MASK DesiredAccess,
	__in POBJECT_TYPE ObjectType,
	__in KPROCESSOR_MODE AccessMode,
	__inout_opt PVOID ParseContext,
	__out PVOID *Object
);

extern POBJECT_TYPE *IoDriverObjectType;

PDRIVER_OBJECT g_PDriverObj;

VOID NewHide() {
	LARGE_INTEGER sleep = { 0 };
	sleep.QuadPart = -2000000;
	PDRIVER_OBJECT ghostofdeath = NULL;
	UNICODE_STRING ghostname = { 0 };
	RtlInitUnicodeString(&ghostname, L"\\Driver\\beep");
	ObReferenceObjectByName(&ghostname, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, &ghostofdeath);
	if (g_PDriverObj&&ghostofdeath) {
		ObDereferenceObject(ghostofdeath);
		PLDR_DATA_TABLE_ENTRY entry = (PLDR_DATA_TABLE_ENTRY)g_PDriverObj->DriverSection;
		PLDR_DATA_TABLE_ENTRY ghost_entry = (PLDR_DATA_TABLE_ENTRY)ghostofdeath->DriverSection;

		//巧妙的方法，保证SEH不崩溃
		entry->DllBase = ghost_entry->DllBase;

		//摘链核心
		PLIST_ENTRY pList = &(entry->InLoadOrderLinks);
		//PLIST_ENTRY ghost_pList = &(ghost_entry->InLoadOrderLinks);
		RemoveEntryList(pList);

		pList->Flink = NULL;
		pList->Blink = NULL;
		g_PDriverObj->DriverSection = NULL;
		g_PDriverObj->DriverSize = NULL;
		g_PDriverObj->DriverUnload = NULL;
		g_PDriverObj->DriverInit = NULL;
		g_PDriverObj->DeviceObject = NULL;
		
		sleep.QuadPart = -1000000;
		KeDelayExecutionThread(KernelMode, FALSE, &sleep);
		
		//类似销毁驱动
		ObMakeTemporaryObject(g_PDriverObj);

		//Now SEH test!!
		INT a = 1, b = 0,c=0;
		__try {
			c = a / b;
		}
		__except(TRUE) {
			DbgPrint("Catch SEH from Hidden Driver!");
		}
		
	}

	//You can do sth here as this Driver is hidden now.

	DbgPrint("End of NewHide.");
}

void unload()
{
	DbgPrint("NowUnload!");
}
NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING RegPath) {

	DbgPrint("Entry come!!");
	g_PDriverObj = driverObject;
	driverObject->DriverUnload = unload;
	//Now NewHide fun is a new DriverEntry
	IoRegisterDriverReinitialization(driverObject,NewHide,NULL);
}