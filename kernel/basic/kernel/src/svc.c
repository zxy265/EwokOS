#include <kernel/kernel.h>
#include <kernel/interrupt.h>
#include <kernel/svc.h>
#include <kernel/schedule.h>
#include <kernel/system.h>
#include <kernel/proc.h>
#include <kernel/ipc.h>
#include <kernel/hw_info.h>
#include <kernel/kevqueue.h>
#include <kernel/signal.h>
#include <mm/kalloc.h>
#include <mm/shm.h>
#include <mm/kmalloc.h>
#include <sysinfo.h>
#include <dev/uart.h>
#include <dev/timer.h>
#include <syscalls.h>
#include <kstring.h>
#include <kprintf.h>
#include <stddef.h>
#include <dev/fbinfo.h>

static void sys_kprint(const char* s, int32_t len) {
	(void)len;
	printf(s);
}

static void sys_exit(context_t* ctx, int32_t res) {
	ctx->gpr[0] = 0;
	proc_t* cproc = get_current_proc();
	proc_exit(ctx, cproc, res);
	schedule(ctx);
}

static int32_t sys_signal_setup(uint32_t entry) {
	return proc_signal_setup(entry);
}

static void sys_signal(context_t* ctx, int32_t pid, int32_t sig) {
	ctx->gpr[0] = -1;
	proc_t* proc = proc_get(pid);
	proc_t* cproc = get_current_proc();
	if((cproc->info.owner > 0 &&
			cproc->info.owner != proc->info.owner) ||
			proc->info.owner < 0) {
		return;
	}
	proc_signal_send(ctx, proc, sig);
}

static void sys_signal_end(context_t* ctx) {
	proc_signal_end(ctx);
}

static int32_t sys_getpid(int32_t pid) {
	proc_t * cproc = get_current_proc();
	proc_t * proc = cproc;
	if(pid >= 0)
		proc = proc_get(pid);

	if(proc == NULL)
		return -1;

	if(cproc->info.owner > 0 && cproc->info.owner != proc->info.owner)
		return -1;

	proc_t* p = proc_get_proc(proc);
	if(p != NULL)
		return p->info.pid;
	return -1;
}

static int32_t sys_get_threadid(void) {
	proc_t * cproc = get_current_proc();
	if(cproc == NULL || cproc->info.type != PROC_TYPE_THREAD)
		return -1;
	return cproc->info.pid; 
}

static void sys_usleep(context_t* ctx, uint32_t count) {
	proc_t * cproc = get_current_proc();
	if(cproc->ipc_task.state == IPC_IDLE) // no ipc task to handle
		proc_usleep(ctx, count);
}

static int32_t sys_malloc(int32_t size) {
	return (int32_t)proc_malloc(size);
}

static int32_t sys_realloc(void* p, int32_t size) {
	return (int32_t)proc_realloc(p, size);
}

static void sys_free(int32_t p) {
	if(p == 0)
		return;
	proc_free((void*)p);
}

static void sys_fork(context_t* ctx) {
	proc_t *proc;
	proc = kfork(ctx, PROC_TYPE_PROC);
	if(proc == NULL) {
		ctx->gpr[0] = -1;
		return;
	}

	memcpy(&proc->ctx, ctx, sizeof(context_t));
	proc->ctx.gpr[0] = 0;
	ctx->gpr[0] = proc->info.pid;

	if(proc->info.state == CREATED && _core_proc_ready) {
		proc->info.state = BLOCK;
		proc->info.block_by = _core_proc_pid;
		proc->block_event = proc->info.pid;

		proc_t* cproc = get_current_proc();
		cproc->info.state = BLOCK;
		cproc->block_event = proc->info.pid;
		cproc->ctx.gpr[0] = proc->info.pid;
		cproc->info.block_by = _core_proc_pid;
		schedule(ctx);
	}
}

static void sys_detach(void) {
	proc_t* cproc = get_current_proc();
	cproc->info.father_pid = 0;
}

static int32_t sys_thread(context_t* ctx, uint32_t entry, uint32_t func, int32_t arg) {
	proc_t *proc = kfork(ctx, PROC_TYPE_THREAD);
	if(proc == NULL)
		return -1;
	uint32_t sp = proc->ctx.sp;
	memcpy(&proc->ctx, ctx, sizeof(context_t));
	proc->ctx.sp = sp;
	proc->ctx.pc = entry;
	proc->ctx.lr = entry;
	proc->ctx.gpr[0] = func;
	proc->ctx.gpr[1] = arg;
	return proc->info.pid;
}

static void sys_waitpid(context_t* ctx, int32_t pid) {
	proc_waitpid(ctx, pid);
}

