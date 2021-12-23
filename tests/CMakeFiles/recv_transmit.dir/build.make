# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/projects/cs144

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/projects/cs144

# Include any dependencies generated for this target.
include tests/CMakeFiles/recv_transmit.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tests/CMakeFiles/recv_transmit.dir/compiler_depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/recv_transmit.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/recv_transmit.dir/flags.make

tests/CMakeFiles/recv_transmit.dir/recv_transmit.cc.o: tests/CMakeFiles/recv_transmit.dir/flags.make
tests/CMakeFiles/recv_transmit.dir/recv_transmit.cc.o: tests/recv_transmit.cc
tests/CMakeFiles/recv_transmit.dir/recv_transmit.cc.o: tests/CMakeFiles/recv_transmit.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/projects/cs144/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/recv_transmit.dir/recv_transmit.cc.o"
	cd /root/projects/cs144/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/recv_transmit.dir/recv_transmit.cc.o -MF CMakeFiles/recv_transmit.dir/recv_transmit.cc.o.d -o CMakeFiles/recv_transmit.dir/recv_transmit.cc.o -c /root/projects/cs144/tests/recv_transmit.cc

tests/CMakeFiles/recv_transmit.dir/recv_transmit.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/recv_transmit.dir/recv_transmit.cc.i"
	cd /root/projects/cs144/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/projects/cs144/tests/recv_transmit.cc > CMakeFiles/recv_transmit.dir/recv_transmit.cc.i

tests/CMakeFiles/recv_transmit.dir/recv_transmit.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/recv_transmit.dir/recv_transmit.cc.s"
	cd /root/projects/cs144/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/projects/cs144/tests/recv_transmit.cc -o CMakeFiles/recv_transmit.dir/recv_transmit.cc.s

# Object files for target recv_transmit
recv_transmit_OBJECTS = \
"CMakeFiles/recv_transmit.dir/recv_transmit.cc.o"

# External object files for target recv_transmit
recv_transmit_EXTERNAL_OBJECTS =

tests/recv_transmit: tests/CMakeFiles/recv_transmit.dir/recv_transmit.cc.o
tests/recv_transmit: tests/CMakeFiles/recv_transmit.dir/build.make
tests/recv_transmit: tests/libspongechecks.a
tests/recv_transmit: libsponge/libsponge.a
tests/recv_transmit: tests/CMakeFiles/recv_transmit.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/projects/cs144/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable recv_transmit"
	cd /root/projects/cs144/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/recv_transmit.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/recv_transmit.dir/build: tests/recv_transmit
.PHONY : tests/CMakeFiles/recv_transmit.dir/build

tests/CMakeFiles/recv_transmit.dir/clean:
	cd /root/projects/cs144/tests && $(CMAKE_COMMAND) -P CMakeFiles/recv_transmit.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/recv_transmit.dir/clean

tests/CMakeFiles/recv_transmit.dir/depend:
	cd /root/projects/cs144 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/projects/cs144 /root/projects/cs144/tests /root/projects/cs144 /root/projects/cs144/tests /root/projects/cs144/tests/CMakeFiles/recv_transmit.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/recv_transmit.dir/depend

