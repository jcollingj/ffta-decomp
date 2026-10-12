AS      := arm-none-eabi-as
.DEFAULT_GOAL := all
LD      := arm-none-eabi-ld
OBJCOPY := arm-none-eabi-objcopy
CPP     := arm-none-eabi-cpp
AGBCC   := tools/.deps/agbcc/gcc/agbcc
CFLAGS  := -O2 -mthumb-interwork

ROM     := build/ffta.gba
ELF     := build/ffta.elf
# Link order = ROM layout order
OBJS    := \
	build/external_symbols.o \
	build/header.o \
	build/rom.o \
	build/random.o \
	build/rom_content_prefix.o \
	build/content_flags.o \
	build/rom_content_flag_clear.o \
	build/content_index.o \
	build/rom_random_tail.o \
	build/list_count_wrappers.o \
	build/rom_list_count_wrappers_suffix.o \
	build/list_count.o \
	build/rom_list_accessors_prefix.o \
	build/list_accessors.o \
	build/rom_random_tail_suffix.o \
	build/list_search_callback.o \
	build/list_callbacks.o \
	build/unit_fields.o \
	build/unit_pool.o \
	build/rom_fields_tail.o \
	build/unit_job_set.o \
	build/rom_unit_set_tail.o \
	build/content_values.o \
	build/job_property.o \
	build/rom_job_tail.o \
	build/unit_property.o \
	build/job_mapping.o \
	build/game_flags.o \
	build/save_access.o \
	build/rom_flags_tail.o \
	build/item.o \
	build/inventory.o \
	build/rom_item_tail.o \
	build/inventory_entry_byte3.o \
	build/rom_inventory_entry_tail.o \
	build/equipment_inspect.o \
	build/rom_equipment_inspect_tail.o \
	build/inventory_partition.o \
	build/rom_inventory_partition_tail.o \
	build/inventory_lookup.o \
	build/special_item_mapping.o \
	build/unit_equipment_mask.o \
	build/record_decode.o \
	build/rom_decode_tail.o \
	build/content_pointer_helpers.o \
	build/equipment_scan.o \
	build/job_item_eligibility.o \
	build/item_helpers.o \
	build/unit_record14.o \
	build/rom_pointer_tail.o \
	build/record_copy_source.o \
	build/save_milestone.o \
	build/item_amount.o \
	build/rom_record_source_tail.o \
	build/inventory_class.o \
	build/rom_inventory_class_tail.o \
	build/unit_pool_action.o \
	build/unit_pool_set.o \
	build/rom_pool_action_tail.o \
	build/random_item_count.o \
	build/inventory_filter.o \
	build/rom_random_items_tail.o \
	build/item_adjustment.o \
	build/item_adjustment_list.o \
	build/rom_adjustment_tail.o \
	build/collection_slots.o \
	build/mapped_job.o \
	build/save_collection.o \
	build/unit_pool_predicate.o \
	build/unit_byte_amount.o \
	build/unit_pool_process.o \
	build/item_adjustment_record.o \
	build/ability.o \
	build/ability_list.o \
	build/item_abilities.o \
	build/rom_ability_tail.o \
	build/job_ability_progress.o \
	build/ability_state.o \
	build/ability_state_setter.o \
	build/ability_state_query.o \
	build/ability_slots.o \
	build/unit_status.o \
	build/unit_state_access.o \
	build/unit_counters.o \
	build/record70.o \
	build/rom_slots_prefix.o \
	build/class_records4.o \
	build/rom_slots_tail.o \
	build/equipment_class_masks.o \
	build/items.o \
	build/item_ability_sets.o \
	build/rom_item_sets_tail.o \
	build/jobs.o \
	build/job_requirements.o \
	build/job_mapping_data.o \
	build/rom_mapping_prefix.o \
	build/class_records16.o \
	build/rom_mapping_tail.o \
	build/counted_record_ids.o \
	build/rom_counted_ids_tail.o \
	build/title_palette.o \
	build/rom2.o

BASEROM_SHA1 := 4ac05441f4de70a4ec3dd932116346c61b8783d9

# COMPARE=0 skips the byte-match check (for modified builds)
COMPARE ?= 1

.PHONY: all clean mod-ability-name mod-field-aid check progress compiler compare

compiler: $(AGBCC)

$(AGBCC):
	python3 tools/decomp/bootstrap_agbcc.py

compare: $(ROM)
	python3 tools/decomp/compare.py --no-build

progress: all compare
	python3 tools/decomp/progress.py

check: all compare
	python3 -m unittest discover -s tests -v
	python3 tools/decomp/progress.py --output build/check-progress.json

mod-ability-name: all
	python3 tools/mods/rename_ability.py

mod-field-aid: all
	python3 tools/mods/field_aid.py

all: $(ROM)
ifeq ($(COMPARE),1)
	@echo "$(BASEROM_SHA1)  $(ROM)" | shasum -a 1 -c && echo "ROM matches baserom ✔"
else
	@echo "Built $(ROM) (compare skipped)"
endif

build/%.o: src/%.c $(AGBCC) Makefile config/toolchain.json
	@mkdir -p build
	$(CPP) -P -Iinclude -MMD -MP -MF build/$*.d -MT $@ $< -o build/$*.i
	$(AGBCC) $(CFLAGS) build/$*.i -o build/$*.s
	$(AS) -mcpu=arm7tdmi -o $@ build/$*.s

build/%.o: asm/%.s baserom.gba
	@mkdir -p build
	$(AS) -mcpu=arm7tdmi -o $@ $<

build/%.o: data/%.s
	@mkdir -p build
	$(AS) -mcpu=arm7tdmi -o $@ $<

$(ELF): $(OBJS) ld/rom.ld
	$(LD) -T ld/rom.ld -Map build/ffta.map -o $@ $(OBJS)

$(ROM): $(ELF)
	$(OBJCOPY) -O binary $< $@
	@python3 tools/gbafix.py $@

clean:
	rm -rf build

# Compiler-generated transitive header dependencies keep recovery iterations small.
-include $(wildcard build/*.d)
