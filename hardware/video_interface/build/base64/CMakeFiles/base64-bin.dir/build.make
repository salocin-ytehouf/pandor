# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

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
CMAKE_COMMAND = /home/nico/pandore/ai/plate_recognition/venv/lib/python3.8/site-packages/cmake/data/bin/cmake

# The command to remove a file.
RM = /home/nico/pandore/ai/plate_recognition/venv/lib/python3.8/site-packages/cmake/data/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/nico/pandore/hardware/video_interface

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nico/pandore/hardware/video_interface/build

# Include any dependencies generated for this target.
include base64/CMakeFiles/base64-bin.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include base64/CMakeFiles/base64-bin.dir/compiler_depend.make

# Include the progress variables for this target.
include base64/CMakeFiles/base64-bin.dir/progress.make

# Include the compile flags for this target's objects.
include base64/CMakeFiles/base64-bin.dir/flags.make

base64/CMakeFiles/base64-bin.dir/bin/base64.c.o: base64/CMakeFiles/base64-bin.dir/flags.make
base64/CMakeFiles/base64-bin.dir/bin/base64.c.o: /home/nico/pandore/hardware/video_interface/base64/bin/base64.c
base64/CMakeFiles/base64-bin.dir/bin/base64.c.o: base64/CMakeFiles/base64-bin.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/nico/pandore/hardware/video_interface/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object base64/CMakeFiles/base64-bin.dir/bin/base64.c.o"
	cd /home/nico/pandore/hardware/video_interface/build/base64 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT base64/CMakeFiles/base64-bin.dir/bin/base64.c.o -MF CMakeFiles/base64-bin.dir/bin/base64.c.o.d -o CMakeFiles/base64-bin.dir/bin/base64.c.o -c /home/nico/pandore/hardware/video_interface/base64/bin/base64.c

base64/CMakeFiles/base64-bin.dir/bin/base64.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/base64-bin.dir/bin/base64.c.i"
	cd /home/nico/pandore/hardware/video_interface/build/base64 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/nico/pandore/hardware/video_interface/base64/bin/base64.c > CMakeFiles/base64-bin.dir/bin/base64.c.i

base64/CMakeFiles/base64-bin.dir/bin/base64.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/base64-bin.dir/bin/base64.c.s"
	cd /home/nico/pandore/hardware/video_interface/build/base64 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/nico/pandore/hardware/video_interface/base64/bin/base64.c -o CMakeFiles/base64-bin.dir/bin/base64.c.s

# Object files for target base64-bin
base64__bin_OBJECTS = \
"CMakeFiles/base64-bin.dir/bin/base64.c.o"

# External object files for target base64-bin
base64__bin_EXTERNAL_OBJECTS =

base64/bin/base64: base64/CMakeFiles/base64-bin.dir/bin/base64.c.o
base64/bin/base64: base64/CMakeFiles/base64-bin.dir/build.make
base64/bin/base64: base64/libbase64.a
base64/bin/base64: base64/CMakeFiles/base64-bin.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/nico/pandore/hardware/video_interface/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable bin/base64"
	cd /home/nico/pandore/hardware/video_interface/build/base64 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/base64-bin.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
base64/CMakeFiles/base64-bin.dir/build: base64/bin/base64
.PHONY : base64/CMakeFiles/base64-bin.dir/build

base64/CMakeFiles/base64-bin.dir/clean:
	cd /home/nico/pandore/hardware/video_interface/build/base64 && $(CMAKE_COMMAND) -P CMakeFiles/base64-bin.dir/cmake_clean.cmake
.PHONY : base64/CMakeFiles/base64-bin.dir/clean

base64/CMakeFiles/base64-bin.dir/depend:
	cd /home/nico/pandore/hardware/video_interface/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nico/pandore/hardware/video_interface /home/nico/pandore/hardware/video_interface/base64 /home/nico/pandore/hardware/video_interface/build /home/nico/pandore/hardware/video_interface/build/base64 /home/nico/pandore/hardware/video_interface/build/base64/CMakeFiles/base64-bin.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : base64/CMakeFiles/base64-bin.dir/depend
