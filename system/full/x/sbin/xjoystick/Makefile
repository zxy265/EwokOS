include ../../../../make.rule

BUILD_DIR = ../../../../build
TARGET_DIR = $(BUILD_DIR)/rootfs

LDFLAGS = -L$(BUILD_DIR)/lib -lnosys

CFLAGS += -I $(BUILD_DIR)/include

XJOYSTICKD_OBJS = xjoystickd.o 

XJOYSTICKD = $(TARGET_DIR)/sbin/x/xjoystickd

$(XJOYSTICKD): $(XJOYSTICKD_OBJS) \
		$(BUILD_DIR)/lib/libewokc.a 
	$(LD) -Ttext=100 $(XJOYSTICKD_OBJS) -o $(XJOYSTICKD) $(LDFLAGS) -lewokc -lc

clean:
	rm -f $(XJOYSTICKD_OBJS)
