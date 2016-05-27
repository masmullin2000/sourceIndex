ifdef CLANG
  CC=clang
  CPP=clang++
  AR=llvm-ar
else
  CC=gcc
  CPP=g++
  AR=ar
endif

INSTALL_DIR=/usr/bin
SRC=src                           \
    src/parsers

INC=inc                           \
  $(SRC)                          \
  ../include/                     \
  /usr/lib/$(LLVM)/include        \
  inc/parsers

INCS=$(addprefix -I,$(INC))

#LIB_PATH=-L./$(BLD) -L../sql/.libs
LLVM=llvm-3.6
LIBRARY_PATH=                     \
  ./$(BLD)                        \
  ../sql/.libs                    \
  /usr/lib/$(LLVM)/lib

LIB_PATH=$(addprefix -L,$(LIBRARY_PATH))

ifdef DBG
  OPT_OR_DBG=-g
else
  OPT_OR_DBG=-O3
endif

C_C=$(CC) -Wall -x c++ $(gDEFs) $(INCS)
C_CPP=$(CC) -Wall -std=c++11 -x c++ $(gDEFs) $(INCS)
COMPILE=-c $< -o $@ $(OPT_OR_DBG)

vpath %.c $(SRC)
vpath %.cpp $(SRC)
vpath %.h %(INC)

IDX=idx
IDX_L=libGrokIdx.a
EXE_IDX=$(BLD)/$(IDX)
IDX_LIB=$(BLD)/$(IDX_L)
SRCH=srch
SRCH_L=libGrokSrch.a
EXE_SRCH=$(BLD)/$(SRCH)
SRCH_LIB=$(BLD)/$(SRCH_L)

BLD=out

IOBJS=\
     FileProcessor.o            \
     utils.o                    \
     FileList.o                 \
     SqliteAdapter.o            \
     SqliteAdapterInsert.o      \
     FileParser.o

SOBJS=\
    SqliteAdapter.o \
    SqliteAdapterQuery.o

IDX_OBJS=$(addprefix $(BLD)/,$(IOBJS))
SRCH_OBJS=$(addprefix $(BLD)/,$(SOBJS))

test: all
	./$(IDX) -l ../f.lst -j 3 && ./srch main

clean:
	-rm -rf $(BLD)
	-rm $(IDX)
	-rm $(SRCH)
	-rm *.db*

all: $(IDX) $(SRCH)

install: all
	-cp $(IDX) $(INSTALL_DIR)
	-cp $(SRCH) $(INSTALL_DIR)

$(SRCH): $(BLD) $(EXE_SRCH)
	-cp $(EXE_SRCH) $(SRCH)

$(IDX): $(BLD) $(EXE_IDX)
	-cp $(EXE_IDX) $(IDX)

$(EXE_IDX): $(EXE_IDX).o $(IDX_LIB)
	$(CPP) -o $(EXE_IDX) $< $(LIB_PATH) -lGrokIdx -lpthread -lsqlite3 -lclang -lboost_filesystem -lboost_system

$(EXE_SRCH): $(EXE_SRCH).o $(SRCH_LIB)
	$(CPP) -o $(EXE_SRCH) $< $(LIB_PATH) -lGrokSrch -lpthread -lsqlite3

$(IDX_LIB): $(IDX_OBJS)
	$(AR) rcs $(IDX_LIB) $(IDX_OBJS)

$(SRCH_LIB): $(SRCH_OBJS)
	$(AR) rcs $(SRCH_LIB) $(SRCH_OBJS)

$(BLD):
	-mkdir $(BLD)

$(BLD)/%.o : %.cpp
	$(C_CPP) $(COMPILE)

$(BLD)/%.o : %.c
	$(C_C) $(COMPILE)