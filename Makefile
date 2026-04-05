SRC_DIR = ./SRC/
TESTS_DIR = ./TESTS/

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

TIDAL_OBJS=tlMain.obj tlGfx.obj tlAssert.obj tlAlloc.obj tlVga.obj tlObj.obj tlMaths.obj tlStr.obj tlBitmap.obj

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

TEST_OBJS=tests.obj tlAssert.obj tlAlloc.obj

TESTS.EXE : $(TEST_OBJS) TESTS.LNK 
	echo $(MESSAGE_$(VERSION))
	$(LD) $(LFLAGS) @TESTS.LNK

TESTS.LNK: $(TEST_OBJS)
	%create $^@
	%append $^@ NAME TESTS.EXE
	%append $^@ SYSTEM DOS4G
	%append $^@ OPTION QUIET
	%append $^@ OPTION STACK=16k
	@for %i in ($(TEST_OBJS)) do %append $^@ FILE %i

.c.obj:
	$(CC) $[* $(CFLAGS)

.c: $(SRC_DIR);$(TESTS_DIR)

.asm.obj:
	$(AS) $[* $(AFLAGS)

.asm: $(SRC_DIR)

clean: .symbolic
	del *.err
	del *.obj
	del *.EXE
	del *.LNK
	del *.mbr

build: TIDAL.EXE
tests: TESTS.EXE 
all: build tests

