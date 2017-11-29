

PROGNAME = batmanVjoker


OFILES = project.o font.o my_images.o


HFILES = myLib.h my_images.h

################################################################################
# These are various settings used to make the GBA toolchain work
# DO NOT EDIT BELOW.
################################################################################
include res/GBAVariables.mak

.PHONY: all
all : CFLAGS += $(CRELEASE)
all: $(PROGNAME).gba
	@echo "[FINISH] Created $(PROGNAME).gba"

$(PROGNAME).gba : $(PROGNAME).elf
	@echo "[LINK] Linking objects together to create $(PROGNAME).gba"
	@$(OBJCOPY) -O binary $(PROGNAME).elf $(PROGNAME).gba

$(PROGNAME).elf : res/crt0.o $(GCCLIB)/crtbegin.o $(GCCLIB)/crtend.o $(GCCLIB)/crti.o $(GCCLIB)/crtn.o $(OFILES)
	@$(CC) $(LINKFLAGS) -o $(PROGNAME).elf $^ -lgcc -lc $(LDDEBUG)
	@rm -f *.d


%.o : %.c
	@echo "[COMPILE] Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

%.o : %.s                                                                        
	@echo "[ASSEMBLE] Assembling $<"                                               
	@$(AS) $(MODEL) $< -o $@

.PHONY : vba
vba : CFLAGS += $(CRELEASE)
vba : $(PROGNAME).gba
	@echo "[EXECUTE] Running Emulator VBA-M"
	@vbam $(VBAOPT) $(PROGNAME).gba > /dev/null 2> /dev/null

.PHONY : mgba
mgba : CFLAGS += $(CRELEASE)
mgba : $(PROGNAME).gba
	@echo "[EXECUTE] Running Emulator VBA-M"
	@mgba $(PROGNAME).gba > /dev/null 2> /dev/null

.PHONY : med
med : CFLAGS += $(CRELEASE)
med : $(PROGNAME).gba
	@echo "[EXECUTE] Running emulator Mednafen"
	@mednafen $(PROGNAME).gba > /dev/null 2>&1

.PHONY : clean
clean :
	@echo "[CLEAN] Removing all compiled files"
	@rm -f *.o *.elf *.gba *.d res/*.o res/*.d

-include $(CFILES:%.c=%.d)
