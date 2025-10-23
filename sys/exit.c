#include "encoding.h"
#include "syscall.h"
#include <stdio.h>
#include <atomic.h>
#include <string.h>
#include <stdint.h>

void __attribute__ ((noreturn)) _exit(int code)
{
    char buf[128];
    unsigned long hartid;
    unsigned long cycles;
    __asm__ volatile ("csrr %0, mhartid\n" : "=r"(hartid));
    __asm__ volatile ("csrr %0, mcycle\n" : "=r"(cycles));
    int color = code ? 31 : 32;
    sprintf(buf, "\e[0;%dmEXIT from hart %ld with code %d at cycle %ld\n\e[0m", color, hartid, code, cycles);
    _write(1, buf, strlen(buf));
    while(1);
}
