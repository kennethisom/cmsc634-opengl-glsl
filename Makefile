# view assignment make file

# GNU make magic to get the right platform-dependent flags for compiling. 
# For example, on linux this will include Defs.Linux, while on mac it
# will include Defs.Darwin
include Defs.$(shell uname)

# files and intermediate files we create
OBJS  = GLdemo.o Input.o Scene.o Terrain.o Marker.o Shader.o ImagePPM.o \
	MatPair.cpp Mat.cpp
PROG  = GLdemo

# set to -O for optimized, -g for debug
OPT = -O

# rules for building -- ordered from final output to original .c for no
# particular reason other than that the first rule is the default

# program from .o files
$(PROG): $(OBJS)
	$(CXX) $(OPT) -o $(PROG) $(OBJS) $(LDFLAGS) $(LDLIBS)

# .o from .c or .cxx
%.o: %.cpp
	$(CXX) $(OPT) -c -o $@ $< $(CXXFLAGS)
%.o: %.c
	$(CXX) $(OPT) -c -o $@ $< $(CFLAGS)


# remove everything but the program
clean:
	rm -f *~ *.o

# remove everything including program
clobber: clean
	rm -f $(PROG)

# any .o from .cpp uses built-in rule
# the following dependencies (generated with 'g++ -MM *.cpp) 
# ensure that the .o files will be regenerated when any source file 
# they depend on changes
GLdemo.o: GLdemo.cpp AppContext.hpp Input.hpp Scene.hpp Vec.hpp \
  MatPair.hpp Mat.hpp Terrain.hpp Shader.hpp Marker.hpp
ImagePPM.o: ImagePPM.cpp ImagePPM.hpp Vec.hpp
Input.o: Input.cpp Input.hpp AppContext.hpp Scene.hpp Vec.hpp MatPair.hpp \
  Mat.hpp Terrain.hpp Shader.hpp Marker.hpp
Marker.o: Marker.cpp Marker.hpp Vec.hpp MatPair.hpp Mat.hpp Shader.hpp \
  AppContext.hpp Vec.inl MatPair.inl Mat.inl
Mat.o: Mat.cpp Mat.inl Mat.hpp Vec.hpp Vec.inl
MatPair.o: MatPair.cpp MatPair.inl MatPair.hpp Mat.hpp Vec.hpp Mat.inl \
  Vec.inl
Scene.o: Scene.cpp Scene.hpp Vec.hpp MatPair.hpp Mat.hpp AppContext.hpp \
  Marker.hpp Shader.hpp MatPair.inl Mat.inl Vec.inl
Shader.o: Shader.cpp Shader.hpp
Terrain.o: Terrain.cpp Terrain.hpp Vec.hpp Shader.hpp AppContext.hpp \
  ImagePPM.hpp Vec.inl
