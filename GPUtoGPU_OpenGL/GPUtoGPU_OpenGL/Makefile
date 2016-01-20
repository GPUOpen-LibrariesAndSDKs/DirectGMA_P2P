#!make


.SUFFIXES: .cpp .h

SRCDIR = ./
OBJDIR = ./obj
BINDIR = ./bin


CXX = g++
CXXFLAGS = -O2 -I./ -I. -I../../common -I/usr/X11R6/include -I../../external/glew-1.9.0/include -I../../external -DGLEW_STATIC -DLINUX

CC = gcc
CFLAGS = $(CXXFLAGS)

LD_FLAGS = -L/usr/X11R6/lib 

LIBS = -lGLU -lGL -lpthread -lX11 -ldl




####### Output directory

OBJECTS_DIR   = ./obj

####### Files

OBJECTS       = $(OBJDIR)/GLWindow.o \
		$(OBJDIR)/ADLtool.o \
		$(OBJDIR)/glew.o \
		$(OBJDIR)/SyncedBuffer.o \
		$(OBJDIR)/RenderTarget.o \
		$(OBJDIR)/GLTransferBuffers.o \
		$(OBJDIR)/FormatInfo.o \
		$(OBJDIR)/Thread.o \
		$(OBJDIR)/GLSource.o \
		$(OBJDIR)/GLSink.o \
		$(OBJDIR)/LnxMain.o \
		$(OBJDIR)/GLCopyApp.o \
		$(OBJDIR)/FireCube.o \
		$(OBJDIR)/rgbimage.o




TARGET        = $(BINDIR)/GpuToGpu

all: 
	if [ ! -d ./$(OBJDIR) ] ; then mkdir ./$(OBJDIR); fi
	if [ ! -d ./$(BINDIR) ] ; then mkdir ./$(BINDIR); fi
	make $(TARGET)
	cp -f ../resources/AMD_FirePro.rgb $(BINDIR)


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

$(OBJDIR)/RenderTarget.o: ../../common/RenderTarget.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ ../../common/RenderTarget.cpp
	
$(OBJDIR)/FormatInfo.o: ../../common/FormatInfo.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ ../../common/FormatInfo.cpp

$(OBJDIR)/Thread.o: ../../common/Thread.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ ../../common/Thread.cpp

$(OBJDIR)/GLTransferBuffers.o: GLTransferBuffers.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ GLTransferBuffers.cpp

$(OBJDIR)/GLSource.o: GLSource.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ GLSource.cpp

$(OBJDIR)/GLSink.o: GLSink.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ GLSink.cpp

$(OBJDIR)/LnxMain.o: LnxMain.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ LnxMain.cpp

$(OBJDIR)/GLCopyApp.o: GLCopyApp.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ GLCopyApp.cpp

$(OBJDIR)/FireCube.o: FireCube.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ FireCube.cpp

$(OBJDIR)/rgbimage.o: rgbimage.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ rgbimage.cpp

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(OBJDIR)/$(TARGET)
	rm -f $(TARGET)
	
