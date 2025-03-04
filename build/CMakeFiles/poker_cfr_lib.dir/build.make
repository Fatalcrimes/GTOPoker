# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

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
CMAKE_COMMAND = /opt/homebrew/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/maxim/Desktop/GTOPoker

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/maxim/Desktop/GTOPoker/build

# Include any dependencies generated for this target.
include CMakeFiles/poker_cfr_lib.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/poker_cfr_lib.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/poker_cfr_lib.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/poker_cfr_lib.dir/flags.make

CMakeFiles/poker_cfr_lib.dir/codegen:
.PHONY : CMakeFiles/poker_cfr_lib.dir/codegen

CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.o: /Users/maxim/Desktop/GTOPoker/src/abstraction/BetAbstraction.cpp
CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/abstraction/BetAbstraction.cpp

CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/abstraction/BetAbstraction.cpp > CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/abstraction/BetAbstraction.cpp -o CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.s

CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.o: /Users/maxim/Desktop/GTOPoker/src/abstraction/HandAbstraction.cpp
CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/abstraction/HandAbstraction.cpp

CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/abstraction/HandAbstraction.cpp > CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/abstraction/HandAbstraction.cpp -o CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.s

CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.o: /Users/maxim/Desktop/GTOPoker/src/cfr/CFRSolver.cpp
CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/cfr/CFRSolver.cpp

CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/cfr/CFRSolver.cpp > CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/cfr/CFRSolver.cpp -o CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.s

CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.o: /Users/maxim/Desktop/GTOPoker/src/cfr/RegretTable.cpp
CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/cfr/RegretTable.cpp

CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/cfr/RegretTable.cpp > CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/cfr/RegretTable.cpp -o CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.s

CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.o: /Users/maxim/Desktop/GTOPoker/src/cfr/StrategyTable.cpp
CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/cfr/StrategyTable.cpp

CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/cfr/StrategyTable.cpp > CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/cfr/StrategyTable.cpp -o CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.s

CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.o: /Users/maxim/Desktop/GTOPoker/src/game/Action.cpp
CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/game/Action.cpp

CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/game/Action.cpp > CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/game/Action.cpp -o CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.s

CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.o: /Users/maxim/Desktop/GTOPoker/src/game/GameState.cpp
CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/game/GameState.cpp

CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/game/GameState.cpp > CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/game/GameState.cpp -o CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.s

CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.o: /Users/maxim/Desktop/GTOPoker/src/game/PokerDefs.cpp
CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/game/PokerDefs.cpp

CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/game/PokerDefs.cpp > CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/game/PokerDefs.cpp -o CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.s

CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.o: /Users/maxim/Desktop/GTOPoker/src/utils/Logger.cpp
CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/utils/Logger.cpp

CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/utils/Logger.cpp > CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/utils/Logger.cpp -o CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.s

CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.o: /Users/maxim/Desktop/GTOPoker/src/utils/Random.cpp
CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/utils/Random.cpp

CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/utils/Random.cpp > CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/utils/Random.cpp -o CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.s

CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.o: CMakeFiles/poker_cfr_lib.dir/flags.make
CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.o: /Users/maxim/Desktop/GTOPoker/src/utils/Serialization.cpp
CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.o: CMakeFiles/poker_cfr_lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.o -MF CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.o.d -o CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.o -c /Users/maxim/Desktop/GTOPoker/src/utils/Serialization.cpp

CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/maxim/Desktop/GTOPoker/src/utils/Serialization.cpp > CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.i

CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/maxim/Desktop/GTOPoker/src/utils/Serialization.cpp -o CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.s

# Object files for target poker_cfr_lib
poker_cfr_lib_OBJECTS = \
"CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.o" \
"CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.o" \
"CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.o" \
"CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.o" \
"CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.o" \
"CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.o" \
"CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.o" \
"CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.o" \
"CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.o" \
"CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.o" \
"CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.o"

# External object files for target poker_cfr_lib
poker_cfr_lib_EXTERNAL_OBJECTS =

libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/abstraction/BetAbstraction.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/abstraction/HandAbstraction.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/cfr/CFRSolver.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/cfr/RegretTable.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/cfr/StrategyTable.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/game/Action.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/game/GameState.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/game/PokerDefs.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/utils/Logger.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/utils/Random.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/src/utils/Serialization.cpp.o
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/build.make
libpoker_cfr_lib.a: CMakeFiles/poker_cfr_lib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/maxim/Desktop/GTOPoker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Linking CXX static library libpoker_cfr_lib.a"
	$(CMAKE_COMMAND) -P CMakeFiles/poker_cfr_lib.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/poker_cfr_lib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/poker_cfr_lib.dir/build: libpoker_cfr_lib.a
.PHONY : CMakeFiles/poker_cfr_lib.dir/build

CMakeFiles/poker_cfr_lib.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/poker_cfr_lib.dir/cmake_clean.cmake
.PHONY : CMakeFiles/poker_cfr_lib.dir/clean

CMakeFiles/poker_cfr_lib.dir/depend:
	cd /Users/maxim/Desktop/GTOPoker/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/maxim/Desktop/GTOPoker /Users/maxim/Desktop/GTOPoker /Users/maxim/Desktop/GTOPoker/build /Users/maxim/Desktop/GTOPoker/build /Users/maxim/Desktop/GTOPoker/build/CMakeFiles/poker_cfr_lib.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/poker_cfr_lib.dir/depend

