#source file search paths
VPATH = ./src 

	# compiler include search paths
INCS = \
	-I../windex/include \
	-I../raven-set \
	-I../await/include

LIBS = \
	-lgdiplus -lgdi32 -lcomdlg32 -lstdc++fs -lws2_32 -lwsock32 
	#-mwindows

static_link_flags = -static-libstdc++ -static-libgcc -static

# folders
ODIR = ./.vscode/obj
RDIR = ../ravencode/pub/src

OPT = -g
CCflags = -std=c++17 $(OPT) -D_mingw_

# sources
OBJ_demo = \
	$(ODIR)/demo.o \
	$(ODIR)/cTCP.o \
	$(ODIR)/cTCPServerMultiClient.o

### COMPILE

$(ODIR)/%.o: %.cpp  
	g++  $(CCflags) \
	-c -o $@ $< $(INCS)

demo: $(OBJ_demo)
	g++ $(OPT) \
	-o bin/demo.exe $^  \
	$(static_link_flags) $(LIBS)

.PHONY: clean
clean:
	del /q .vscode\\obj\\*.o

