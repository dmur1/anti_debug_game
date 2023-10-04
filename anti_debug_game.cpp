#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <Windows.h>

#define MEMORY_SIZE (128*1024*1024)
uintptr_t memory;

unsigned int* player_health;
unsigned int* ammo_count;
float* position;

void init_game() {
    srand(time(0));

    memory = (uintptr_t)VirtualAlloc(0, MEMORY_SIZE, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

    player_health = (unsigned int*)((memory + (rand() % (MEMORY_SIZE - sizeof(unsigned int)))) & ~0xf);
    *player_health = 100;

    player_health = (unsigned int*)EncodePointer((PVOID)player_health);

    ammo_count = (unsigned int*)((memory + (rand() % (MEMORY_SIZE - sizeof(unsigned int)))) & ~0xf);
    *ammo_count = 42;

    ammo_count = (unsigned int*)EncodePointer((PVOID)ammo_count);

    position = (float*)((memory + (rand() % (MEMORY_SIZE - sizeof(float[3])))) & ~0xf);
    position[0] = 345.2f;
    position[1] = 78.7f;
    position[2] = 90.4f;

    position = (float*)EncodePointer((PVOID)position);
}

void print_player_health() {
    printf("Health: %u\n", *(unsigned int*)DecodePointer((PVOID)player_health));
}

void print_player_ammo_count() {
    printf("Ammo Count: %u\n", *(unsigned int*)DecodePointer((PVOID)ammo_count));
}

void print_player_position() {
    float* pos = (float*)DecodePointer((PVOID)position);
    printf("Position: x:%.2f y:%.2f z:%.2f\n", pos[0], pos[1], pos[2]);
}

int main(int argc, char** argv) {
    init_game();

    puts("Welcome to the anti-debug game!");

    for ( ; ; ) {
        printf("[0] Get player health (%p)\n", player_health);
        printf("[1] Get player ammo count (%p)\n", ammo_count);
        printf("[2] Get player position (%p)\n", position);

        int choice;
        scanf("%d", &choice);

        switch(choice) {
            case 0:
                print_player_health();
                break;
            case 1:
                print_player_ammo_count();
                break;
            case 2:
                print_player_position();
                break;
            default:
                return 0;
        }
    }

    return 0;
}

