CLANG := clang-9
FUSE_LD := lld-link

.PHONY: all clean run
default: all

TARGET := bin/efi/boot/BOOTX64.EFI

SRCS += $(shell find src/ -name '*.c')
OBJS := $(SRCS:%=obj/%.o)

INCLUDE_DIRS += edk2/MdePkg/Include
INCLUDE_DIRS += edk2/MdePkg/Include/X64
INCLUDE_DIRS += edk2/MdePkg/Include/Protocol

CFLAGS := \
	-target x86_64-unknown-windows \
	-ffreestanding \
	-fshort-wchar \
	-nostdlib \
	-std=c11 \
	-Wall \
	-Werror \
	-flto \
	-g

CFLAGS += $(INCLUDE_DIRS:%=-I%)

LDFLAGS := \
	-target x86_64-unknown-windows \
	-nostdlib \
	-Wl,-entry:EfiMain \
	-Wl,-subsystem:efi_application \
	-fuse-ld=$(FUSE_LD)

clean:
	rm -rf bin obj

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CLANG) $(LDFLAGS) -o $@ $(OBJS)

remake: clean all

obj/%.c.o: %.c
	@mkdir -p $(@D)
	$(CLANG) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	@qemu-system-x86_64.exe -L externals -bios OVMF.fd -hdd fat:rw:bin
