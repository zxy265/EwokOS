include ../../../../make.rule

BUILD_DIR = ../../../../build
TARGET_DIR = $(BUILD_DIR)/rootfs

LDFLAGS = -L$(BUILD_DIR)/lib -lnosys

CFLAGS += -I $(BUILD_DIR)/include

SYSINFOD_OBJS = sysinfod.o

SYSINFOD = $(TARGET_DIR)/drivers/proc/sysinfod

$(SYSINFOD): $(SYSINFOD_OBJS) \
		$(BUILD_DIR)/lib/libewokc.a
	$(LD) -Ttext=100 $(SYSINFOD_OBJS) -o $(SYSINFOD) $(LDFLAGS) -lewokc -lc

clean:
	rm -f $(SYSINFOD_OBJS)
