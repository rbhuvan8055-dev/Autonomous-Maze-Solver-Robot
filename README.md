# Autonomous-Maze-Solver-Robot
Autonomous line-following robot that solves a maze using path recording, optimization algorithms, and EEPROM-based shortest path execution
# Autonomous Maze Solver Robot (Shortest Path + EEPROM)

## Overview
This project implements an autonomous robot capable of solving a maze and computing the shortest path. The robot first explores the maze, records the path, optimizes it, and then executes the shortest path.

## Features
- Dry Run: Explores maze and records path (L, R, S, U)
- Path Optimization Algorithm for shortest path
- EEPROM storage for path persistence
- Final Run: Executes optimized path at higher speed
- PD control for accurate line following

## Hardware Used
- Arduino
- IR Sensors (5 sensor array)
- Motor Driver (L298N or similar)
- DC Motors
- EEPROM (internal)

## Key Concepts
- Path planning and optimization
- Embedded systems programming
- Sensor-based navigation
- PD control algorithm

## Working
1. Robot explores maze and records path
2. Detects dead ends and applies backtracking
3. Optimizes path using reduction rules
4. Stores shortest path in EEPROM
5. Re-runs maze using optimized path

## Outcome
Built a fully autonomous robot capable of solving unknown mazes and executing the shortest path efficiently.

Done by R Bhuvan  follow for more iot stuff :)
