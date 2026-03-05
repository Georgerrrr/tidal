SRC_DIR = ./SRC/

CC=wcc386
AS=tasm
LD=wlink

model=c

MESSAGE_DEBUG = Building debug
MESSAGE_PRODUCTION = Building production

VERSION=DEBUG

CDEBUG=-d3 -db
CPRODUCTION=
CFLAGS=$(C$(VERSION)) -i.

ASMDEBUG=-Zi
ASMPRODUCTION=
AFLAGS=$(A$(VERSION)) -m -ml

LDEBUG=debug all
LPRODUCTION=
LFLAGS=$(L$(VERSION))

TIDAL_OBJS=tlMain.obj tlGfx.obj tlAssert.obj tlAlloc.obj tlVga.obj tlMaths.obj

TIDAL.EXE : $(TIDAL_OBJS) TIDAL.LNK
	echo $(MESSAGE_$(VERSION))
	$(LD) $(LFLAGS) @TIDAL.LNK

TIDAL.LNK: $(TIDAL_OBJS)
	%create $^@
	%append $^@ NAME TIDAL.EXE
	%append $^@ SYSTEM DOS4G
	%append $^@ OPTION QUIET
	%append $^@ OPTION STACK=16k
	@for %i in ($(TIDAL_OBJS)) do %append $^@ FILE %i

.c.obj:
	$(CC) $[* $(CFLAGS)

.c: $(SRC_DIR)

.asm.obj:
	$(AS) $[* $(AFLAGS)

.asm: $(SRC_DIR)

clean: .symbolic
	del *.err
	del *.obj
	del *.EXE
	del *.LNK

build: TIDAL.EXE