static void sys_load_elf(context_t* ctx, const char* cmd, void* elf, uint32_t elf_size) {
	if(elf == NULL) {
		ctx->gpr[0] = -1;
		return;
	}
	proc_t* cproc = get_current_proc();
	strcpy(cproc->info.cmd, cmd);
	if(proc_load_elf(cproc, elf, elf_size) != 0) {
		ctx->gpr[0] = -1;
		return;
	}

	ctx->gpr[0] = 0;
	memcpy(ctx, &cproc->ctx, sizeof(context_t));
}

static int32_t sys_proc_set_uid(int32_t uid) {
	proc_t* cproc = get_current_proc();
	if(cproc->info.owner > 0)	
		return -1;
	cproc->info.owner = uid;
	return 0;
}

static int32_t sys_proc_get_cmd(int32_t pid, char* cmd, int32_t sz) {
	proc_t* proc = proc_get(pid);
	if(proc == NULL)
		return -1;
	strncpy(cmd, proc->info.cmd, sz);
	return 0;
}

static void sys_proc_set_cmd(const char* cmd) {
	proc_t* cproc = get_current_proc();
	strncpy(cproc->info.cmd, cmd, PROC_INFO_CMD_MAX-1);
}

static void	sys_get_sys_info(sys_info_t* info) {
	if(info == NULL)
		return;
	memcpy(info, &_sys_info, sizeof(sys_info_t));
}

static void	sys_get_sys_state(sys_state_t* info) {
	if(info == NULL)
		return;

	info->mem.free = get_free_mem_size();
	info->mem.shared = shm_alloced_size();
	info->kernel_sec = _kernel_sec;
}

static int32_t sys_shm_alloc(uint32_t size, int32_t flag) {
	return shm_alloc(size, flag);
}

static void* sys_shm_map(int32_t id) {
	proc_t* cproc = get_current_proc();
	return shm_proc_map(cproc->info.pid, id);
}

static int32_t sys_shm_unmap(int32_t id) {
	proc_t* cproc = get_current_proc();
	return shm_proc_unmap(cproc->info.pid, id);
}

static int32_t sys_shm_ref(int32_t id) {
	proc_t* cproc = get_current_proc();
	return shm_proc_ref(cproc->info.pid, id);
}
	
static uint32_t sys_mem_map(uint32_t vaddr, uint32_t paddr, uint32_t size) {
	proc_t* cproc = get_current_proc();
	if(cproc->info.owner > 0)
		return 0;
	/*allocatable memory can only mapped by kernel,
	userspace can map upper address such as MMIO/FRAMEBUFFER... */
	if(paddr > _allocatable_mem_base && paddr < _allocatable_mem_top)
		return 0;
	uint32_t no_cache = (size & 0x80000000) == 0 ? 1:0;
	size &= 0x7fffffff;
	map_pages(cproc->space->vm, vaddr, paddr, paddr+size, AP_RW_RW, no_cache);
	flush_tlb();
	return vaddr;
}

static uint32_t sys_kpage_map(void) {
	proc_t* cproc = get_current_proc();
	if(cproc->space->kpage != 0)
		return cproc->space->kpage;

	uint32_t page = (uint32_t)kalloc4k();
	map_page(cproc->space->vm, page, V2P(page), AP_RW_RW, 0);
	flush_tlb();
	cproc->space->kpage = page;
	return page;
}

static void sys_ipc_setup(context_t* ctx, uint32_t entry, uint32_t extra_data, uint32_t flags) {
	proc_t* cproc = get_current_proc();
	ctx->gpr[0] = proc_ipc_setup(ctx, entry, extra_data, flags);
	if((flags & IPC_NON_BLOCK) == 0) {
		cproc->info.state = BLOCK;
		cproc->info.block_by = cproc->info.pid;
		schedule(ctx);
	}
}

static void sys_ipc_call(context_t* ctx, int32_t serv_pid, int32_t call_id, proto_t* data) {
	ctx->gpr[0] = 0;

	proc_t* client_proc = get_current_proc();
	proc_t* serv_proc = proc_get(serv_pid);

	if(serv_proc == NULL ||
			client_proc->info.pid == serv_pid || //can't do self ipc
			serv_proc->space->ipc_server.entry == 0) //no ipc service setup
		return;

	if(serv_proc->ipc_task.state != IPC_IDLE) {
		ctx->gpr[0] = -1; //busy for single task , should retry
		if((call_id & IPC_NON_RETURN) == 0) { //request proc need return
			proc_block_on(serv_pid, (uint32_t)&serv_proc->space->ipc_server);
			schedule(ctx);
		}
		return;
	}

	ipc_task_t* ipc = &serv_proc->ipc_task;
	ipc->state = IPC_BUSY;
	uint32_t uid = (int32_t)proc_ipc_req();
	ctx->gpr[0] = uid;
	ipc->client_pid = client_proc->info.pid;
	ipc->uid = uid;
	ipc->call_id = call_id;
	if(data != NULL)
		proto_copy(&ipc->data, data->data, data->size);

	if((call_id & IPC_NON_RETURN) == 0)
		proc_block_on(serv_pid, (uint32_t)&serv_proc->ipc_task);

	proc_ipc_task(ctx, serv_proc);
}

