/bin/rundev /drivers/raspix/pl011_uartd   /dev/tty0
#/bin/rundev /drivers/raspix/mini_uartd    /dev/tty0

$

/bin/rundev /drivers/timerd               /dev/timer
/bin/rundev /drivers/nulld                /dev/null
/bin/rundev /drivers/ramfsd               /tmp
/bin/rundev /drivers/proc/sysinfod        /proc/sysinfo
/bin/rundev /drivers/proc/stated          /proc/state

@/bin/session
