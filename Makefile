# LICENSE
#
# Copyright © 2022 Blue-Maned_Hawk.  All rights reserved.
#
# This project should have come with a file called `LICENSE`.  In the event of any conflict between this comment and that file, that file shall be considered the authority.
#
# You may freely use this software.  You may freely distribute this software, so long as you distribute the license and source code with it.  You may freely modify this software and distribute the modifications under a similar license, so long as you distribute the sources with them and you don't claim that they're the original software.  None of this overrides local laws, and if you excercise these rights, you cannot claim that your actions are condoned by the author.
#
# This license does not apply to patents or trademarks.
#
# This software comes with no warranty, implied or explicit.  The author disclaims any liability for damages caused by this software.

# This is the makefile for ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls.

# Supposedly, this will crash non-GNU makes.  I can't test this, since I don't have any other makes.  (I should also clarify that I'm using GNU make not because I support the FSF, but because it's the only make I have access to.
ifneq (,)
This makefile requires GNU Make.
endif

CC = clang-13
CFLAGS = -Wall -Werror -Wextra -pedantic
CFLAGS += -std=c2x -Og -glldb `pkg-config --cflags glfw3 `
LDFLAGS = ` pkg-config --libs glfw3 `
SRC = $(wildcard Source/*.c)
OBJ = $(SRC:.c=.o)

help:
	@echo You need to call the makefile with an argument.  Use \"make release\" to make a release.

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

release: $(OBJ)
	$(CC) $(LDFLAGS) $^ -o "ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls.elf"
