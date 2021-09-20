#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include "variables.hpp"

bool variables::inf_hp, variables::inf_mana, variables::temporary_strenght;

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

void update_menu()
{
	system("cls");

	for (int i = 0; i < 31; i++) std::cout << "-";

	std::cout << "\nF1 - inf. hp\nF2 - inf. mana\nF3 - temporary strenght\n";

	for (int i = 0; i < 31; i++) std::cout << "-";

	if (variables::inf_hp)
	{
		std::cout << "\ninf. hp - active";
	}

	if (variables::inf_mana)
	{
		std::cout << "\ninf. mana - active";
	}

	if (variables::temporary_strenght)
	{
		std::cout << "\ntemporary strenght - active";
	}

	std::cout << "\n"; // yes.

	if (variables::inf_hp || variables::inf_mana || variables::temporary_strenght) for (int i = 0; i < 31; i++) std::cout << "-";
}

void main()
{

	update_menu();

	while (true)
	{
		if (GetAsyncKeyState(VK_F1) & 1)
		{
			variables::inf_hp = !variables::inf_hp;
			update_menu();
		}

		if (GetAsyncKeyState(VK_F2) & 1)
		{
			variables::inf_mana = !variables::inf_mana;
			update_menu();
		}

		if (GetAsyncKeyState(VK_F3) & 1)
		{
			variables::temporary_strenght = !variables::temporary_strenght;
			update_menu();
		}

		HWND hwnd = FindWindowA(NULL, "Gothic II - 2.6 (pol)");

		int hp_value = 1, mana_value= 1;
		int max_hp_value = 0, max_mana_value = 0;
		int new_temporary_strenght_value = 0;
		static int static_old_strenght_value = 0, old_strenght_value = 0;

		if (hwnd)
		{
			DWORD process_id;
			GetWindowThreadProcessId(hwnd, &process_id);
			HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
			uintptr_t module_base = module_base_address(process_id, L"Gothic2.exe");
			uintptr_t pointer_base_address = module_base + 0x42990C;

			uintptr_t hp_address = multilevel_pointer(handle, pointer_base_address, { 0x1B8 });
			uintptr_t max_hp_address = multilevel_pointer(handle, pointer_base_address, { 0x1BC });

			uintptr_t mana_address = multilevel_pointer(handle, pointer_base_address, { 0x1C0 });
			uintptr_t max_mana_address = multilevel_pointer(handle, pointer_base_address, { 0x1C4 });

			uintptr_t strenght_address = multilevel_pointer(handle, pointer_base_address, { 0x1C8 });

			if (process_id)
			{
				if (variables::inf_hp)
				{
					ReadProcessMemory(handle, (BYTE*)max_hp_address, &max_hp_value, sizeof(max_hp_value), 0);

					hp_value = max_hp_value;

					WriteProcessMemory(handle, (BYTE*)hp_address, &hp_value, sizeof(hp_value), 0);
				}

				if (variables::inf_mana)
				{
					ReadProcessMemory(handle, (BYTE*)max_mana_address, &max_mana_value, sizeof(max_mana_value), 0);

					mana_value = max_mana_value;

					WriteProcessMemory(handle, (BYTE*)max_mana_address, &mana_value, sizeof(mana_value), 0);
				}

				static bool at_least_once_enabled_strenght_hack;

				if (variables::temporary_strenght)
				{
					if (!at_least_once_enabled_strenght_hack)
					{
						ReadProcessMemory(handle, (BYTE*)strenght_address, &old_strenght_value, sizeof(old_strenght_value), 0);

						static_old_strenght_value = old_strenght_value;

						at_least_once_enabled_strenght_hack = true;
					}
					else
					{
						new_temporary_strenght_value = 190;

						WriteProcessMemory(handle, (BYTE*)strenght_address, &new_temporary_strenght_value, sizeof(new_temporary_strenght_value), 0);
					}
				}
				else
				{
					if (at_least_once_enabled_strenght_hack)
					{
						WriteProcessMemory(handle, (BYTE*)strenght_address, &static_old_strenght_value, sizeof(static_old_strenght_value), 0);
					}
				}
			}
		}
		else
		{
			system("cls");
			std::cout << "can't find game\nrestart external";
		}
		Sleep(1);
	}
}
