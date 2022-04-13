#include "vmem.hpp"

#include "noitree.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>

int main() {
    constexpr size_t memSize = 0x1000;
    os::vmem::VirtualMemory mem{memSize * 3};
    printf("Virtual memory allocated: %zu bytes at %p\n", mem.Size(), mem.Ptr());

    // TODO: make a more realistic demo
    // - two os::vmem::VirtualMemory instances (read, write)
    // - os::vmem::BackedMemory instances for:
    //   - zero page (for open bus reads)
    //   - discard page (for writes to read-only areas)
    //   - RAM
    //   - ROM
    // - mappings:
    //   region     base     read   write
    //   ROM        0x0000   ROM    discard
    //   RAM        0x1000   RAM    RAM
    //   MMIO       0x2000   -      -
    //   open bus   0x3000   zero   discard
    // TODO: implement exception handling system for MMIO and other special cases
    // TODO: efficient mirroring
    // TODO: fast map/unmap on Windows (for NDS VRAM and TCM)

    auto u8mem = reinterpret_cast<uint8_t *>(mem.Ptr());

    os::vmem::BackedMemory ram{0x1000, os::vmem::Access::ReadWrite};
    printf("RAM allocated: %zu bytes\n", ram.Size());

    auto view1 = mem.Map(ram, 0x0000);
    if (view1.ptr) {
        printf("RAM mapped to 0x0000 -> %p\n", view1.ptr);
    }

    auto view2 = mem.Map(ram, 0x1000);
    if (view2.ptr) {
        printf("RAM mirror mapped to 0x1000 -> %p\n", view2.ptr);
    }

    volatile uint8_t *mmio = &u8mem[0x2000];
    printf("MMIO at 0x2000 -> %p\n", mmio);

    mem.AddUnmappedAccessHandlers(
        0x2000, 0x2FFF, nullptr,
        [](void *context, uintptr_t address, size_t size, void *value) {
            printf("Read size=%zu, addr=%zx\n", size, address);
            switch (size) {
            case 1: *reinterpret_cast<uint8_t *>(value) = 12u; break;
            case 2: *reinterpret_cast<uint16_t *>(value) = 1234u; break;
            case 4: *reinterpret_cast<uint32_t *>(value) = 12345678u; break;
            case 8: *reinterpret_cast<uint64_t *>(value) = 1234567890123456u; break;
            }
        },
        [](void *context, uintptr_t address, size_t size, const void *value) {
            printf("Write size=%zu, addr=%zx, value=", size, address);
            switch (size) {
            case 1: printf("%u\n", *reinterpret_cast<const uint8_t *>(value)); break;
            case 2: printf("%u\n", *reinterpret_cast<const uint16_t *>(value)); break;
            case 4: printf("%u\n", *reinterpret_cast<const uint32_t *>(value)); break;
            case 8: printf("%llu\n", *reinterpret_cast<const uint64_t *>(value)); break;
            }
        });
    printf("Added unmapped access handlers to MMIO region\n");

    /*uint8_t *u8buf = static_cast<uint8_t *>(mem.Ptr());
    uint8_t *u8view1 = static_cast<uint8_t *>(view1.ptr);
    uint8_t *u8view2 = static_cast<uint8_t *>(view2.ptr);

    auto printMem = [&] {
        printf("(direct - main)    %02X %02X %02X %02X\n", u8buf[0], u8buf[1], u8buf[2], u8buf[3]);
        printf("(direct - mirror)  %02X %02X %02X %02X\n", u8buf[memSize + 0], u8buf[memSize + 1], u8buf[memSize + 2],
               u8buf[memSize + 3]);
        printf("(view - main)      %02X %02X %02X %02X\n", u8view1[0], u8view1[1], u8view1[2], u8view1[3]);
        printf("(view - mirror)    %02X %02X %02X %02X\n", u8view2[0], u8view2[1], u8view2[2], u8view2[3]);
    };

    std::fill_n(u8buf, memSize, (uint8_t)0);
    u8buf[0] = 15;
    u8buf[1] = 33;
    u8buf[2] = 64;
    printf("Memory contents after direct manipulation to main region:\n");
    printMem();
    printf("\n");
    u8buf[memSize + 0] = 22;
    u8buf[memSize + 1] = 41;
    u8buf[memSize + 2] = 78;
    printf("Memory contents after direct manipulation to mirror region:\n");
    printMem();
    printf("\n");
    u8view1[1] = 73;
    u8view1[2] = 41;
    u8view1[3] = 1;
    printf("Memory contents after manipulation through main view:\n");
    printMem();
    printf("\n");
    u8view2[0] = 99;
    u8view2[1] = 88;
    u8view2[3] = 77;
    printf("Memory contents after manipulation through mirror view:\n");
    printMem();*/

    // Try accessing MMIO
    mmio[0] = 21;
    *reinterpret_cast<volatile uint16_t *>(&mmio[2]) = 4321;
    *reinterpret_cast<volatile uint32_t *>(&mmio[4]) = 87654321;
    *reinterpret_cast<volatile uint64_t *>(&mmio[6]) = 54321;
    *reinterpret_cast<volatile uint64_t *>(&mmio[8]) = 6543210987654321;
    printf("MMIO write done\n");
    uint8_t mmioVal8 = mmio[1];
    uint16_t mmioVal16 = *reinterpret_cast<volatile uint16_t *>(&mmio[3]);
    uint32_t mmioVal32 = *reinterpret_cast<volatile uint32_t *>(&mmio[5]);
    uint64_t mmioVal64 = *reinterpret_cast<volatile uint64_t *>(&mmio[7]);
    printf("MMIO value read: %u %u %u %llu\n", mmioVal8, mmioVal16, mmioVal32, mmioVal64);

    /*if (mem.Unmap(view1)) {
        printf("RAM unmapped from 0x0000\n");
    }
    if (mem.Unmap(view2)) {
        printf("RAM unmapped from 0x1000\n");
    }*/

    return EXIT_SUCCESS;
}
