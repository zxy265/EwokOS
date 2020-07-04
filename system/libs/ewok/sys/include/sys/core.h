#ifndef CORE_H
#define CORE_H

#ifdef __cplusplus
extern "C" {
#endif


enum {
	CORE_CMD_GLOBAL_SET = 0,
	CORE_CMD_GLOBAL_GET,
	CORE_CMD_GLOBAL_DEL,
	CORE_CMD_IPC_SERV_REG,
	CORE_CMD_IPC_SERV_UNREG,
	CORE_CMD_IPC_SERV_GET,
	CORE_CMD_LOCK_NEW,
	CORE_CMD_LOCK_FREE,
	CORE_CMD_LOCK,
	CORE_CMD_UNLOCK
};

#ifdef __cplusplus
}
#endif

#endif
