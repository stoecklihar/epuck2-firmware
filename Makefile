##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16 -std=gnu99

  # Aseba doesn't build with strict aliasing
  USE_OPT += -fno-strict-aliasing

  # Protection against stack overflows
  USE_OPT += -fstack-protector-all -L .
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT =
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = -lm
endif

# Enable this if you want link time optimizations (LTO)
ifeq ($(USE_LTO),)
  USE_LTO = no
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = no
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x400
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

ifeq ($(USE_ASEBA_BOOTLOADER),)
	USE_ASEBA_BOOTLOADER=no
endif

# Enables the use of FPU on Cortex-M4 (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = hard
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = epuck2

# Imported source files and paths
CHIBIOS = ./ChibiOS/
# Startup files.
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/startup_stm32f4xx.mk
# HAL-OSAL files.
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx/platform.mk
include $(CHIBIOS)/os/hal/osal/rt/osal.mk
# RTOS files.
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/rt/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk
# Other files.
include $(CHIBIOS)/test/rt/test.mk
include $(CHIBIOS)/os/various/fatfs_bindings/fatfs.mk

include src/src.mk

ifeq ($(USE_ASEBA_BOOTLOADER),yes)
	LDSCRIPT= stm32f407xG.ld
else
	LDSCRIPT= stm32f407xG_no_bootloader.ld
endif


# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC += $(STARTUPSRC) \
        $(KERNSRC) \
        $(PORTSRC) \
        $(OSALSRC) \
        $(HALSRC) \
        $(PLATFORMSRC) \
        $(BOARDSRC) \
        $(TESTSRC) \
        $(CHIBIOS)/os/various/shell.c \
        $(CHIBIOS)/os/hal/lib/streams/memstreams.c \
        $(CHIBIOS)/os/hal/lib/streams/chprintf.c \
		${CHIBIOS}/os/various/fatfs_bindings/fatfs_diskio.c \
        ${CHIBIOS}/os/various/fatfs_bindings/fatfs_syscall.c \
        $(ASEBASRC)

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC =

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = $(STARTUPASM) $(PORTASM) $(OSALASM)

INCDIR += $(STARTUPINC) $(KERNINC) $(PORTINC) $(OSALINC) \
          $(HALINC) $(PLATFORMINC) $(BOARDINC) $(TESTINC) \
          $(CHIBIOS)/os/various \
          $(CHIBIOS)/os/hal/lib/streams \
          $(ASEBAINC) \
		  src

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m4

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy -j startup -j constructors -j destructors -j .text -j .ARM.extab -j .ARM.exidx -j .eh_frame_hdr -j .eh_frame -j .textalign -j .data
AS   = $(TRGT)gcc -x assembler-with-cpp
AR   = $(TRGT)ar
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
NM   = $(TRGT)nm
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra -Wundef

#
# Compiler settings
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS = -DMPU_DETAILED_MSG

UDEFS += -DSTDOUT_SD=SDU1 -DSTDIN_SD=SDU1

ifeq ($(USE_ASEBA_BOOTLOADER),yes)
	UDEFS += -DCORTEX_VTOR_INIT=0x08020000
endif

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# End of user defines
##############################################################################

RULESPATH = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC
include $(RULESPATH)/rules.mk

# Empty libraries, required by stack smashing protection
PRE_MAKE_ALL_RULE_HOOK: libssp.a libssp_nonshared.a
libssp.a:
	arm-none-eabi-ar rcs $@

libssp_nonshared.a:
	arm-none-eabi-ar rcs $@

# Generates a ctags file containing the correct definition for the build
.PHONY: ctags
ctags:
	@echo "Generating ctags file..."
	@cat .dep/*.d | grep ":$$" | sed "s/://" | sort | uniq | xargs ctags  --extra=+qf $(CSRC) $(CPPSRC)

flash: build/$(PROJECT).elf
	openocd -f oocd.cfg -c "program build/$(PROJECT).elf verify reset exit"

format:
	uncrustify -c uncrustify.cfg --replace --no-backup `find src -name "*.c"`
	uncrustify -c uncrustify.cfg --replace --no-backup `find src -name "*.h"`

.PHONY: mem_info
mem_info: $(PROJECT).elf
	@$(NM) --size-sort --print-size $(PROJECT).elf > $(BUILDDIR)/$(PROJECT).mem_size.txt
	@$(NM) --numeric-sort --print-size $(PROJECT).elf > $(BUILDDIR)/$(PROJECT).mem_layout.txt


