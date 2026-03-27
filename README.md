This is a comprehensive `README.md` file tailored for your project. It explains the story, the features you've implemented (like the Save and Difficulty systems), and how the code is structured so that others (or future you) can understand the project quickly.

***

# Warmroom: The Last Hunter

**Warmroom** is a 2D top-down dungeon crawler built in **C++** using the **Raylib** library. You play as the last surviving Hunter of the Warmroom Civilization, tasked with reclaiming the Stellar Shards from the invading Randy Civilization to reignite the Stellar Core and save planet Aether.

## 📖 The Story
On planet Aether, the Warmroom Civilization flourished for a millennium, sustained by the power of the **Stellar Core**. The Randy Civilization descended from deep space to plunder this energy, shattering the Core into fragments. These shards gave birth to Feral Beasts, pushing Warmroom to the edge of extinction. 

Out of 20 elite Hunters, 19 have fallen. **You are the last.**

---

## 🎮 Gameplay Features
*   **Dynamic Combat:** Use a bow and arrow with orbit-based aiming.
*   **Pixel-Perfect Collision:** Arrows only register hits on the actual visible pixels of an enemy sprite, not just their "box."
*   **Difficulty System:** 
    *   **Normal:** Standard enemy speed and health.
    *   **Hard:** Enemies move **2x faster** and have **2x more health**.
*   **Save & Continue:** The game automatically saves your progress whenever you enter a new level, including your difficulty setting and "Missed Enemy" count.
*   **Branching Endings:** 
    *   **Good Ending:** Reclaim the shards and defeat Randy while clearing all feral beasts.
    *   **Bad Ending:** Defeat Randy but leave monsters roaming the ruins.
*   **Cinematic Cutscenes:** A custom typewriter-style dialogue system with color-coded text (Sky Blue for the Guide, Yellow for the Hunter).

---

## ⌨️ Controls
| Action | Key / Input |
| :--- | :--- |
| **Movement** | `W` `A` `S` `D` |
| **Aim** | Mouse Movement |
| **Shoot Arrow** | `Left Click` |
| **Interact (Portal)** | `E` |
| **Advance Dialogue** | `ENTER` |
| **Pause Game** | `ESC` |

---

## 🛠️ Technical Details
### Smart Screen Scaling
The game is rendered at a base resolution of **800x600** into a `RenderTexture2D`. It then uses mathematical letterboxing to scale that image to fit any window size (resizable) while maintaining the original aspect ratio and "pixel-art" sharpness.

### Save System
Data is stored in `save.dat` using a binary `SaveData` struct:
```cpp
typedef struct {
    int currentLevel;
    int missedEnemies;
    float difficulty;
    bool exists;
} SaveData;
```

### Level Loading
Maps are loaded using two images per level:
1.  **Visual Map:** The actual artwork the player sees.
2.  **Logic Map:** A color-coded pixel map where specific colors represent Walls (Black), Player Spawns (Green), Slimes (Red), and Portals (Blue).

---

## 📁 File Structure
*   `main.cpp`: The central game loop and state management.
*   `player/`: Handles movement, bow rotation, and multi-layered animations (Legs/Body).
*   `enemy/`: Contains the AI (Patrol vs Chase) and pixel-perfect hit detection.
*   `level/`: Logic for loading maps and merging horizontal collision barriers.
*   `cutscene/`: The "Typewriter" engine for story moments.
*   `core/save.cpp`: The logic for reading and writing progress to the disk.
*   `assets/`: Contains all `.png` textures, `.mp3` music, and `.txt` story files.

---

## 🚀 How to Build
1.  Install **Visual Studio 2022**.
2.  Install the **Raylib** library.
3.  Open the `.sln` file.
4.  Ensure the Working Directory is set to the project folder (so the game can find the `assets/` folder).
5.  Press **F5** to compile and play.

---

*“For Warmroom. For the fallen. Reclaim the light.”*

Created as a learning project using **C++ and Raylib**.
