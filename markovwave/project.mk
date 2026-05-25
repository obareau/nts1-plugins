PROJECT = markovwave

UCSRC = markovwave.c

UCXXSRC =

UINCDIR =

UDEFS =

ULIB =

ULIBDIR =

# Use system-installed arm-none-eabi-gcc instead of bundled toolchain
GCC_BIN_PATH = $(dir $(shell which arm-none-eabi-gcc))
