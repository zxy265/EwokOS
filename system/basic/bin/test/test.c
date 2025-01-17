#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/interrupt.h>
#include <sys/timer.h>

static uint32_t _v;

static void timer_handle(void) {
	_v++;
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	_v = 0;
	
	uint32_t id = timer_set(1000000, timer_handle);

	while(1) {
		printf("pid: %d, v: %d\n", getpid(), _v);
		sleep(1);

		/*if(_v > 10)  {
			timer_remove(id);
		}
		*/
	}
	return 0;
}
