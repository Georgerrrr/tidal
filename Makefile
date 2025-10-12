SRC_DIR  =./src/
TEST_DIR =./test/

CC=wcc386
AS=tasm
LD=wlink

DEL=del

MODEL=c

MESSAGE_DEBUG= Linking debug build
MESSAGE_PRODUCTION= Linking production build

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

TIDE_OBJS = alloc.obj tlAssert.obj gfx.obj 
TEST_OBJS = test.obj 

tide.lbc : $(TIDE_OBJS)
		%create $^@ 
    @for %i in ($(TIDE_OBJS)) do %append $^@ +%i

tide.lib : $(TIDE_OBJS) tide.lbc
		echo $(MESSAGE_$(VERSION))
		wlib -n -q -b tide.lib @tide.lbc

test.exe : $(TEST_OBJS) test.lnk 
		echo $(MESSAGE_$(VERSION))
		$(LD) $(LFLAGS) @test.lnk

test.lnk : $(TEST_OBJS) tide.lib
		%create $^@
		%append $^@ NAME test.exe
    %append $^@ SYSTEM DOS4G
    %append $^@ OPTION QUIET
    %append $^@ OPTION STACK=16k
		%append $^@ LIBRARY tide.lib
    @for %i in ($(TEST_OBJS)) do %append $^@ FILE %i

.c.obj:
		$(CC) $[* $(CFLAGS)

.c: $(SRC_DIR);$(TEST_DIR)

.asm.obj:
		$(AS) $[* $(AFLAGS)

.asm: $(SRC_DIR);$(TEST_DIR)

tidal: tide.lib
test: test.exe

clean: .symbolic
		$(DEL) *.err
    $(DEL) *.obj
    $(DEL) *.mbr
		$(DEL) *.lnk

