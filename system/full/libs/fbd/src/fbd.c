#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/vfs.h>
#include <sys/syscall.h>
#include <sys/vdevice.h>
#include <sys/shm.h>
#include <fb/fb.h>
#include <fbd/fbd.h>

typedef struct {
	void* data;
	uint32_t size;
	int32_t shm_id;
} fb_dma_t;

static fbinfo_t* _fbinfo = NULL;
static int32_t _rotate = 0;
static fbd_t* _fbd = NULL;

static int fb_fcntl(int fd, 
		int from_pid,
		fsinfo_t* info, 
		int cmd, 
		proto_t* in, 
		proto_t* out,
		void* p) {

	(void)fd;
	(void)from_pid;
	(void)info;
	(void)in;
	(void)p;
	if(cmd == FB_CNTL_GET_INFO) { //get fb size
		if(_rotate == G_ROTATE_N90 || _rotate == G_ROTATE_90)
			PF->addi(out, _fbinfo->height)->addi(out, _fbinfo->width)->addi(out, _fbinfo->depth);
		else
			PF->addi(out, _fbinfo->width)->addi(out, _fbinfo->height)->addi(out, _fbinfo->depth);
	}
	return 0;
}

static int fb_dma_init(fb_dma_t* dma) {
	memset(dma, 0, sizeof(fb_dma_t));
	//dma->shm_id = shm_alloc(_fbinfo->size_max, 1);
	uint32_t sz = _fbinfo->width*_fbinfo->height*4;
	dma->shm_id = shm_alloc(sz, 1);
	if(dma->shm_id <= 0)
		return -1;
	//dma->size = _fbinfo->size_max;
	dma->size = sz;
	dma->data = shm_map(dma->shm_id);
	if(dma->data == NULL)
		return -1;
	return 0;
}

static int fb_dev_cntl(int from_pid, int cmd, proto_t* in, proto_t* ret, void* p) {
	(void)from_pid;
	(void)ret;
	(void)p;

	if(cmd == FB_DEV_CNTL_SET_INFO) { //set fb size and bpp
		int w = proto_read_int(in);
		int h = proto_read_int(in);
		int bpp = proto_read_int(in);
		if(_fbd->init(w, h, bpp) != 0)
			return -1;
		_fbinfo = _fbd->get_info();
	}
	else if(cmd == FB_DEV_CNTL_GET_INFO) {
		if(_rotate == G_ROTATE_N90 || _rotate == G_ROTATE_90)
			PF->addi(ret, _fbinfo->height)->addi(ret, _fbinfo->width)->addi(ret, _fbinfo->depth);
		else
			PF->addi(ret, _fbinfo->width)->addi(ret, _fbinfo->height)->addi(ret, _fbinfo->depth);
	}	
	return 0;
}

static int32_t do_flush(fb_dma_t* dma) {
	const void* buf = dma->data;
	uint32_t size = dma->size;
	return (int32_t)_fbd->flush(_fbinfo, buf, size, _rotate);
}

/*return
0: error;
-1: resized;
>0: size flushed*/
static int do_fb_flush(int fd, int from_pid, fsinfo_t* info, void* p) {
	(void)fd;
	(void)from_pid;
	(void)info;
	fb_dma_t* dma = (fb_dma_t*)p;
	return do_flush(dma);
}

static int fb_dma(int fd, int from_pid, fsinfo_t* info, int* size, void* p) {
	(void)fd;
	(void)from_pid;
	(void)info;
	fb_dma_t* dma = (fb_dma_t*)p;
	*size = dma->size;
	return dma->shm_id;
}

int fbd_run(fbd_t* fbd, int argc, char** argv) {
	_fbd = fbd;
	_rotate = 0;
	const char* mnt_name = argc > 1 ? argv[1]: "/dev/fb0";
	int w = 640;
	int h = 480;

	if(argc > 3) {
		w = atoi(argv[2]);
		h = atoi(argv[3]);
	}

	if(argc > 4) {
		_rotate = atoi(argv[4]);
	}

	fb_dma_t dma;
	dma.shm_id = 0;
	if(fbd->init(w, h, 32) != 0)
		return -1;
	_fbinfo = fbd->get_info();
	
	if(fb_dma_init(&dma) != 0)
		return -1;

	vdevice_t dev;
	memset(&dev, 0, sizeof(vdevice_t));
	strcpy(dev.name, "framebuffer");
	dev.dma = fb_dma;
	dev.flush = do_fb_flush;
	dev.fcntl = fb_fcntl;
	dev.dev_cntl = fb_dev_cntl;

	dev.extra_data = &dma;
	device_run(&dev, mnt_name, FS_TYPE_CHAR);
	shm_unmap(dma.shm_id);
	return 0;
}
