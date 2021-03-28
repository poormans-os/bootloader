CLANG := clang-9
AS := nasm
FUSE_LD := lld-link

.PHONY: all clean run
default: all

TARGET := bin/efi/boot/BOOTX64.EFI

SRCS += $(shell find src/ -name '*.c')
SRCS += $(shell find lang/ -name '*.c')

OBJS := $(SRCS:%=build/%.o)

INCLUDE_DIRS += include
INCLUDE_DIRS += lang

INCLUDE_DIRS += edk2/MdePkg/Include
INCLUDE_DIRS += edk2/MdePkg/Include/GUID
INCLUDE_DIRS += edk2/MdePkg/Include/X64
INCLUDE_DIRS += edk2/MdePkg/Include/Protocol
INCLUDE_DIRS += edk2/basetools/source/c/genfw

INCLUDE_DIRS += edk2/MdePkg/Include/Pi
INCLUDE_DIRS += edk2/MdePkg/Include/Library
INCLUDE_DIRS += edk2/MdePkg/Include/Library/baselib
INCLUDE_DIRS += edk2/MdeModulePkg/Include/Guid

ASFLAGS = -fwin64

CFLAGS := \
	-target x86_64-unknown-windows \
	-ffreestanding \
	-fshort-wchar \
	-nostdlib \
	-std=c11 \
	-Wall \
	-Werror \
	-fasm-blocks \
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
	@rm -rf build
	@rm -rf obj

all: $(TARGET)

$(TARGET): $(ASM_OBJS) $(OBJS)
	@mkdir -p $(@D)
	@$(CLANG) $(LDFLAGS) -o $@ $(OBJS) $(ASM_OBJS)

remake: clean all

build/%.c.o: %.c
	@mkdir -p $(@D)
	@echo "\033[35m[Compiling]\033[0m $@"
	@$(CLANG) $(CFLAGS) -c -o $@ $<

run: $(TARGET) 
	@mkdir -p $(@D)
	@echo "\033[36m[Running on qemu]\033[0m"
	@qemu-system-x86_64 -s -L externals -bios externals/OVMF.fd -hdd fat:rw:bin --enable-kvm -cpu host -smp 4,sockets=1,cores=2,threads=2 -m 4096 --monitor stdio

