#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.147                       #
#------------------------------------------------------------------------------#


WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = -Ideps
CFLAGS = -Wall -fexceptions -std=c++17 -Wno-narrowing -DNET_CLIENT -DNO_COMPRESSION
RESINC = 
LIBDIR = 
LIB = C:/Users/James/Desktop/projects/binmat_game/libduktape.so.202.20200
LDFLAGS = -lmingw32 -limgui -lsfml-graphics -lsfml-audio -lsfml-network -lsfml-window -lsfml-system -lfreetype -lopengl32 -lws2_32 -lflac -lopenal32 -logg

INC_DEBUG = $(INC)
CFLAGS_DEBUG = $(CFLAGS) -g
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = obj/Debug
DEP_DEBUG = 
OUT_DEBUG = bin/Debug/hide_and_destroy

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -O2
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = bin/Release/hide_and_destroy

INC_PROFILE = $(INC)
CFLAGS_PROFILE = $(CFLAGS) -pg -Og
RESINC_PROFILE = $(RESINC)
RCFLAGS_PROFILE = $(RCFLAGS)
LIBDIR_PROFILE = $(LIBDIR)
LIB_PROFILE = $(LIB)
LDFLAGS_PROFILE = $(LDFLAGS) -pg -lgmon
OBJDIR_PROFILE = obj/Profile
DEP_PROFILE = 
OUT_PROFILE = bin/Profile/hide_and_destroy

INC_ARM_NATIVE = $(INC)
CFLAGS_ARM_NATIVE = $(CFLAGS) -O2
RESINC_ARM_NATIVE = $(RESINC)
RCFLAGS_ARM_NATIVE = $(RCFLAGS)
LIBDIR_ARM_NATIVE = $(LIBDIR)
LIB_ARM_NATIVE = $(LIB)
LDFLAGS_ARM_NATIVE = $(LDFLAGS) -s
OBJDIR_ARM_NATIVE = obj/Release
DEP_ARM_NATIVE = 
OUT_ARM_NATIVE = bin/Release/hide_and_destroy

OBJ_DEBUG = $(OBJDIR_DEBUG)/deps/4space_server/networking.o $(OBJDIR_DEBUG)/deps/serialise/serialise.o $(OBJDIR_DEBUG)/main.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/deps/4space_server/networking.o $(OBJDIR_RELEASE)/deps/serialise/serialise.o $(OBJDIR_RELEASE)/main.o

OBJ_PROFILE = $(OBJDIR_PROFILE)/deps/4space_server/networking.o $(OBJDIR_PROFILE)/deps/serialise/serialise.o $(OBJDIR_PROFILE)/main.o

OBJ_ARM_NATIVE = $(OBJDIR_ARM_NATIVE)/deps/4space_server/networking.o $(OBJDIR_ARM_NATIVE)/deps/serialise/serialise.o $(OBJDIR_ARM_NATIVE)/main.o

all: before_build build_debug build_release build_profile build_arm_release build_arm_native after_build

clean: clean_debug clean_release clean_profile clean_arm_release clean_arm_native

before_build: 
	update_submodules.bat

after_build: 
	post_build.bat

before_debug: 
	test -d bin/Debug || mkdir -p bin/Debug
	test -d $(OBJDIR_DEBUG)/deps/4space_server || mkdir -p $(OBJDIR_DEBUG)/deps/4space_server
	test -d $(OBJDIR_DEBUG)/deps/serialise || mkdir -p $(OBJDIR_DEBUG)/deps/serialise
	test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_debug: 

build_debug: before_debug out_debug after_debug

debug: before_build build_debug after_build

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LIBDIR_DEBUG) -o $(OUT_DEBUG) $(OBJ_DEBUG)  $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/deps/4space_server/networking.o: deps/4space_server/networking.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c deps/4space_server/networking.cpp -o $(OBJDIR_DEBUG)/deps/4space_server/networking.o

$(OBJDIR_DEBUG)/deps/serialise/serialise.o: deps/serialise/serialise.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c deps/serialise/serialise.cpp -o $(OBJDIR_DEBUG)/deps/serialise/serialise.o

$(OBJDIR_DEBUG)/main.o: main.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c main.cpp -o $(OBJDIR_DEBUG)/main.o

clean_debug: 
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf bin/Debug
	rm -rf $(OBJDIR_DEBUG)/deps/4space_server
	rm -rf $(OBJDIR_DEBUG)/deps/serialise
	rm -rf $(OBJDIR_DEBUG)

before_release: 
	test -d bin/Release || mkdir -p bin/Release
	test -d $(OBJDIR_RELEASE)/deps/4space_server || mkdir -p $(OBJDIR_RELEASE)/deps/4space_server
	test -d $(OBJDIR_RELEASE)/deps/serialise || mkdir -p $(OBJDIR_RELEASE)/deps/serialise
	test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)

after_release: 

build_release: before_release out_release after_release

release: before_build build_release after_build

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LIBDIR_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE)  $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/deps/4space_server/networking.o: deps/4space_server/networking.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c deps/4space_server/networking.cpp -o $(OBJDIR_RELEASE)/deps/4space_server/networking.o

