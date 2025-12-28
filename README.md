# Agent Smith

<p align="center">
  <img 
    src="/assets/repo_images/logo.jpg" 
    width="200" 
    height="300"
  />
</p>

## Overview
Agent Smith is a chess engine whose ELO rating is about 1400. It uses the UCI protocol, meaning that it can analyze moves from any given position and be installed in almost any modern Chess GUI. Chess "states" (i.e. piece locations, attacked squares, etc) are all represented in unsigned 64 bit integers, or "bitboards," with each set bit representing a square. For example, you may have a bitboard whose set bits represent piece locations, attack rays, etc. Bitboard representation allows for extremely fast move generation, as most algorithms consist entirely of bitwise operations. 

## 30 Second Installation 

1. Go to releases and download the latest version of Agent Smith. 
2. Extract the zip folder to any location. The directory should contain agent_smith.exe and agent_smith_profiling.exe. Note that to use the profiler app, you must have python installed. 
3. (Optional). Set the CHESS_ASSET_DIR environment variable to the "assets" subdirectory in order to specify the directory in which profiling sessions and bitboard images will be stored. 
## Usage 

### Installing Agent Smith in a Chess GUI
1. Open your Chess GUI of choice. 
2. Go to the engine management section and add a new UCI engine.
3. Select either the agent_smith.exe or agent_smith_profiling.exe file from the extracted folder.

### Command Line Usage

Run ./agent_smith help to see a list of all available commands. Note that to create a bitboard image, you must set the CHESS_ASSET_DIR environment variable as specified in step 3 of installation. 

### Profiling

Agent Orange comes with a custom profiling app built off of Python's Tkinter library. 
1. Set the CHESS_PROFILING_SESSIONS environment variable to specify the directory in which profiling session files will be stored.
2. Run agent_smith_profiling.exe. After the binary finishes running, a new profiling session file should appear in the specified directory. 
3. Profiling sessions can be visualized inside the custom profiler app. To run this app, first create a custom Python environment in the profiler_visualizer directory. 
4. Run /.venv/Scripts/Activate. Finally, run "python main.py" from the profiler_visualizer directory.

## Move Search Strategy
Agent Smith uses alpha-beta pruning and the principal variation (PV) algorithm. Upon calculating each legal move in a position, Agent Smith will order them in a way that allows maximum pruning. PV moves are ordered first, followed by captures and evasion moves sorted by the material gained/saved. 

## Position Evaluation
Agent Smith uses a variety of heuristics to analyze a given position:
1. Material
2. Pawn advancement
3. Pawn island count
4. King tropism
5. Square control, with squares near the king weighted higher
6. Attacked piece count
7. Castling ability

## Profiling
Agent Smith uses a custom profiling app built off Python's Tkinter library. You can look at individual function benchmarks in a bar graph visualization, or at how much relative time they are taking in the engine's evaluation. 

<p align="center">
  <img src="assets/repo_images/profiling.png" alt="Logo" width="1250">
</p>
