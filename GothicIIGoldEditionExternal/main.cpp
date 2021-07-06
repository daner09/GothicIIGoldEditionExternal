#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

uintptr_t module_base_address(DWORD process_id, const wchar_t* module_name)
{
	uintptr_t module_base_addres = 0;
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);

	if (snap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 mod_entry;
		mod_entry.dwSize = sizeof(mod_entry);
		if (Module32First(snap, &mod_entry))
		{
			do
			{
				if (!_wcsicmp(mod_entry.szModule, module_name))
				{
					module_base_addres = (uintptr_t)mod_entry.modBaseAddr;
					break;
				}
			} while (Module32Next(snap, &mod_entry));
		}
	}

	CloseHandle(snap);
	return module_base_addres;
}

uintptr_t multilevel_pointer(HANDLE process, uintptr_t pointer, std::vector<unsigned int> offsets)
{
	uintptr_t address = pointer;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		ReadProcessMemory(process, (BYTE*)address, &address, sizeof(address), 0);
		address += offsets[i];
	}
	return address;
}

void main()
{
	//0x1B8 - local
	//? + ?[4] + 0x1B8 - local pointers?
	/* upon damage
	
	EAX=00000000
EBX=FFFFFFFF
ECX=28187248
EDX=FFFFFFFB
ESI=0126EA10
EDI=FFFFFFFB
EBP=28187248
ESP=0126E638
EIP=006C506D

Probable base pointer =28187248

Label1
006C5064 - jne 006C50B8
006C5066 - add [ecx+eax*4+000001B8],edx
006C506D - jns 006C507A
006C506F - mov [ecx+eax*4+000001B8],00000000

	
	*/

	/* local loop
	
	EAX=0126F274
EBX=281A5960
ECX=28187248
EDX=0000000F
ESI=281A5960
EDI=0BC4FCA8
EBP=00000000
ESP=0126F198
EIP=006CA616

Probable base pointer =28187248

Label1
006CA60F - nop
006CA610 - mov edx,[ecx+000001B8]
006CA616 - xor eax,eax
006CA618 - test edx,edx

	
	*/

	/* here is module + pointer + hp | for ur own usage if u r going to paste my code, hf
	
	EAX=00000901
EBX=008297D0
ECX=2A1293D0
EDX=C1C2B500
ESI=0126F178
EDI=00829B4C
EBP=0079762C
ESP=0126F0DC
EIP=0049C34B

Probable base pointer =008297D0

Label1
0049C340 - mov eax,[007F52CC]
0049C345 - mov ecx,[ebx+0000013C]
0049C34B - push eax
0049C34C - push ecx

	
	*/

	HWND hwnd = FindWindowA(NULL, "Gothic II - 2.6 (pol)");
	int value = 1;
	int max_hp_value = 0;

	if (hwnd)
	{
		DWORD process_id;
		GetWindowThreadProcessId(hwnd, &process_id);
		HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
		uintptr_t module_base = module_base_address(process_id, L"Gothic2.exe");
		uintptr_t pointer_base_address = module_base + 0x42990C;
		uintptr_t hp_address = multilevel_pointer(handle, pointer_base_address, { 0x1B8 });
		uintptr_t max_hp_address = multilevel_pointer(handle, pointer_base_address, { 0x1BC });

		if (process_id)
		{
			for (;;)
			{
				ReadProcessMemory(handle, (BYTE*)max_hp_address, &max_hp_value, sizeof(max_hp_value), 0);

				value = max_hp_value;

				WriteProcessMemory(handle, (BYTE*)hp_address, &value, sizeof(value), 0);
			}
		}
	}
	std::cin.get();
}