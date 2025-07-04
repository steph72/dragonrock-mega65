# DragonRock

A CRPG for the MEGA65 computer.

## Overview

DragonRock is an immersive fantasy role-playing game developed specifically for the MEGA65 computer. Taking inspiration from classic CRPGs of the 8-bit era, DragonRock offers adventurers a world filled with cities to explore, dungeons to conquer, and monsters to battle.

## System Requirements

- MEGA65 computer or compatible emulator
- Storage device for game data (SD card recommended)


## Building From Source

### Requirements

- CC65 Compiler Suite
- Python 3.6 or later
- Git
- LaTeX (for building the game manual)

### Setup

1. clone the project: 
```
git clone https://github.com/steph72/dragonrock-mega65.git
```

2. install the project submodules: 
```
git submodule update --init
```

3. setup the python environment & requirements. Don't worry – there's a script for that ;-) 
```
./setupPythonEnvironment.sh
```

That's it, you're ready to go

### Building

DragonRock uses the scons build system because it is so very much nicer and more flexible than Makefiles. Scons is installed alongside with the python environment. 

#### Compile the game only:
```
# Compile the game only
scons
# or
scons compile

# Compile and build the disc image
scons build

# Clean all build artifacts
scons clean
```



## Development Tools

DragonRock comes with custom development tools to create and extend the game world. These are described in detail in the doc folder.

### tools/maped.py
A map editor for dungeon and outoors maps

### tools/mc
A map compiler.
The game content is scripted in a simple language called DRScript. The map compiler converts DRScript into bytecode which can be used in the game.

### tools/png2dbm.py
A PNG to DBM converter for displaying ingame graphics as super extended colour mode images




### Build Commands



### Directory Structure

- `src/` - Source code files
- `tools/` - Development and build tools
- `maps/` - Game maps and level data
- `graphics/` - Graphic resources
- `gamedata-src/` - Source files for game data

When building Dragon Rock, the artifacts are placed in the following folders:
- `obj/` - Compiled object files (created during build)
- `bin/` - Binary output files (created during build)
- `gamedata/` - Game resources and data files (created during build)
- `disc/` - Disc image files (created during build)

## Contributing

Contributions to DragonRock are welcome! Whether it's bug fixes, new features, or content additions, feel free to fork the repository and submit a pull request.

## License

TODO


## Acknowledgments

- Thanks to the MEGA65 team for creating an amazing platform
