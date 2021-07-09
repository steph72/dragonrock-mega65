#include "memory.h"
#include <stdio.h>

#define DMALIST (*(struct dmagic_dmalist*)0x500)

// struct dmagic_dmalist dmalist;

unsigned char dma_byte;

void do_dma(void) {
    //  unsigned char i;
    mega65_io_enable();

    //  for(i=0;i<24;i++)
    //    printf("%02x ",PEEK(i+(unsigned int)&dmalist));
    //  printf("\n");
    //  while(1) continue;

    // Now run DMA job (to and from anywhere, and list is in low 1MB)
    POKE(0xd702U, 0);
    POKE(0xd704U, 0x00); // List is in $00xxxxx
    POKE(0xd701U, ((unsigned int)&DMALIST) >> 8);
    POKE(0xd705U, ((unsigned int)&DMALIST) & 0xff); // triggers enhanced DMA
}


unsigned char lpeek(long address) {
    // Read the byte at <address> in 28-bit address space
    // XXX - Optimise out repeated setup etc
    // (separate DMA lists for peek, poke and copy should
    // save space, since most fields can stay initialised).

    DMALIST.option_0b= 0x0b;
    DMALIST.option_80= 0x80;
    DMALIST.source_mb= (address >> 20);
    DMALIST.option_81= 0x81;
    DMALIST.dest_mb= 0x00; // dma_byte lives in 1st MB
    DMALIST.option_85= 0x85;
    DMALIST.dest_skip= 1;
    DMALIST.end_of_options= 0x00;
    DMALIST.sub_cmd= 0x00;

    DMALIST.command= 0x00; // copy
    DMALIST.count= 1;
    DMALIST.source_addr= address & 0xffff;
    DMALIST.source_bank= (address >> 16) & 0x0f;
    DMALIST.dest_addr= (unsigned int)&dma_byte;
    DMALIST.dest_bank= 0;

    do_dma();

    return dma_byte;
}

unsigned char db1, db2, db3;

unsigned char lpeek_debounced(long address) {
    db1= 0;
    db2= 1;
    while (db1 != db2 || db1 != db3) {
        db1= lpeek(address);
        db2= lpeek(address);
        db3= lpeek(address);
    }
    return db1;
}

void lpoke(long address, unsigned char value) {

    DMALIST.option_0b= 0x0b;
    DMALIST.option_80= 0x80;
    DMALIST.source_mb= 0x00; // dma_byte lives in 1st MB
    DMALIST.option_81= 0x81;
    DMALIST.dest_mb= (address >> 20);
    DMALIST.option_85= 0x85;
    DMALIST.dest_skip= 1;
    DMALIST.end_of_options= 0x00;
    DMALIST.sub_cmd= 0x00;

    dma_byte= value;
    DMALIST.command= 0x00; // copy
    DMALIST.count= 1;
    DMALIST.source_addr= (unsigned int)&dma_byte;
    DMALIST.source_bank= 0;
    DMALIST.dest_addr= address & 0xffff;
    DMALIST.dest_bank= (address >> 16) & 0x0f;

    do_dma();
    return;
}

void lcopy(long source_address, long destination_address, unsigned int count) {
    DMALIST.option_0b= 0x0b;
    DMALIST.option_80= 0x80;
    DMALIST.source_mb= source_address >> 20;
    DMALIST.option_81= 0x81;
    DMALIST.dest_mb= (destination_address >> 20);
    DMALIST.option_85= 0x85;
    DMALIST.dest_skip= 1;
    DMALIST.end_of_options= 0x00;
    DMALIST.sub_cmd= 0x00;

    DMALIST.command= 0x00; // copy
    DMALIST.count= count;
    DMALIST.sub_cmd= 0;
    DMALIST.source_addr= source_address & 0xffff;
    DMALIST.source_bank= (source_address >> 16) & 0x0f;
    if (source_address >= 0xd000 && source_address < 0xe000)
        DMALIST.source_bank|= 0x80;
    DMALIST.dest_addr= destination_address & 0xffff;
    DMALIST.dest_bank= (destination_address >> 16) & 0x0f;
    if (destination_address >= 0xd000 && destination_address < 0xe000)
        DMALIST.dest_bank|= 0x80;

    do_dma();
    return;
}

void lfill(long destination_address, unsigned char value, unsigned int count) {
    DMALIST.option_0b= 0x0b;
    DMALIST.option_80= 0x80;
    DMALIST.source_mb= 0x00;
    DMALIST.option_81= 0x81;
    DMALIST.dest_mb= destination_address >> 20;
    DMALIST.option_85= 0x85;
    DMALIST.dest_skip= 1;
    DMALIST.end_of_options= 0x00;

    DMALIST.command= 0x03; // fill
    DMALIST.sub_cmd= 0;
    DMALIST.count= count;
    DMALIST.source_addr= value;
    DMALIST.dest_addr= destination_address & 0xffff;
    DMALIST.dest_bank= (destination_address >> 16) & 0x0f;
    if (destination_address >= 0xd000 && destination_address < 0xe000)
        DMALIST.dest_bank|= 0x80;

    do_dma();
    return;
}

void lfill_skip(long destination_address, unsigned char value,
                unsigned int count, unsigned char skip) {

    DMALIST.option_0b= 0x0b;
    DMALIST.option_80= 0x80;
    DMALIST.source_mb= 0x00;
    DMALIST.option_81= 0x81;
    DMALIST.dest_mb= destination_address >> 20;
    DMALIST.option_85= 0x85;
    DMALIST.dest_skip= skip;
    DMALIST.end_of_options= 0x00;

    DMALIST.command= 0x03; // fill
    DMALIST.sub_cmd= 0;
    DMALIST.count= count;
    DMALIST.source_addr= value;
    DMALIST.dest_addr= destination_address & 0xffff;
    DMALIST.dest_bank= (destination_address >> 16) & 0x0f;
    if (destination_address >= 0xd000 && destination_address < 0xe000)
        DMALIST.dest_bank|= 0x80;

    do_dma();
    return;
}

void mega65_io_enable(void) {
    // Gate C65 IO enable
    POKE(0xd02fU, 0x47);
    POKE(0xd02fU, 0x53);
    // Force to full speed
    POKE(0, 65);
}
