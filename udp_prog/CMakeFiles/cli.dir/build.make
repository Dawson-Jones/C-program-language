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
CMAKE_SOURCE_DIR = /home/bytedance/test/udp_prog

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/bytedance/test/udp_prog

# Include any dependencies generated for this target.
include CMakeFiles/cli.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/cli.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/cli.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cli.dir/flags.make

CMakeFiles/cli.dir/cli.c.o: CMakeFiles/cli.dir/flags.make
CMakeFiles/cli.dir/cli.c.o: cli.c
CMakeFiles/cli.dir/cli.c.o: CMakeFiles/cli.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bytedance/test/udp_prog/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/cli.dir/cli.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/cli.dir/cli.c.o -MF CMakeFiles/cli.dir/cli.c.o.d -o CMakeFiles/cli.dir/cli.c.o -c /home/bytedance/test/udp_prog/cli.c

CMakeFiles/cli.dir/cli.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cli.dir/cli.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/bytedance/test/udp_prog/cli.c > CMakeFiles/cli.dir/cli.c.i

CMakeFiles/cli.dir/cli.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cli.dir/cli.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/bytedance/test/udp_prog/cli.c -o CMakeFiles/cli.dir/cli.c.s

# Object files for target cli
cli_OBJECTS = \
"CMakeFiles/cli.dir/cli.c.o"

# External object files for target cli
cli_EXTERNAL_OBJECTS =

cli: CMakeFiles/cli.dir/cli.c.o
cli: CMakeFiles/cli.dir/build.make
cli: libmytbf/libmytbf.a
cli: CMakeFiles/cli.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bytedance/test/udp_prog/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable cli"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cli.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/cli.dir/build: cli
.PHONY : CMakeFiles/cli.dir/build

CMakeFiles/cli.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/cli.dir/cmake_clean.cmake
.PHONY : CMakeFiles/cli.dir/clean

CMakeFiles/cli.dir/depend:
	cd /home/bytedance/test/udp_prog && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bytedance/test/udp_prog /home/bytedance/test/udp_prog /home/bytedance/test/udp_prog /home/bytedance/test/udp_prog /home/bytedance/test/udp_prog/CMakeFiles/cli.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/cli.dir/depend

