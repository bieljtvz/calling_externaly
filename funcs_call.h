namespace funcs
{
	DWORD WINAPI stub_call();

	//
	//Threads
	int WINAPI thread_messagebox(PMSGBOX_DATA* cData);
	HANDLE WINAPI thread_ldrloaddll(PTHREAD_DATA data);	

	//
	//Calls
	bool call_messagebox(DWORD PID);
	bool call_ldrloaddll(DWORD PID, const char* dll_path);
};

DWORD WINAPI funcs::stub_call()
{
	return 0;
}

int WINAPI funcs::thread_messagebox(PMSGBOX_DATA* data)
{
	data->fnMessageBoxA(data->unk0, data->chMessage, data->chTitle, data->unk1);
	return 1;
}

HANDLE WINAPI funcs::thread_ldrloaddll(PTHREAD_DATA data)
{
	data->fnRtlInitUnicodeString(&data->UnicodeString, data->DllName);
	data->fnLdrLoadDll(data->DllPath, data->Flags, &data->UnicodeString, &data->ModuleHandle);
	return data->ModuleHandle;
}

bool funcs::call_messagebox(DWORD PID)
{
	LPVOID MessageBox_Addr = (LPVOID)GetProcAddress(GetModuleHandleA("user32.dll"), "MessageBoxA");
	if (!MessageBox_Addr)
	{
		printf("[-] Carregando user32.dll: \n");
		LoadLibrary(L"user32.dll");
		Sleep(1000);
		MessageBox_Addr = (LPVOID)GetProcAddress(GetModuleHandleA("user32.dll"), "MessageBoxA");
		if (!MessageBox_Addr)
		{
			printf("[-] Impossivel achar MessageBoxA: \n");
			return 1;
		}
		
	}
	

	PMSGBOX_DATA data;
	
	data.fnMessageBoxA = (pfnMessageBoxA)MessageBox_Addr;
	data.unk0 = NULL;
	data.unk1 = NULL;
	strcpy(data.chMessage,"Ola, fui chamado por remote process");
	strcpy(data.chTitle,"data");	
		

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);	
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		printf("[-] Falha ao abrir processo: \n");
		CloseHandle(hProcess);		
		return 0;
	}

	LPVOID pThreadData = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!pThreadData)
		return 0;

	WriteProcessMemory(hProcess, pThreadData, &data, sizeof(data), NULL);	

	DWORD SizeOfCode = (DWORD)funcs::stub_call - (DWORD)funcs::thread_messagebox;
	if (!SizeOfCode)
		return 0;

	LPVOID pCode = VirtualAllocEx(hProcess, NULL, SizeOfCode, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pCode)
		return 0;

	WriteProcessMemory(hProcess, pCode, (PVOID)funcs::thread_messagebox, SizeOfCode, NULL);

	auto status = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pCode, pThreadData, 0, 0);
	if (status == INVALID_HANDLE_VALUE)
	{
		printf("[-] Falha no CreateRemoteThread: verifique a handle\n");
		CloseHandle(status);
		return 0;
	}	
	

	CloseHandle(status);
	CloseHandle(hProcess);

	printf("[+] MessageBox injetado com exito! \n");
}

bool funcs::call_ldrloaddll(DWORD PID, const char* dll_path)
{
	LPVOID LdrLoadDll_Addr = (LPVOID)GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrLoadDll");
	LPVOID RtlInitUnicodeString_Addr = (LPVOID)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlInitUnicodeString");

	HANDLE hProcess = OpenProcess(GENERIC_ALL, NULL, PID);
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		printf("[-] Falha ao abrir processo: %X\n", GetLastError());
		CloseHandle(hProcess);
		return 0;
	}

	///////Init////////	

	size_t len = strlen(dll_path) + 1;
	size_t converted = 0;
	wchar_t* copy_wc_path;
	copy_wc_path = (wchar_t*)malloc(len * sizeof(wchar_t));
	mbstowcs_s(&converted, copy_wc_path, len, dll_path, _TRUNCATE);

	THREAD_DATA data;
	data.fnRtlInitUnicodeString = (pfnRtlInitUnicodeString)RtlInitUnicodeString_Addr;
	data.fnLdrLoadDll = (pfnLdrLoadDll)LdrLoadDll_Addr;
	memcpy(data.DllName, copy_wc_path, (wcslen(copy_wc_path) + 1) * sizeof(WCHAR));
	data.DllPath = NULL;
	data.Flags = 0;
	data.ModuleHandle = INVALID_HANDLE_VALUE;

	LPVOID pThreadData = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(hProcess, pThreadData, &data, sizeof(data), NULL);
	DWORD SizeOfCode = (DWORD)funcs::stub_call - (DWORD)funcs::thread_ldrloaddll;
	LPVOID pCode = VirtualAllocEx(hProcess, NULL, SizeOfCode, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(hProcess, pCode, (PVOID)funcs::thread_ldrloaddll, SizeOfCode, NULL);

	////////Thread////////////	

	auto status = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pCode, pThreadData, 0, 0);
	if (status == INVALID_HANDLE_VALUE)
	{
		CloseHandle(status);
		CloseHandle(hProcess);
		return 0;
	}

	CloseHandle(status);
	CloseHandle(hProcess);

	printf("[+] Dll injetada com sucesso: %X \n", status);
	return 1;
}