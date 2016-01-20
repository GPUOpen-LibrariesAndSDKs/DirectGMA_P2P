#!make


.SUFFIXES: .cpp .h

SRCDIR = ./
OBJDIR = ./obj
BINDIR = ./bin


CXX = g++
CXXFLAGS = -O2 -I./ -I. -I../../common -I/usr/X11R6/include -I../../external/glew-1.9.0/include -I/opt/AMDAPPSDK-2.9-1/include -I../../external -DGLEW_STATIC -DLINUX

CC = gcc
CFLAGS = $(CXXFLAGS)

LD_FLAGS = -L/usr/X11R6/lib -L/opt/AMDAPPSDK-2.9-1/lib/x86_64

LIBS = -lGLU -lGL -lpthread -lX11 -ldl -lOpenCL




####### Output directory

OBJECTS_DIR   = ./obj

####### Files


OBJECTS       = $(OBJDIR)/GLWindow.o 		\
		$(OBJDIR)/ADLtool.o 		\
		$(OBJDIR)/glew.o 		\
		$(OBJDIR)/SyncedBuffer.o 	\
		$(OBJDIR)/Thread.o 		\
		$(OBJDIR)/CLTransferBuffer.o 	\
		$(OBJDIR)/CLSource.o 		\
		$(OBJDIR)/CLSink.o 		\
		$(OBJDIR)/LnxMain.o 		\
		$(OBJDIR)/CLCopyApp.o


TARGET        = $(BINDIR)/GpuToGpuCL

all: 
	if [ ! -d ./$(OBJDIR) ] ; then mkdir ./$(OBJDIR); fi
	if [ ! -d ./$(BINDIR) ] ; then mkdir ./$(BINDIR); fi
	make $(TARGET)
	

$(TARGET): $(OBJECTS)
	$(CXX) -o $@  $(LD_FLAGS) $(OBJECTS) $(LIBS)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<



$(OBJDIR)/GLWindow.o: ../../common/GLWindow.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ ../../common/GLWindow.cpp

$(OBJDIR)/ADLtool.o: ../../common/ADLtool.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ ../../common/ADLtool.cpp

$(OBJDIR)/glew.o: ../../external/glew-1.9.0/src/glew.c 
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ ../../external/glew-1.9.0/src/glew.c

$(OBJDIR)/SyncedBuffer.o: ../../common/BufferQueue.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ ../../common/BufferQueue.cpp

$(OBJDIR)/Thread.o: ../../common/Thread.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ ../../common/Thread.cpp

$(OBJDIR)/CLTransferBuffer.o: CLTransferBuffer.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ CLTransferBuffer.cpp
	
$(OBJDIR)/CLSource.o: CLSource.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ CLSource.cpp

$(OBJDIR)/CLSink.o: CLSink.cpp .
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ CLSink.cpp
	
$(OBJDIR)/CLCopyApp.o: CLCopyApp.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ CLCopyApp.cpp


$(OBJDIR)/LnxMain.o: LnxMain.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ LnxMain.cpp


clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(OBJDIR)/$(TARGET)
	rm -f $(TARGET)
	
