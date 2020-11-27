#define _CRT_SECURE_NO_WARNINGS

#include <windows.h> 
#include <tlhelp32.h> 
#include <shlwapi.h> 
#include <conio.h> 
#include <stdio.h> 
#include "structs.h"
#include "utils.h"
#include "funcs_call.h"

int main()
{
	
	printf("[+] Iniciando: \n");

	utils::EnableDebugPrivilege(1);


inicio:

	DWORD PID = utils::get_pid_process("raidcall.exe");
	if (!PID)
	{
		printf("[-] Processo nao atacado: \n");
		Sleep(2000);
		goto inicio;		
		return 0;
	}

	//funcs::call_messagebox(PID);
	funcs::call_ldrloaddll(PID, "C:\\Users\\ELB\\source\\repos\\MessageBox_Dll\\Debug\\MessageBox_Dll.dll");
	
	getchar();
	return 0;
}