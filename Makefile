CLANG := clang-9
FUSE_LD := lld-link

.PHONY: all clean
default: all

SRCS += $(shell find src/ -name '*.c')
OBJS := $(SRCS:%=obj/%.o)
INCLUDE_DIRS += edk2/MdePkg/Include
INCLUDE_DIRS += edk2/MdePkg/Include/X64

CFLAGS := \
	-target x86_64-unknown-windows \
	-ffreestanding \
	-fshort-wchar \
	-nostdinc \
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

all: ./bin/efi/boot/BOOTX64.EFI

bin/efi/boot/BOOTX64.EFI: $(OBJS)
	@mkdir -p $(@D)
	$(CLANG) $(LDFLAGS) -o $@ $(OBJS)

obj/%.c.o: %.c
	@mkdir -p $(@D)
	$(CLANG) $(CFLAGS) -c -o $@ $<
