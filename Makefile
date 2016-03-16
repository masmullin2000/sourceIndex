
ifdef CLANG
  CC=clang
  CPP=clang++
else
  CC=gcc
  CPP=g++
endif

SRC=src
INC=inc $(SRC) ../include/
INCS=$(addprefix -I,$(INC))

ifdef DBG
  OPT_OR_DBG=-g
else
  OPT_OR_DBG=-O3
endif

ifdef SZ
  OPT_OR_DBG+=-DSIZE
endif

C_C=$(CC) -Wall -x c++ $(gDEFs) $(INCS)
C_CPP=$(CC) -Wall -std=c++11 -x c++ $(gDEFs) $(INCS)
COMPILE=-c $< -o $@ $(OPT_OR_DBG)

vpath %.c $(SRC)
vpath %.cpp $(SRC)
vpath %.h %(INC)

IDX=idx
EXE_IDX=$(BLD)/$(IDX)
SRCH=srch
EXE_SRCH=$(BLD)/$(SRCH)

BLD=out

IOBJS=idx.o \
     FileProcessor.o \
     utils.o \
     FileList.o \
     SqliteAdapter.o \
     SqliteAdapterInsert.o

SOBJS=srch.o

IDX_OBJS=$(addprefix $(BLD)/,$(IOBJS))
SRCH_OBJS=$(addprefix $(BLD)/,$(SOBJS))

test: all
	./$(IDX) files.lst

clean:
	-rm -rf $(BLD)
	-rm $(IDX)
	-rm $(SRCH)
	-rm *.db*

all: $(IDX) $(SRCH)

$(SRCH): $(BLD) $(EXE_SRCH)
	-cp $(EXE_SRCH) $(SRCH)

$(IDX): $(BLD) $(EXE_IDX)
	-cp $(EXE_IDX) $(IDX)

$(EXE_IDX): $(IDX_OBJS)
	$(CPP) -o $(EXE_IDX) $^ -lpthread -lsqlite3

$(EXE_SRCH): $(SRCH_OBJS)
	$(CPP) -o $(EXE_SRCH) $^ -lpthread -lsqlite3

$(BLD):
	-mkdir $(BLD)

$(BLD)/%.o : %.cpp
	$(C_CPP) $(COMPILE)

$(BLD)/%.o : %.c
	$(C_C) $(COMPILE)