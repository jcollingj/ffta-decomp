AS      := arm-none-eabi-as
LD      := arm-none-eabi-ld
OBJCOPY := arm-none-eabi-objcopy

ROM     := build/ffta.gba
ELF     := build/ffta.elf
# Link order = ROM layout order
OBJS    := build/header.o build/rom.o build/title_palette.o build/rom2.o

BASEROM_SHA1 := 4ac05441f4de70a4ec3dd932116346c61b8783d9

# COMPARE=0 skips the byte-match check (for modified builds)
COMPARE ?= 1

.PHONY: all clean

all: $(ROM)
ifeq ($(COMPARE),1)
	@echo "$(BASEROM_SHA1)  $(ROM)" | shasum -a 1 -c && echo "ROM matches baserom ✔"
else
	@echo "Built $(ROM) (compare skipped)"
endif

build/%.o: asm/%.s baserom.gba
	@mkdir -p build
	$(AS) -mcpu=arm7tdmi -o $@ $<

build/%.o: data/%.s
	@mkdir -p build
	$(AS) -mcpu=arm7tdmi -o $@ $<

$(ELF): $(OBJS) ld/rom.ld
	$(LD) -T ld/rom.ld -o $@ $(OBJS)

$(ROM): $(ELF)
	$(OBJCOPY) -O binary $< $@
	@python3 tools/gbafix.py $@

clean:
	rm -rf build
