/bin/rundev /drivers/raspix/fbd           /dev/fb0 640 480
/bin/rundev /drivers/raspix/pl011_uartd   /dev/tty0
#/bin/rundev /drivers/raspix/mini_uartd    /dev/tty0

/bin/rundev /drivers/displayd             /dev/display /dev/fb0
/bin/rundev /drivers/consoled             /dev/console0 /dev/display

$

/bin/rundev /drivers/nulld                /dev/null
/bin/rundev /drivers/ramfsd               /tmp
/bin/rundev /drivers/proc/sysinfod        /proc/sysinfo
/bin/rundev /drivers/proc/stated          /proc/state
/bin/rundev /drivers/timerd               /dev/timer

/bin/rundev /drivers/xserverd             /dev/x

#@/sbin/x/xmoused /dev/mouse0 &
@/sbin/x/xim_none &
@/sbin/x/xim_vkey &
@/bin/x/launcher &

@/bin/session