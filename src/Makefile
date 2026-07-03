CC      = gcc
PROF    = -g
NOCRYPT =
# -fcommon: this 1990s Merc code relies on tentative/common global definitions
#   that gcc 10+ rejects under its default -fno-common (multiple-definition link
#   errors). -fcommon restores the legacy behavior.
# -m32: build 32-bit. The code assumes sizeof(int) == sizeof(pointer) == 4; on a
#   64-bit build, pointers stored in/cast through int get truncated and the server
#   crashes during gameplay (combat, etc.). Compiling 32-bit is the standard,
#   reliable fix for Diku/Merc-era code. Needs gcc-multilib + libc6-dev-i386.
# -Iinclude: headers live in src/include/ (the source tree is grouped into
#   subdirs). The .c files use bare includes (#include "merc.h"), so include/ is
#   put on the header search path here.
# These warning categories are benign-by-design in this 1990s code and not worth
# churning the source over: dead bookkeeping vars, the "??" in emote text read as
# trigraphs, cosmetic indentation, int*/socklen_t* mismatch on socket calls, and
# gcc's worst-case sprintf sizing (real overflows were fixed; the rest are bounded
# by short game data like names/area-names). Everything else is fixed in code.
WSUPPRESS = -Wno-unused-but-set-variable -Wno-trigraphs -Wno-misleading-indentation \
            -Wno-pointer-sign -Wno-format-truncation
C_FLAGS = -O -Wall -fcommon -m32 -Iinclude $(WSUPPRESS) $(PROF) $(NOCRYPT)
# -rdynamic exports symbol names so the SIGSEGV backtrace handler in comm.c can
# print readable function names in crash dumps.
L_FLAGS = -O -m32 -rdynamic $(PROF)
# math routines (act_info.c, etc.) need libm. crypt() is macro'd to plaintext
# on Linux in merc.h (original Static Chaos behavior), so no libcrypt is linked.
LIBS    = -lm

# Sources are grouped into subdirs (core/ commands/ classes/ olc/ world/); headers
# in include/. Object paths mirror the source layout. (test.c, xrand.c and
# memwatch.c live under core/ but are intentionally NOT built -- unused/debug.)
O_FILES = core/comm.o core/db.o core/handler.o core/interp.o core/update.o \
          core/fight.o core/save.o core/string.o core/bit.o core/mem.o \
          core/const.o core/gmcp.o \
          commands/act_comm.o commands/act_info.o commands/act_move.o \
          commands/act_obj.o commands/act_wiz.o \
          classes/saiyan.o classes/fist.o classes/patryn.o classes/mazoku.o \
          classes/sorcerer.o classes/soldier.o \
          olc/olc.o olc/olc_act.o olc/olc_save.o \
          world/magic.o world/mob_commands.o world/mob_prog.o world/special.o \
          world/quest.o world/trials.o world/board.o world/clan.o world/suit.o

HEADERS = $(wildcard include/*.h)

chaosium: $(O_FILES)
	rm -f chaosium
	$(CC) $(L_FLAGS) -o chaosium $(O_FILES) $(LIBS)

# Pattern rule: build each .o next to its .c (-o $@), finding headers via
# -Iinclude. Depending on all headers rebuilds everything when any header changes.
%.o: %.c $(HEADERS)
	$(CC) -c $(C_FLAGS) $< -o $@

clean:
	rm -f */*.o *.o chaosium
