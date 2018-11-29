#
# Tile Lib Makefile
#
# By default, this will build out/default/libtile.a with the default compiler
# and archiver for the platform. You can override some variables:
#
#   CC        Set the compiler
#   AR        Set the archiver
#   PLATFORM  Change the output directory to out/$(PLATFORM)
#   CFLAGS    Pass in some compiler flags
#   TARGET    Specify a different place for the .a file
#
# Ex: make CC=armcc CFLAGS='-I.' TARGET='./libtile.a'
#

PLATFORM ?= default

$(info compiling with CC as $(CC), AR as $(AR), and PLATFORM as $(PLATFORM))

TILE_FW_DIR := ..

SRCDIRS := src src/toa src/drivers src/crypto
INCDIRS := include $(SRCDIRS)

override CFLAGS += -std=c99 -Wall -Werror

OUTDIR  := out/$(PLATFORM)
TARGET  := $(OUTDIR)/libtile.a
LIBRARY := 1

include $(TILE_FW_DIR)/tile_tools/Makefile.generic

