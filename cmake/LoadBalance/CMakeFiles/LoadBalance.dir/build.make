# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_SOURCE_DIR = /home/aditya/quinoa/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/aditya/quinoa/cmake

# Include any dependencies generated for this target.
include LoadBalance/CMakeFiles/LoadBalance.dir/depend.make

# Include the progress variables for this target.
include LoadBalance/CMakeFiles/LoadBalance.dir/progress.make

# Include the compile flags for this target's objects.
include LoadBalance/CMakeFiles/LoadBalance.dir/flags.make

LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o: LoadBalance/CMakeFiles/LoadBalance.dir/flags.make
LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o: /home/aditya/quinoa/src/LoadBalance/CommMap.C
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/aditya/quinoa/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o"
	cd /home/aditya/quinoa/cmake/LoadBalance && /usr/bin/mpicxx   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/LoadBalance.dir/CommMap.C.o -c /home/aditya/quinoa/src/LoadBalance/CommMap.C

LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LoadBalance.dir/CommMap.C.i"
	cd /home/aditya/quinoa/cmake/LoadBalance && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/aditya/quinoa/src/LoadBalance/CommMap.C > CMakeFiles/LoadBalance.dir/CommMap.C.i

LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LoadBalance.dir/CommMap.C.s"
	cd /home/aditya/quinoa/cmake/LoadBalance && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/aditya/quinoa/src/LoadBalance/CommMap.C -o CMakeFiles/LoadBalance.dir/CommMap.C.s

LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o.requires:

.PHONY : LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o.requires

LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o.provides: LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o.requires
	$(MAKE) -f LoadBalance/CMakeFiles/LoadBalance.dir/build.make LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o.provides.build
.PHONY : LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o.provides

LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o.provides.build: LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o


LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o: LoadBalance/CMakeFiles/LoadBalance.dir/flags.make
LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o: /home/aditya/quinoa/src/LoadBalance/LinearMap.C
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/aditya/quinoa/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o"
	cd /home/aditya/quinoa/cmake/LoadBalance && /usr/bin/mpicxx   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/LoadBalance.dir/LinearMap.C.o -c /home/aditya/quinoa/src/LoadBalance/LinearMap.C

LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LoadBalance.dir/LinearMap.C.i"
	cd /home/aditya/quinoa/cmake/LoadBalance && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/aditya/quinoa/src/LoadBalance/LinearMap.C > CMakeFiles/LoadBalance.dir/LinearMap.C.i

LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LoadBalance.dir/LinearMap.C.s"
	cd /home/aditya/quinoa/cmake/LoadBalance && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/aditya/quinoa/src/LoadBalance/LinearMap.C -o CMakeFiles/LoadBalance.dir/LinearMap.C.s

LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o.requires:

.PHONY : LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o.requires

LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o.provides: LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o.requires
	$(MAKE) -f LoadBalance/CMakeFiles/LoadBalance.dir/build.make LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o.provides.build
.PHONY : LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o.provides

LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o.provides.build: LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o


LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o: LoadBalance/CMakeFiles/LoadBalance.dir/flags.make
LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o: /home/aditya/quinoa/src/LoadBalance/UnsMeshMap.C
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/aditya/quinoa/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o"
	cd /home/aditya/quinoa/cmake/LoadBalance && /usr/bin/mpicxx   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o -c /home/aditya/quinoa/src/LoadBalance/UnsMeshMap.C

LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LoadBalance.dir/UnsMeshMap.C.i"
	cd /home/aditya/quinoa/cmake/LoadBalance && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/aditya/quinoa/src/LoadBalance/UnsMeshMap.C > CMakeFiles/LoadBalance.dir/UnsMeshMap.C.i

LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LoadBalance.dir/UnsMeshMap.C.s"
	cd /home/aditya/quinoa/cmake/LoadBalance && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/aditya/quinoa/src/LoadBalance/UnsMeshMap.C -o CMakeFiles/LoadBalance.dir/UnsMeshMap.C.s

LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o.requires:

.PHONY : LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o.requires

LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o.provides: LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o.requires
	$(MAKE) -f LoadBalance/CMakeFiles/LoadBalance.dir/build.make LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o.provides.build
.PHONY : LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o.provides

LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o.provides.build: LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o


# Object files for target LoadBalance
LoadBalance_OBJECTS = \
"CMakeFiles/LoadBalance.dir/CommMap.C.o" \
"CMakeFiles/LoadBalance.dir/LinearMap.C.o" \
"CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o"

# External object files for target LoadBalance
LoadBalance_EXTERNAL_OBJECTS =

LoadBalance/libquinoa_loadbalance.so: LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o
LoadBalance/libquinoa_loadbalance.so: LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o
LoadBalance/libquinoa_loadbalance.so: LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o
LoadBalance/libquinoa_loadbalance.so: LoadBalance/CMakeFiles/LoadBalance.dir/build.make
LoadBalance/libquinoa_loadbalance.so: LoadBalance/CMakeFiles/LoadBalance.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/aditya/quinoa/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX shared library libquinoa_loadbalance.so"
	cd /home/aditya/quinoa/cmake/LoadBalance && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/LoadBalance.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
LoadBalance/CMakeFiles/LoadBalance.dir/build: LoadBalance/libquinoa_loadbalance.so

.PHONY : LoadBalance/CMakeFiles/LoadBalance.dir/build

LoadBalance/CMakeFiles/LoadBalance.dir/requires: LoadBalance/CMakeFiles/LoadBalance.dir/CommMap.C.o.requires
LoadBalance/CMakeFiles/LoadBalance.dir/requires: LoadBalance/CMakeFiles/LoadBalance.dir/LinearMap.C.o.requires
LoadBalance/CMakeFiles/LoadBalance.dir/requires: LoadBalance/CMakeFiles/LoadBalance.dir/UnsMeshMap.C.o.requires

.PHONY : LoadBalance/CMakeFiles/LoadBalance.dir/requires

LoadBalance/CMakeFiles/LoadBalance.dir/clean:
	cd /home/aditya/quinoa/cmake/LoadBalance && $(CMAKE_COMMAND) -P CMakeFiles/LoadBalance.dir/cmake_clean.cmake
.PHONY : LoadBalance/CMakeFiles/LoadBalance.dir/clean

LoadBalance/CMakeFiles/LoadBalance.dir/depend:
	cd /home/aditya/quinoa/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/aditya/quinoa/src /home/aditya/quinoa/src/LoadBalance /home/aditya/quinoa/cmake /home/aditya/quinoa/cmake/LoadBalance /home/aditya/quinoa/cmake/LoadBalance/CMakeFiles/LoadBalance.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : LoadBalance/CMakeFiles/LoadBalance.dir/depend

