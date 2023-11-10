#include <string.h>
#include <WS2tcpip.h>
#include <TlHelp32.h>
#include <psapi.h>

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>

using namespace std;

size_t offset = 0;

void test()
{
    int pid = 1176;
    
    HANDLE process_info_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid);
    LPVOID base_address = NULL;

    string process_name_path(10000, '\0');
    process_name_path.resize(GetProcessImageFileNameA(process_info_handle, &process_name_path[0], 10000));
    process_name_path.resize(strlen(process_name_path.data()));
    // cout << process_name_path << endl;

    vector<HMODULE> module_list(10000);
    DWORD size = 0;

    if (EnumProcessModules(process_info_handle, module_list.data(), 10000, &size) == 0)
    {
        return;
    }
    module_list.resize( size / sizeof(HMODULE));
    for (auto handle_module: module_list)
    {
        string module_name(10000, '\0');
        if ( GetModuleFileNameExA( process_info_handle, handle_module, module_name.data(), 10000))
        {
            module_name.resize(strlen(&module_name[0]));
            int end_of_device = module_name.find("\\", 0);
            
            std::string dos_device_name(10000,'\0');
            QueryDosDeviceA(module_name.substr(0, end_of_device).data(), &dos_device_name[0], 10000);
            dos_device_name.resize(strlen(&dos_device_name[0]));

            std::string image_file_name = dos_device_name + module_name.substr(end_of_device, module_name.size());

            if (image_file_name == process_name_path)
            {
                MODULEINFO module_info;
                GetModuleInformation(process_info_handle, handle_module, &module_info, sizeof(MODULEINFO));
                base_address = module_info.lpBaseOfDll;
                break;
            }
        }
    }

    cout << "Target base address: 0x" << hex << base_address << endl;

    size_t addr = (unsigned long long)base_address + offset;
    size_t n_bytes;
    std::vector<UCHAR> c;
    DWORD image_size = 0x00024000;
    c.resize(image_size);
    MEMORY_BASIC_INFORMATION mem_info;
    
    if (ReadProcessMemory(process_info_handle, (LPVOID)(addr), c.data(), image_size, &n_bytes) == 0)
    {
        cout << "Read false" << endl;
        return;
    }
    else
    {
        cout << "Read: 0x" << hex << n_bytes << " bytes." << endl;
    }
    cout << hex << (ULONG)(c[0]) << endl;
    cout << hex << (ULONG)(c[1]) << endl;

    CloseHandle(process_info_handle);
}

int main()
{
    test();
    return 0;
}