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
# These warning categories are benign-by-design in this 1990s code and not worth
# churning the source over: dead bookkeeping vars, the "??" in emote text read as
# trigraphs, cosmetic indentation, int*/socklen_t* mismatch on socket calls, and
# gcc's worst-case sprintf sizing (real overflows were fixed; the rest are bounded
# by short game data like names/area-names). Everything else is fixed in code.
WSUPPRESS = -Wno-unused-but-set-variable -Wno-trigraphs -Wno-misleading-indentation \
            -Wno-pointer-sign -Wno-format-overflow
C_FLAGS = -O -Wall -fcommon -m32 $(WSUPPRESS) $(PROF) $(NOCRYPT)
# -rdynamic exports symbol names so the SIGSEGV backtrace handler in comm.c can
# print readable function names in crash dumps.
L_FLAGS = -O -m32 -rdynamic $(PROF)
# math routines (act_info.c, etc.) need libm. crypt() is macro'd to plaintext
# on Linux in merc.h (original Static Chaos behavior), so no libcrypt is linked.
LIBS    = -lm

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o bit.o board.o clan.o  \
          comm.o const.o db.o fight.o fist.o gmcp.o handler.o interp.o mazoku.o      \
	  magic.o mem.o mob_commands.o mob_prog.o olc.o olc_act.o olc_save.o patryn.o \
	  quest.o saiyan.o save.o special.o soldier.o sorcerer.o string.o suit.o     \
	  update.o

chaosium: $(O_FILES)
	rm -f chaosium 
	$(CC) $(L_FLAGS) -o chaosium $(O_FILES) $(LIBS)

# Pattern rule (not the old ".c.o: merc.h" suffix rule, which make warns about
# and silently drops the merc.h prerequisite). This builds each .o from its .c
# and correctly rebuilds everything when merc.h changes.
%.o: %.c merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f *.o chaosium