static int32_t sys_ipc_get_return(context_t* ctx, int32_t pid, uint32_t uid, proto_t* data) {
	proc_t* client_proc = get_current_proc();
	if(uid == 0 || client_proc == NULL)
		return -2;

	if(client_proc->ipc_req.state != IPC_RETURN) { //block retry for serv return
		proc_t* serv_proc = proc_get(pid);
		proc_block_on(pid, (uint32_t)&serv_proc->ipc_task);
		ctx->gpr[0] = -1;
		schedule(ctx);
		return -1;
	}

	if(data != NULL) {//get return value
		data->total_size = data->size = client_proc->ipc_req.data.size;
		if(data->size > 0) {
			data->data = (proto_t*)proc_malloc(client_proc->ipc_req.data.size);
			memcpy(data->data, client_proc->ipc_req.data.data, data->size);
		}
	}

	client_proc->ipc_req.state = IPC_IDLE;
	proto_clear(&client_proc->ipc_req.data);
	return 0;
}

static int32_t sys_ipc_get_info(uint32_t uid, int32_t* pid, int32_t* cmd) {
	proc_t* serv_proc = get_current_proc();
	ipc_task_t* ipc = &serv_proc->ipc_task;
	if(uid == 0 ||
			ipc->uid != uid ||
			ipc->state != IPC_BUSY ||
			serv_proc->space->ipc_server.entry == 0) {
		return 0;
	}

	*pid = ipc->client_pid;
	*cmd = ipc->call_id;

	proto_t* ret = NULL;
	if(ipc->data.size > 0) { //get request input args
		ret = (proto_t*)proc_malloc(sizeof(proto_t));
		memset(ret, 0, sizeof(proto_t));
		ret->data = proc_malloc(ipc->data.size);
		ret->total_size = ret->size = ipc->data.size;
		memcpy(ret->data, ipc->data.data, ipc->data.size);
		proto_clear(&ipc->data);
	}

	return (int32_t)ret;
}

static void sys_ipc_set_return(context_t* ctx, uint32_t uid, proto_t* data) {
	proc_t* serv_proc = get_current_proc();
	ipc_task_t* ipc = &serv_proc->ipc_task;
	if(uid == 0 ||
			ipc->uid != uid ||
			serv_proc->space->ipc_server.entry == 0 ||
			ipc->state != IPC_BUSY) {
		return;
	}

	proc_t* client_proc = proc_get(ipc->client_pid);
	if(client_proc != NULL) {
		client_proc->ipc_req.state = IPC_RETURN;
		if(data != NULL)
			proto_copy(&client_proc->ipc_req.data, data->data, data->size);

		proc_ready(client_proc);
		if(client_proc->info.core == serv_proc->info.core) {
			proc_switch(ctx, client_proc, true);
			return;
		}
		schedule(ctx);
	}
}

static void sys_ipc_end(context_t* ctx) {
	proc_t* serv_proc = get_current_proc();
	ipc_task_t* ipc = &serv_proc->ipc_task;
	if(serv_proc == NULL ||
			serv_proc->space->ipc_server.entry == 0 ||
			ipc->state == IPC_IDLE)
		return;

	memcpy(ctx, &ipc->saved_ctx, sizeof(context_t));
	serv_proc->info.state = ipc->saved_state;
	serv_proc->info.block_by = ipc->saved_block_by;
	if(serv_proc->info.state == READY || serv_proc->info.state == RUNNING)
		proc_ready(serv_proc);

	//wake up request proc to get return
	proc_ipc_close(ipc);
	proc_wakeup(serv_proc->info.pid, (uint32_t)&serv_proc->space->ipc_server); 
	//schedule(ctx);
}

static int32_t sys_ipc_lock(void) {
	proc_t* cproc = get_current_proc();
	if(cproc->ipc_task.state != IPC_IDLE)
		return -1;
	cproc->ipc_task.state = IPC_BUSY;
	return 0;
}

