#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/vfs.h>
#include <sys/vdevice.h>
#include <sys/charbuf.h>
#include <sys/mmio.h>
#include <sys/proc.h>
#include <sys/ipc.h>
#include <sys/interrupt.h>
#include <sys/interrupt.h>

#include "ms_serial.h"


#define BASE (_mmio_base + 0x221000)
#define UART_MULTI_REG8(_x_)  ((uint8_t volatile *)(BASE))[((_x_) * 4) - ((_x_) & 1)]


static int uart_read(int fd, int from_pid, fsinfo_t* info, 
		void* buf, int size, int offset, void* p) {
	(void)fd;
	(void)from_pid;
	(void)offset;
	(void)info;
	(void)size;
	(void)p;

	char c;

    if(!(UART_MULTI_REG8(UART_LSR) & UART_LSR_DR))
		return ERR_RETRY_NON_BLOCK;

    ((uint8_t*)buf)[0]=(char) ( UART_MULTI_REG8(UART_TX) & 0xff);

    return 1;
}

static int uart_write(int fd, int from_pid, fsinfo_t* info,
		const void* buf, int size, int offset, void* p) {
	(void)fd;
	(void)info;
	(void)from_pid;
	(void)offset;
	(void)p;
	char c;

	for(int i = 0; i < size; i++){
		c = ((char*)buf)[i];
		if(c == '\r') c = '\n';
    	    while (!(UART_MULTI_REG8(UART_LSR) & UART_LSR_THRE));
    	    UART_MULTI_REG8(UART_TX) = c;
	}
	return size;
}

static void interrupt_handle(uint32_t interrupt, uint32_t data) {
	(void)interrupt;
	(void)data;
	char c;


	sys_interrupt_end();
}

int main(int argc, char** argv) {
	const char* mnt_point = argc > 1 ? argv[1]: "/dev/tty1";
	_mmio_base = mmio_map();
	vdevice_t dev;
	memset(&dev, 0, sizeof(vdevice_t));
	strcpy(dev.name, "ms_uart");
	dev.read = uart_read;
	dev.write = uart_write;

	//sys_interrupt_setup(SYS_INT_TIMER0, interrupt_handle, 0);
	device_run(&dev, mnt_point, FS_TYPE_CHAR);
	return 0;
}
