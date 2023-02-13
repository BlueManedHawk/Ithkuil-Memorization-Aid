# LICENSE
#
# Copyright © 2022, 2023 Blue-Maned_Hawk. All rights reserved.
#
# You may freely use this work for any purpose, to the extent permitted by law. You may freely make this work available to others by any means, to the extent permitted by law. You may freely modify this work in any way, to the extent permitted by law. You may freely make works derived from this work available to others by any means, to the extent permitted by law.
#
# Should you choose to exercise any of these rights, you must give clear and conspicuous attribution to the original author, and you must not make it seem in any way like the author condones your act of exercising these rights in any way.
#
# Should you choose to exercise the second right listed above, you must make this license clearly and conspicuously available along with the original work, and you must clearly and conspicuously make the information necessary to reconstruct the work available along with the work.
#
# Should you choose to exercise the fourth right listed above, you must put any derived works you construct under a license that grants the same rights as this one under the same conditions and with the same restrictions, you must clearly and conspicuously make that license available alongside the work, you must clearly and conspicuously make the information necessary to reconstruct the work available alongside the work, you must clearly and conspicuously describe the changes which have been made from the original work, and you must not make it seem in any way like your derived works are the original work in any way.
#
# This license only applies to the copyright of this work, and does not apply to any other intellectual property rights, including but not limited to patent and trademark rights.
#
# THIS WORK COMES WITH ABSOLUTELY NO WARRANTY OF ANY KIND, IMPLIED OR EXPLICIT. THE AUTHOR DISCLAIMS ANY LIABILITY FOR ANY DAMAGES OF ANY KIND CAUSED DIRECTLY OR INDIRECTLY BY THIS WORK.

# This is the makefile for ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls.

# Supposedly, this will crash non-GNU makes.  I can't test this, since I don't have any other makes, though not by choice.
ifneq (,)
This makefile requires GNU Make.
endif

CC = clang-15
CFLAGS = -Werror -Wall -Wextra -pedantic
CFLAGS += -Wno-gnu-binary-literal # Workaround for an issue with Clang.
CFLAGS += -std=c2x -glldb ` sdl2-config --cflags ` -Og
LDFLAGS = ` sdl2-config --libs ` -lSDL2_ttf -lm
SRC = $(wildcard Source/*.c Libraries/*.c)
OBJ = $(SRC:.c=.o)

help:
	@echo You need to call the makefile with an argument.  Use \"make release\" to make a release.

Source/%.o: Source/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

Libraries/%.o: Libraries/%.c
	$(CC) -DMJSON_ENABLE_PRINT=0 -DMJSON_ENABLE_BASE64=0 -DMJSON_ENABLE_RPC=0 -c $^ -o $@

release: $(OBJ)
	$(CC) $(LDFLAGS) $^ -o "ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls.elf"
