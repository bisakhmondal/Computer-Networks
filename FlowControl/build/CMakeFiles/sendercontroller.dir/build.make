# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/build

# Include any dependencies generated for this target.
include CMakeFiles/sendercontroller.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/sendercontroller.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sendercontroller.dir/flags.make

CMakeFiles/sendercontroller.dir/sendercontroller.cpp.o: CMakeFiles/sendercontroller.dir/flags.make
CMakeFiles/sendercontroller.dir/sendercontroller.cpp.o: ../sendercontroller.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/sendercontroller.dir/sendercontroller.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sendercontroller.dir/sendercontroller.cpp.o -c /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/sendercontroller.cpp

CMakeFiles/sendercontroller.dir/sendercontroller.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sendercontroller.dir/sendercontroller.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/sendercontroller.cpp > CMakeFiles/sendercontroller.dir/sendercontroller.cpp.i

CMakeFiles/sendercontroller.dir/sendercontroller.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sendercontroller.dir/sendercontroller.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/sendercontroller.cpp -o CMakeFiles/sendercontroller.dir/sendercontroller.cpp.s

# Object files for target sendercontroller
sendercontroller_OBJECTS = \
"CMakeFiles/sendercontroller.dir/sendercontroller.cpp.o"

# External object files for target sendercontroller
sendercontroller_EXTERNAL_OBJECTS =

sendercontroller: CMakeFiles/sendercontroller.dir/sendercontroller.cpp.o
sendercontroller: CMakeFiles/sendercontroller.dir/build.make
sendercontroller: ../include/library.so
sendercontroller: CMakeFiles/sendercontroller.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable sendercontroller"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sendercontroller.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sendercontroller.dir/build: sendercontroller

.PHONY : CMakeFiles/sendercontroller.dir/build

CMakeFiles/sendercontroller.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sendercontroller.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sendercontroller.dir/clean

CMakeFiles/sendercontroller.dir/depend:
	cd /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/build /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/build /home/bisakh/Desktop/bcse/Computer-Networks/FlowControl/build/CMakeFiles/sendercontroller.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/sendercontroller.dir/depend
