include ../../../make.rule

BUILD_DIR = ../../../build
TARGET_DIR = $(BUILD_DIR)/rootfs

LDFLAGS = -L$(BUILD_DIR)/lib -lnosys

CFLAGS += -I $(BUILD_DIR)/include

RUNDEV_OBJS = rundev.o

RUNDEV = $(TARGET_DIR)/bin/rundev

$(RUNDEV): $(RUNDEV_OBJS) \
		$(BUILD_DIR)/lib/libewokc.a
	$(LD) -Ttext=100 $(RUNDEV_OBJS) -o $(RUNDEV) $(LDFLAGS) -lewokc -lc

clean:
	rm -f $(RUNDEV_OBJS)