static void sys_ipc_unlock(void) {
	proc_t* cproc = get_current_proc();
	cproc->ipc_task.state = IPC_IDLE;
	proc_wakeup(cproc->info.pid, (uint32_t)&cproc->space->ipc_server);
}

static int32_t sys_proc_ping(int32_t pid) {
	proc_t* proc = proc_get(pid);
	if(proc == NULL || !proc->space->ready_ping)
		return -1;
	return 0;
}

static void sys_proc_ready_ping(void) {
	proc_t* cproc = get_current_proc();
	cproc->space->ready_ping = true;
}

static kevent_t* sys_get_kevent_raw(void) {
	proc_t* cproc = get_current_proc();
	if(cproc->info.owner > 0)	
		return NULL;

	kevent_t* kev = kev_pop();
	if(kev == NULL) {
		return NULL;
	}

	kevent_t* ret = (kevent_t*)proc_malloc(sizeof(kevent_t));
	ret->type = kev->type;
	ret->data[0] = kev->data[0];
	ret->data[1] = kev->data[1];
	ret->data[2] = kev->data[2];
	kfree(kev);
	return ret;
}

static void sys_get_kevent(context_t* ctx) {
	ctx->gpr[0] = 0;	
	kevent_t* kev = sys_get_kevent_raw();
	if(kev == NULL) {
		proc_block_on(-1, (uint32_t)kev_init);
		return;
	}
	ctx->gpr[0] = (int32_t)kev;	
}

static void sys_proc_block(context_t* ctx, int32_t pid, uint32_t evt) {
	proc_t* proc_by = proc_get_proc(proc_get(pid));
	if(proc_by != NULL) {
		proc_block_on(proc_by->info.pid, evt);
	}
	schedule(ctx);	
}

static void sys_proc_wakeup(uint32_t evt) {
	proc_t* proc = proc_get_proc(get_current_proc());
	if(proc->info.block_by == proc->info.pid)
		proc->ipc_task.saved_state = READY;
	proc_wakeup(proc->info.pid, evt);
}

static void sys_core_proc_ready(void) {
	proc_t* cproc = get_current_proc();
	if(cproc->info.owner > 0)
		return;
	_core_proc_ready = true;
	_core_proc_pid = cproc->info.pid;
}

static int32_t sys_core_proc_pid(void) {
	return _core_proc_pid;
}

static uint32_t _svc_tic = 0;
static int32_t sys_get_kernel_tic(void) {
	return _svc_tic;
}

#ifdef LOAD_SD
static int32_t sys_romfs_get(uint32_t index, char* name, char* data) {
	(void)index;
	(void)name;
	(void)data;
	return -1;
}
#else
int32_t romfs_get_by_index(uint32_t index, char* name, char* data);
static int32_t sys_romfs_get(uint32_t index, char* name, char* data) {
	return romfs_get_by_index(index, name, data);
}
#endif

static int32_t sys_interrupt_setup(uint32_t interrupt, uint32_t entry) {
	proc_t * cproc = get_current_proc();
	if(cproc->info.owner > 0)
		return -1;
	interrupt_setup(cproc, interrupt, entry);
	return 0;
}

static void sys_interrupt_end(context_t* ctx) {
	interrupt_end(ctx);
}

