CPP=g++
CC=gcc

SRC=src
INC=inc $(SRC)
INCS=$(addprefix -I,$(INC))

OPT_OR_DBG=-g

C_C=$(CC) -Wall -x c++ $(gDEFs) $(INCS)
C_CPP=$(CC) -Wall -std=c++11 -x c++ $(gDEFs) $(INCS)
COMPILE=-c $< -o $@ $(OPT_OR_DBG)

vpath %.c $(SRC)
vpath %.cpp $(SRC)
vpath %.h %(INC)

APP=si
EXE=$(APP)_e
BLD=out

OBJS=main.o \
     FileProcessor.o \
     utils.o \
     FileList.o

BLD_OBJS=$(addprefix $(BLD)/,$(OBJS))

test: all
	./$(APP) files.lst

clean:
	-rm -rf $(BLD)
	-rm $(APP)

all: $(APP)

$(APP): $(BLD) $(EXE)
	-cp $(BLD)/$(EXE) $(APP)

$(EXE): $(BLD_OBJS)
	$(CPP) -o $(BLD)/$(EXE) $^ -lpthread -lsqlite3

$(BLD):
	-mkdir $(BLD)

$(BLD)/%.o : %.cpp
	$(C_CPP) $(COMPILE)

$(BLD)/%.o : %.c
	$(C_C) $(COMPILE)