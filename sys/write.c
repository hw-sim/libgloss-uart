#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include "syscall.h"
#include "atomic.h"
#include "spinlock.h"

#define UART_BASE            0x10000000UL   // 示例地址，占位：请按你的 SoC 修改！
#define UART_INPUT_CLK_HZ    50000000UL     // 举例 50 MHz

#define UART_TXDATA  (*(volatile uint32_t *)(UART_BASE + 0x00))
#define UART_RXDATA  (*(volatile uint32_t *)(UART_BASE + 0x04))
#define UART_TXCTRL  (*(volatile uint32_t *)(UART_BASE + 0x08))
#define UART_RXCTRL  (*(volatile uint32_t *)(UART_BASE + 0x0C))
#define UART_IE      (*(volatile uint32_t *)(UART_BASE + 0x10))
#define UART_IP      (*(volatile uint32_t *)(UART_BASE + 0x14))
#define UART_DIV     (*(volatile uint32_t *)(UART_BASE + 0x18))


void uart_init() {
    UART_IE = 0;
    UART_DIV = 1;
    UART_TXCTRL = 1u;  // txen=1
    UART_RXCTRL = 1u;  // rxen=1
}

void uart_putc(char c) {
    while (UART_TXDATA & (1u << 31)) { /* spin */ }
    UART_TXDATA = (uint8_t)c;
}

static spinlock_t write_lock = SPINLOCK_INIT;

int uart_initialized = 0;

ssize_t _write(int fd, const void *buf, size_t cnt) {
    // 仅处理 stdout/stderr
    if (fd != 1 && fd != 2) return -1;
    spin_lock(&write_lock);
    wmb();
    if(!uart_initialized) {
        uart_init();
        uart_initialized = 1;
    }
    const char *p = (const char*)buf;
    for (size_t i = 0; i < cnt; ++i) {
        if (p[i] == '\n') uart_putc('\r'); // 终端常见的 CRLF
        uart_putc(p[i]);
    }
    spin_unlock(&write_lock);
    rmb();
    return (int)cnt;
}