$(OBJDIR_RELEASE)/deps/serialise/serialise.o: deps/serialise/serialise.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c deps/serialise/serialise.cpp -o $(OBJDIR_RELEASE)/deps/serialise/serialise.o

$(OBJDIR_RELEASE)/main.o: main.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c main.cpp -o $(OBJDIR_RELEASE)/main.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf bin/Release
	rm -rf $(OBJDIR_RELEASE)/deps/4space_server
	rm -rf $(OBJDIR_RELEASE)/deps/serialise
	rm -rf $(OBJDIR_RELEASE)

before_profile: 
	test -d bin/Profile || mkdir -p bin/Profile
	test -d $(OBJDIR_PROFILE)/deps/4space_server || mkdir -p $(OBJDIR_PROFILE)/deps/4space_server
	test -d $(OBJDIR_PROFILE)/deps/serialise || mkdir -p $(OBJDIR_PROFILE)/deps/serialise
	test -d $(OBJDIR_PROFILE) || mkdir -p $(OBJDIR_PROFILE)

after_profile: 

build_profile: before_profile out_profile after_profile

profile: before_build build_profile after_build

out_profile: before_profile $(OBJ_PROFILE) $(DEP_PROFILE)
	$(LD) $(LIBDIR_PROFILE) -o $(OUT_PROFILE) $(OBJ_PROFILE)  $(LDFLAGS_PROFILE) $(LIB_PROFILE)

$(OBJDIR_PROFILE)/deps/4space_server/networking.o: deps/4space_server/networking.cpp
	$(CXX) $(CFLAGS_PROFILE) $(INC_PROFILE) -c deps/4space_server/networking.cpp -o $(OBJDIR_PROFILE)/deps/4space_server/networking.o

$(OBJDIR_PROFILE)/deps/serialise/serialise.o: deps/serialise/serialise.cpp
	$(CXX) $(CFLAGS_PROFILE) $(INC_PROFILE) -c deps/serialise/serialise.cpp -o $(OBJDIR_PROFILE)/deps/serialise/serialise.o

$(OBJDIR_PROFILE)/main.o: main.cpp
	$(CXX) $(CFLAGS_PROFILE) $(INC_PROFILE) -c main.cpp -o $(OBJDIR_PROFILE)/main.o

clean_profile: 
	rm -f $(OBJ_PROFILE) $(OUT_PROFILE)
	rm -rf bin/Profile
	rm -rf $(OBJDIR_PROFILE)/deps/4space_server
	rm -rf $(OBJDIR_PROFILE)/deps/serialise
	rm -rf $(OBJDIR_PROFILE)

before_arm_native: 
	test -d bin/Release || mkdir -p bin/Release
	test -d $(OBJDIR_ARM_NATIVE)/deps/4space_server || mkdir -p $(OBJDIR_ARM_NATIVE)/deps/4space_server
	test -d $(OBJDIR_ARM_NATIVE)/deps/serialise || mkdir -p $(OBJDIR_ARM_NATIVE)/deps/serialise
	test -d $(OBJDIR_ARM_NATIVE) || mkdir -p $(OBJDIR_ARM_NATIVE)

after_arm_native: 

build_arm_native: before_arm_native out_arm_native after_arm_native

arm_native: before_build build_arm_native after_build

out_arm_native: before_arm_native $(OBJ_ARM_NATIVE) $(DEP_ARM_NATIVE)
	$(LD) $(LIBDIR_ARM_NATIVE) -o $(OUT_ARM_NATIVE) $(OBJ_ARM_NATIVE)  $(LDFLAGS_ARM_NATIVE) $(LIB_ARM_NATIVE)

$(OBJDIR_ARM_NATIVE)/deps/4space_server/networking.o: deps/4space_server/networking.cpp
	$(CXX) $(CFLAGS_ARM_NATIVE) $(INC_ARM_NATIVE) -c deps/4space_server/networking.cpp -o $(OBJDIR_ARM_NATIVE)/deps/4space_server/networking.o

$(OBJDIR_ARM_NATIVE)/deps/serialise/serialise.o: deps/serialise/serialise.cpp
	$(CXX) $(CFLAGS_ARM_NATIVE) $(INC_ARM_NATIVE) -c deps/serialise/serialise.cpp -o $(OBJDIR_ARM_NATIVE)/deps/serialise/serialise.o

$(OBJDIR_ARM_NATIVE)/main.o: main.cpp
	$(CXX) $(CFLAGS_ARM_NATIVE) $(INC_ARM_NATIVE) -c main.cpp -o $(OBJDIR_ARM_NATIVE)/main.o

clean_arm_native: 
	rm -f $(OBJ_ARM_NATIVE) $(OUT_ARM_NATIVE)
	rm -rf bin/Release
	rm -rf $(OBJDIR_ARM_NATIVE)/deps/4space_server
	rm -rf $(OBJDIR_ARM_NATIVE)/deps/serialise
	rm -rf $(OBJDIR_ARM_NATIVE)

.PHONY: before_build after_build before_debug after_debug clean_debug before_release after_release clean_release before_profile after_profile clean_profile before_arm_native after_arm_native clean_arm_native

