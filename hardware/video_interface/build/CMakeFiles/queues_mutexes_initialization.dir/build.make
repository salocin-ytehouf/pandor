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
CMAKE_SOURCE_DIR = /home/nico/pandore/hardware/video_interface

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nico/pandore/hardware/video_interface/build

# Include any dependencies generated for this target.
include CMakeFiles/queues_mutexes_initialization.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/queues_mutexes_initialization.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/queues_mutexes_initialization.dir/flags.make

CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.o: CMakeFiles/queues_mutexes_initialization.dir/flags.make
CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.o: ../queues_mutexes_initialization.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nico/pandore/hardware/video_interface/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.o -c /home/nico/pandore/hardware/video_interface/queues_mutexes_initialization.cpp

CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/nico/pandore/hardware/video_interface/queues_mutexes_initialization.cpp > CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.i

CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/nico/pandore/hardware/video_interface/queues_mutexes_initialization.cpp -o CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.s

# Object files for target queues_mutexes_initialization
queues_mutexes_initialization_OBJECTS = \
"CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.o"

# External object files for target queues_mutexes_initialization
queues_mutexes_initialization_EXTERNAL_OBJECTS =

queues_mutexes_initialization: CMakeFiles/queues_mutexes_initialization.dir/queues_mutexes_initialization.cpp.o
queues_mutexes_initialization: CMakeFiles/queues_mutexes_initialization.dir/build.make
queues_mutexes_initialization: /usr/local/lib/libopencv_gapi.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_highgui.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_ml.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_objdetect.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_photo.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_stitching.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_video.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_videoio.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_imgcodecs.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_dnn.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_calib3d.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_features2d.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_flann.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_imgproc.so.4.8.0
queues_mutexes_initialization: /usr/local/lib/libopencv_core.so.4.8.0
queues_mutexes_initialization: CMakeFiles/queues_mutexes_initialization.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/nico/pandore/hardware/video_interface/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable queues_mutexes_initialization"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/queues_mutexes_initialization.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/queues_mutexes_initialization.dir/build: queues_mutexes_initialization

.PHONY : CMakeFiles/queues_mutexes_initialization.dir/build

CMakeFiles/queues_mutexes_initialization.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/queues_mutexes_initialization.dir/cmake_clean.cmake
.PHONY : CMakeFiles/queues_mutexes_initialization.dir/clean

CMakeFiles/queues_mutexes_initialization.dir/depend:
	cd /home/nico/pandore/hardware/video_interface/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nico/pandore/hardware/video_interface /home/nico/pandore/hardware/video_interface /home/nico/pandore/hardware/video_interface/build /home/nico/pandore/hardware/video_interface/build /home/nico/pandore/hardware/video_interface/build/CMakeFiles/queues_mutexes_initialization.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/queues_mutexes_initialization.dir/depend

