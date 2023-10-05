#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <tlhelp32.h>

#define TARGET_PROCESS_NAME "anti_debug_game.exe"
#define PAGE_SIZE (4096)

#define DEFAULT_VALUE_FOR_PLAYER_HEALTH (100)
#define DEFAULT_VALUE_FOR_AMMO_COUNT (42)

HANDLE process_handle;
LPVOID memory;
LPVOID player_health;
LPVOID ammo_count;
LPVOID position;

DWORD get_process_id() {
    DWORD process_id = -1;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "unable to create snapshot\n");
        CloseHandle(snapshot);
        return process_id;
    }

    PROCESSENTRY32 process_entry;
    process_entry.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(snapshot, &process_entry)) {
        fprintf(stderr, "unable to get first process from snapshot\n");
        CloseHandle(snapshot);
        return process_id;
    }

    do {
        if (strcmp(process_entry.szExeFile, TARGET_PROCESS_NAME) == 0 ) {
            process_id = process_entry.th32ProcessID;
            break;
        }
    } while (Process32Next(snapshot, &process_entry));

    CloseHandle(snapshot);

    return process_id;
}

int try_find_variables(LPVOID base_address, SIZE_T region_size) {
    for (SIZE_T i = 0; i < region_size; ++i) {
        unsigned char byte;
        SIZE_T number_of_bytes_read = 0;
        BOOL success = ReadProcessMemory(process_handle,
            (LPVOID)((uintptr_t)base_address + i), &byte, 1, &number_of_bytes_read);

        if (success == FALSE) {
            DWORD err = GetLastError();
            if (err != ERROR_PARTIAL_COPY) {
                fprintf(stderr, "failed to ReadProcessMemory %d\n", err);
                return 0;
            }
        }

        if (byte) {
            printf("%d\n", byte);
            if (byte == DEFAULT_VALUE_FOR_PLAYER_HEALTH) {
                player_health = (LPVOID)((uintptr_t)base_address + i);
            }
            else if (byte == DEFAULT_VALUE_FOR_AMMO_COUNT) {
                ammo_count = (LPVOID)((uintptr_t)base_address + i);
            }
            else if (!position) {
                position = (LPVOID)((uintptr_t)base_address + i);
            }
        }

        if (player_health && ammo_count && position) {
            break;
        }
    }

    return 1;
}

int enumerate_process_memory() {
    LPVOID offset = 0;
    MEMORY_BASIC_INFORMATION basic_info;
    memset(&basic_info, 0, sizeof(basic_info));

    while (VirtualQueryEx(process_handle, offset, &basic_info, sizeof(basic_info))) {
        if (basic_info.State == MEM_COMMIT && basic_info.Type == MEM_PRIVATE) {
            if (basic_info.RegionSize == 0x8000000) {
                printf("searching memory range %p...\n", (void*)basic_info.AllocationBase);
                if (!try_find_variables(basic_info.AllocationBase, basic_info.RegionSize)) {
                    fprintf(stderr, "failed to find variables\n");
                    return 0;
                }
                break;
            }
        }
        offset = (LPVOID)((uintptr_t)basic_info.BaseAddress + basic_info.RegionSize);
    }

    return 1;
}

DWORD get_player_health() {
    DWORD health;
    ReadProcessMemory(process_handle, player_health, &health, sizeof(health), 0);
    return health;
}

void set_player_health(DWORD new_health) {
    WriteProcessMemory(process_handle, player_health, &new_health, sizeof(new_health), 0);
}

DWORD get_ammo_count() {
    DWORD ammo;
    ReadProcessMemory(process_handle, ammo_count, &ammo, sizeof(ammo), 0);
    return ammo;
}

void set_ammo_count(DWORD new_ammo_count) {
    WriteProcessMemory(process_handle, ammo_count, &new_ammo_count, sizeof(new_ammo_count), 0);
}

void get_player_position(float* current_position) {
    for (int i = 0; i < 12; i++) {
        ReadProcessMemory(process_handle,
            (LPVOID)((uintptr_t)position + i), (unsigned char*)current_position + i, 1, 0);
    } 
}

void set_player_position(float* new_position) {
    for (int i = 0; i < 12; i++) {
        WriteProcessMemory(process_handle,
            (LPVOID)((uintptr_t)position + i), (unsigned char*)new_position + i, 1, 0);
    } 
}

int main(int argc, char** argv) {
    DWORD process_id = get_process_id();
    if (process_id == -1) {
        fprintf(stderr, "unable to get process id (is target running?)\n");
        return -1;
    }

    printf(TARGET_PROCESS_NAME " pid: %u\n", process_id);

    process_handle = OpenProcess(MAXIMUM_ALLOWED, FALSE, process_id);

    if (!enumerate_process_memory()) {
        fprintf(stderr, "failed to enumerate_process_memory\n");
        return -1;
    }

    printf("found player_health: %p\n", player_health);
    printf("found ammo_count: %p\n", ammo_count);
    printf("found player position: %p\n", position);

    DWORD previous_player_health = get_player_health();
    set_player_health(420);
    printf("set player health from %u to %u\n", previous_player_health, get_player_health());

    DWORD previous_ammo_count = get_ammo_count();
    set_ammo_count(0xffffffff);
    printf("set ammo count from %u to %u\n", previous_player_health, get_ammo_count());

    float previous_position[3];
    get_player_position(previous_position);

    float new_position[3] = {2.0f, 3.0f, 4.0f};
    set_player_position(new_position);

    float current_position[3];
    get_player_position(current_position);
    printf("set position from %.2f,%.2f,%.2f to %.2f,%.2f,%.2f\n", 
        previous_position[0], previous_position[1], previous_position[2],
        current_position[0], current_position[1], current_position[2]);

    return 0;
}

