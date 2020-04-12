#ifndef SYSCALLS_H
#define SYSCALLS_H

enum {
	SYS_NONE = 0,
	SYS_KPRINT,
	SYS_DEV_CHAR_READ,
	SYS_DEV_CHAR_WRITE,
	SYS_DEV_BLOCK_READ,
	SYS_DEV_BLOCK_WRITE,
	SYS_DEV_BLOCK_READ_DONE,
	SYS_DEV_BLOCK_WRITE_DONE,

	SYS_MALLOC,
	SYS_FREE,

	SYS_GET_PID,
	SYS_GET_THREAD_ID,
	SYS_FORK,
	SYS_YIELD,
	SYS_WAIT_PID,
	SYS_USLEEP,
	SYS_EXIT,
	SYS_KILL,
	SYS_DETACH,
	SYS_LOCK_NEW,
	SYS_LOCK,
	SYS_UNLOCK,
	SYS_LOCK_FREE,

	SYS_EXEC_ELF,

	SYS_VFS_NEW_NODE,
	SYS_VFS_GET_MOUNT,
	SYS_VFS_GET_MOUNT_BY_ID,
	SYS_VFS_MOUNT,
	SYS_VFS_UMOUNT,
	SYS_VFS_GET,
	SYS_VFS_KIDS,
	SYS_VFS_SET,
	SYS_VFS_ADD,
	SYS_VFS_DEL,
	SYS_VFS_OPEN,
	SYS_VFS_GET_BY_FD,

	SYS_VFS_PROC_CLOSE,
	SYS_VFS_PROC_SEEK,
	SYS_VFS_PROC_TELL,
	SYS_VFS_PROC_GET_BY_FD,
	SYS_VFS_PROC_DUP,
	SYS_VFS_PROC_DUP2,

	SYS_PIPE_OPEN,
	SYS_PIPE_READ,
	SYS_PIPE_WRITE,

	SYS_PROC_GET_CMD,
	SYS_PROC_SET_CWD,
	SYS_PROC_GET_UID,
	SYS_PROC_SET_UID,
	SYS_PROC_GET_CWD,

	SYS_PROC_SET_ENV,
	SYS_PROC_GET_ENV,
	SYS_PROC_GET_ENV_NAME,
	SYS_PROC_GET_ENV_VALUE,

	SYS_PROC_SHM_ALLOC,
	SYS_PROC_SHM_MAP,
	SYS_PROC_SHM_UNMAP,
	SYS_PROC_SHM_REF,

	SYS_GET_SYSINFO,
	SYS_GET_KERNEL_USEC,
	SYS_GET_KERNEL_TIC,
	SYS_GET_PROCS,

	SYS_SET_GLOBAL,
	SYS_GET_GLOBAL,

	SYS_THREAD,

	SYS_MMIO_MAP,
	SYS_FRAMEBUFFER_MAP,

	SYS_PROC_CRITICAL_ENTER,
	SYS_PROC_CRITICAL_QUIT,

	SYS_PROC_IRQ_SETUP,
	SYS_PROC_IRQ_REGISTER,
	SYS_PROC_IRQ_UNREGISTER,
	SYS_PROC_INTERRUPT,
	SYS_INTERRUPT_DATA,

	SYS_IPC_SETUP,
	SYS_IPC_CALL,
	SYS_IPC_GET_ARG,
	SYS_IPC_SET_RETURN,
	SYS_IPC_GET_RETURN,
	SYS_IPC_END,

	SYS_GET_KEVENT
};

#endif
