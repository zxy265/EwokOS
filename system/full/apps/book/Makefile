SYS_DIR=../../..
include $(SYS_DIR)/make.rule

BUILD_DIR = $(SYS_DIR)/build
TARGET_DIR = $(BUILD_DIR)/rootfs

LDFLAGS = -L$(BUILD_DIR)/lib -lnosys

CXXFLAGS += -I $(BUILD_DIR)/include

BOOK_OBJS = book.o

BOOK_DIR = $(TARGET_DIR)/apps/book
BOOK = $(BOOK_DIR)/book

all: $(BOOK_DIR) $(BOOK)

$(BOOK): $(BOOK_OBJS) \
		$(BUILD_DIR)/lib/libewokc.a \
		$(BUILD_DIR)/lib/libx++.a \
		$(BUILD_DIR)/lib/libx.a \
		$(BUILD_DIR)/lib/libttf.a \
		$(BUILD_DIR)/lib/libcxx.a 
	$(LD) -Ttext=100 $(BOOK_OBJS) -o $(BOOK) $(LDFLAGS) -lttf -lgraph -lupng -lx++ -lx -lsconf -lewokc -lc -lcxx

$(BOOK_DIR):
	mkdir -p $(BOOK_DIR)
	cp -r res $(BOOK_DIR)

clean:
	rm -f $(BOOK_OBJS)
	rm -fr $(BOOK_DIR)