static inline void _svc_handler(int32_t code, int32_t arg0, int32_t arg1, int32_t arg2, context_t* ctx) {
	_svc_tic++;

	switch(code) {
	case SYS_EXIT:
		sys_exit(ctx, arg0);
		return;
	case SYS_SIGNAL_SETUP:
		sys_signal_setup(arg0);
		return;
	case SYS_SIGNAL:
		sys_signal(ctx, arg0, arg1);
		return;
	case SYS_SIGNAL_END:
		sys_signal_end(ctx);
		return;
	case SYS_MALLOC:
		ctx->gpr[0] = sys_malloc(arg0);
		return;
	case SYS_REALLOC:
		ctx->gpr[0] = sys_realloc((void*)arg0, arg1);
		return;
	case SYS_FREE:
		sys_free(arg0);
		return;
	case SYS_GET_PID:
		ctx->gpr[0] = sys_getpid(arg0);
		return;
	case SYS_GET_THREAD_ID:
		ctx->gpr[0] = sys_get_threadid();
		return;
	case SYS_USLEEP:
		sys_usleep(ctx, (uint32_t)arg0);
		return;
	case SYS_EXEC_ELF:
		sys_load_elf(ctx, (const char*)arg0, (void*)arg1, (uint32_t)arg2);
		return;
	case SYS_FORK:
		sys_fork(ctx);
		return;
	case SYS_DETACH:
		sys_detach();
		return;
	case SYS_WAIT_PID:
		sys_waitpid(ctx, arg0);
		return;
	case SYS_YIELD: 
		schedule(ctx);
		return;
	case SYS_PROC_SET_UID: 
		ctx->gpr[0] = sys_proc_set_uid(arg0);
		return;
	case SYS_PROC_GET_UID: 
		ctx->gpr[0] = get_current_proc()->info.owner;
		return;
	case SYS_PROC_GET_CMD: 
		ctx->gpr[0] = sys_proc_get_cmd(arg0, (char*)arg1, arg2);
		return;
	case SYS_PROC_SET_CMD: 
		sys_proc_set_cmd((const char*)arg0);
		return;
	case SYS_GET_SYS_INFO:
		sys_get_sys_info((sys_info_t*)arg0);
		return;
	case SYS_GET_SYS_STATE:
		sys_get_sys_state((sys_state_t*)arg0);
		return;
	case SYS_GET_KERNEL_TIC:
		ctx->gpr[0] = sys_get_kernel_tic();
		return;
	case SYS_GET_PROCS: 
		ctx->gpr[0] = (int32_t)get_procs((int32_t*)arg0);
		return;
	case SYS_PROC_SHM_ALLOC:
		ctx->gpr[0] = sys_shm_alloc(arg0, arg1);
		return;
	case SYS_PROC_SHM_MAP:
		ctx->gpr[0] = (int32_t)sys_shm_map(arg0);
		return;
	case SYS_PROC_SHM_UNMAP:
		ctx->gpr[0] = sys_shm_unmap(arg0);
		return;
	case SYS_PROC_SHM_REF:
		ctx->gpr[0] = sys_shm_ref(arg0);
		return;
	case SYS_THREAD:
		ctx->gpr[0] = sys_thread(ctx, (uint32_t)arg0, (uint32_t)arg1, arg2);
		return;
	case SYS_KPRINT:
		sys_kprint((const char*)arg0, arg1);
		return;
	case SYS_MEM_MAP:
		ctx->gpr[0] = sys_mem_map((uint32_t)arg0, (uint32_t)arg1, (uint32_t)arg2);
		return;
	case SYS_KPAGE_MAP:
		ctx->gpr[0] = sys_kpage_map();
		return;
	case SYS_IPC_SETUP:
		sys_ipc_setup(ctx, arg0, arg1, arg2);
		return;
	case SYS_IPC_CALL:
		sys_ipc_call(ctx, arg0, arg1, (proto_t*)arg2);
		return;
	case SYS_IPC_GET_RETURN:
		ctx->gpr[0] = sys_ipc_get_return(ctx, arg0, (uint32_t)arg1, (proto_t*)arg2);
		return;
	case SYS_IPC_SET_RETURN:
		sys_ipc_set_return(ctx, (uint32_t)arg0, (proto_t*)arg1);
		return;
	case SYS_IPC_END:
		sys_ipc_end(ctx);
		return;
	case SYS_IPC_GET_ARG:
		ctx->gpr[0] = sys_ipc_get_info((uint32_t)arg0, (int32_t*)arg1, (int32_t*)arg2);
		return;
	case SYS_PROC_PING:
		ctx->gpr[0] = sys_proc_ping(arg0);
		return;
	case SYS_PROC_READY_PING:
		sys_proc_ready_ping();
		return;
	case SYS_GET_KEVENT:
		sys_get_kevent(ctx);
		return;
	case SYS_WAKEUP:
		sys_proc_wakeup(arg0);
		return;
	case SYS_BLOCK:
		sys_proc_block(ctx, arg0, arg1);
		return;
	case SYS_CORE_READY:
		sys_core_proc_ready();
		return;
	case SYS_CORE_PID:
		ctx->gpr[0] = sys_core_proc_pid();
		return;
	case SYS_IPC_LOCK:
		ctx->gpr[0] = sys_ipc_lock();
		return;
	case SYS_IPC_UNLOCK:
		sys_ipc_unlock();
		return;
	case SYS_KROMFS_GET:
		ctx->gpr[0] = sys_romfs_get(arg0, (char*)arg1, (char*)arg2);
		return;
	case SYS_INTR_SETUP:
		ctx->gpr[0] = sys_interrupt_setup((uint32_t)arg0, (uint32_t)arg1);
		return;
	case SYS_INTR_END:
		sys_interrupt_end(ctx);
		return;
	}
}

inline void svc_handler(int32_t code, int32_t arg0, int32_t arg1, int32_t arg2, context_t* ctx) {
	__irq_disable();
	kernel_lock();
	_svc_handler(code, arg0, arg1, arg2, ctx);
	kernel_unlock();
}
