include ../../../make.rule

BUILD_DIR = ../../../build
TARGET_DIR = $(BUILD_DIR)

LDFLAGS = -L$(TARGET_DIR)/lib -lnosys

CFLAGS += -Iinclude -I $(TARGET_DIR)/include

LIB_SD_OBJS = src/sd.o

SD = $(TARGET_DIR)/lib/libsd.a

$(SD): $(LIB_SD_OBJS)
	$(AR) rT $(SD) $(LIB_SD_OBJS)
	cp -r include/* $(TARGET_DIR)/include


clean:
	rm -f $(LIB_SD_OBJS)
	rm -f $(SD)

