# WallerGlobal - Global Wall-Following Robot Evolution

## Overview
WallerGlobal is a genetic algorithm-based simulation for evolving robot wall-following behavior in a continuous environment. The robot must learn to follow walls while maintaining optimal distance and completing circuits efficiently.

## Technical Specifications

### Environment
- 200x200 grid world
- Continuous movement and positioning
- Global perception of walls
- Real-valued position tracking

### Robot Capabilities
- Continuous motion (forward/backward)
- 30-degree angular turns
- Distance sensing to walls
- Real position tracking using trigonometry

### Genetic Algorithm Parameters
- Population size: 500 individuals
- Generations: 11
- Selection ratio: 70% crossover, 30% reproduction
- Maximum individual complexity: 1000 nodes
- 2 evaluation runs per individual
- 5000 execution steps per run

### Available Functions
- `PROGN3`: Executes three branches sequentially
- `PROGN2`: Executes two branches sequentially  
- `IFWALL`: Conditional wall detection branching

### Terminal Operations
- `WALKFRONT`: Move forward
- `WALKBACK`: Move backward
- `RIGHT`: Turn right 30 degrees
- `LEFT`: Turn left 30 degrees

### Fitness Function
Calculated as: `(50 * FIT) - (10 * UNFIT)`
- FIT: Successful wall-following points
- UNFIT: Penalties for deviating from optimal path

## Project Comparison
This project is part of a series of three wall-following robot evolution simulations:

1. WallerGlobal (this project): Uses continuous movement and global perception
2. WallerLocal: Focuses on ball-following with local perception and obstacles
3. WallerRestricted: Implements discrete movement in a smaller environment

Each variation explores different aspects of evolutionary robotics and navigation strategies.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Disclaimer

This software is provided "as is", without warranty of any kind. Use at your own risk.
