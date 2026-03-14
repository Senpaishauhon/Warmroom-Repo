# Dungeon Game (Raylib)

A simple dungeon shooter built in **C/C++ using Raylib**.
The game features multiple levels, enemy AI, logic-map based level design, and a camera-follow system.

---

## Features

* Multiple dungeon levels
* Logic-map based level generation
* Enemy spawning and management
* Player movement and shooting
* Camera follow system
* Portal system for level progression
* Menu, pause, and difficulty selection
* Ending screen after clearing all levels

---

## Controls

| Key        | Action         |
| ---------- | -------------- |
| W A S D    | Move           |
| Mouse      | Aim            |
| Left Click | Shoot          |
| E          | Use portal     |
| ESC        | Pause / Resume |

---

## How Levels Work

Each level uses **two images**:

* `levelX.png` → the visible map
* `levelX_logic.png` → the logic map used by the game

The logic map determines gameplay elements using colors.

### Logic Map Colors

| Color | Meaning        |
| ----- | -------------- |
| Black | Wall / Barrier |
| Green | Player spawn   |
| Red   | Enemy spawn    |
| Blue  | Portal         |

Example:

Black pixels become collision barriers.
Red pixels spawn enemies.
A blue pixel creates the portal to the next level.

---

## Project Structure

```
DungeonGame
│
├── assets
│   └── maps
│       ├── level1.png
│       ├── level1_logic.png
│       └── ...
│
├── player
├── enemy
├── level
├── camera
│
├── main.cpp
└── README.md
```

---

## Running the Game

Download the release folder and run:

```
DungeonGame.exe
```

Make sure the `assets` folder stays in the same directory as the executable.

No compiler is required to run the game.

---

## Building From Source

Requirements:

* C++ compiler
* Raylib installed
* Visual Studio or another C++ IDE

Steps:

1. Clone the repository
2. Open the project in your IDE
3. Build the project in **Release** mode
4. Run the generated `.exe`

---

## Future Improvements

Planned features include:

* Boss enemies
* More enemy types
* Portal animations
* Sound effects and music
* Improved level design tools
* Better UI

---

## Author

Created as a learning project using **C++ and Raylib**.
