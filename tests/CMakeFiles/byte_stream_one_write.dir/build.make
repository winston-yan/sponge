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
include tests/CMakeFiles/byte_stream_one_write.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tests/CMakeFiles/byte_stream_one_write.dir/compiler_depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/byte_stream_one_write.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/byte_stream_one_write.dir/flags.make

tests/CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o: tests/CMakeFiles/byte_stream_one_write.dir/flags.make
tests/CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o: tests/byte_stream_one_write.cc
tests/CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o: tests/CMakeFiles/byte_stream_one_write.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/projects/cs144/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o"
	cd /root/projects/cs144/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o -MF CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o.d -o CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o -c /root/projects/cs144/tests/byte_stream_one_write.cc

tests/CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.i"
	cd /root/projects/cs144/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/projects/cs144/tests/byte_stream_one_write.cc > CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.i

tests/CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.s"
	cd /root/projects/cs144/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/projects/cs144/tests/byte_stream_one_write.cc -o CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.s

# Object files for target byte_stream_one_write
byte_stream_one_write_OBJECTS = \
"CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o"

# External object files for target byte_stream_one_write
byte_stream_one_write_EXTERNAL_OBJECTS =

tests/byte_stream_one_write: tests/CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o
tests/byte_stream_one_write: tests/CMakeFiles/byte_stream_one_write.dir/build.make
tests/byte_stream_one_write: tests/libspongechecks.a
tests/byte_stream_one_write: libsponge/libsponge.a
tests/byte_stream_one_write: tests/CMakeFiles/byte_stream_one_write.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/projects/cs144/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable byte_stream_one_write"
	cd /root/projects/cs144/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/byte_stream_one_write.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/byte_stream_one_write.dir/build: tests/byte_stream_one_write
.PHONY : tests/CMakeFiles/byte_stream_one_write.dir/build

tests/CMakeFiles/byte_stream_one_write.dir/clean:
	cd /root/projects/cs144/tests && $(CMAKE_COMMAND) -P CMakeFiles/byte_stream_one_write.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/byte_stream_one_write.dir/clean

tests/CMakeFiles/byte_stream_one_write.dir/depend:
	cd /root/projects/cs144 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/projects/cs144 /root/projects/cs144/tests /root/projects/cs144 /root/projects/cs144/tests /root/projects/cs144/tests/CMakeFiles/byte_stream_one_write.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/byte_stream_one_write.dir/depend
