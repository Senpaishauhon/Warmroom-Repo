
Here is the raw Markdown text. You can click the "Copy code" button at the top right of the box below, and paste it directly into your README.md file!
code
Markdown
# 📜 Game Design & Implementation Document

**Project:** Warmroom: The Last Hunter  
**Engine:** Raylib (C++14)  
**Date:** April 2026  

## 1. Project Overview & Lore
**Warmroom: The Last Hunter** is a 2D top-down action-adventure game. 

**The Lore:** On planet Aether, the Warmroom Civilization thrived on the power of the Stellar Core until the Randy Civilization invaded and shattered it, releasing Feral Beasts. You play as the last surviving Hunter of a 20-man elite squad. Your mission is to retrieve the scattered Stellar Shards, defeat Randy, and save your civilization.

---

## 2. System Architecture (General Framework)
To prevent messy, hard-to-read code, our game is divided into strictly decoupled modules. `main.cpp` acts as the central conductor, managing the Game State and passing data between modules via pointers. 

### Module Interaction Diagram:
```text
                     +---------------------------+
                     |  main.cpp                 |
                     |  (Game Loop & States)     |
                     +-------------+-------------+
                                   |
       +---------------+-----------+-----------+---------------+
       |               |                       |               |
       v               v                       v               v
+------------+  +--------------+       +--------------+  +-------------+
| player.cpp |  | enemy.cpp    |       | level.cpp    |  | audio.cpp   |
| (Input &   |  | (AI, Hitbox, |       | (Wall logic, |  | (BGM, SFX,  |
| Animation) |  |  Health)     |       |  Spawns)     |  |  Volumes)   |
+------------+  +--------------+       +--------------+  +-------------+
       |               |                       |                |
       +---------------+-----------------------+                v
                       |                                +--------------+
                       v                                | cutscene.cpp |
             +--------------------+                     | (Lore & Text)|
             | raylib.h (Engine)  |                     +--------------+
             +--------------------+
3. Key Data Structures
Our game uses specific data structures to maximize memory efficiency and prevent crashes.
A. EnemyManager (Texture Caching)
Instead of every Slime or Boss loading its own image into the computer's RAM, the EnemyManager loads the textures exactly once. The individual Enemy structs only hold lightweight data (HP, X/Y position, and type) and share the Manager's textures for drawing.
B. Cutscene (Heap Allocation)
Our cutscene struct holds up to 32 blocks of 2048-character text arrays. Because this equals roughly 65 Kilobytes of data, creating it normally on the Stack would cause a Stack Overflow crash. We solved this by allocating it to the Heap using pointers:
code
C++
// Safely allocated to the Heap to prevent memory crashes
Cutscene* cutscene = new Cutscene(); 

// ... later in shutdown ...
delete cutscene; // Memory returned to the PC
4. Key Algorithms & Logic
A. The "Logic Map" Level Parser
Instead of manually typing out X/Y coordinates for every wall and enemy, level.cpp scans a hidden level_logic.png image pixel-by-pixel.
Black Pixels (R:0, G:0, B:0) = Generates an invisible solid Wall.
Green Pixels (G>200) = Sets the Player's spawn point.
Red Pixels (R>200) = Spawns a Slime enemy.
Magenta/Yellow Pixels = Spawns Boss 1 or Boss 2.
[📸 Insert Screenshot Here: Show an image of your game level side-by-side with its colorful logic map]
B. Independent Wall Sliding & Nudging
Top-down games often suffer from "sticky" walls. We solved this in LevelUpdate by checking the X-axis and Y-axis independently.
If the player hits a wall while holding W and D, the game only cancels the blocked axis, allowing the player to seamlessly slide along the wall on the free axis. Furthermore, we implemented a "Nudge" loop that pushes the player 1-8 pixels to automatically slip past rough, jagged map edges.
C. Pixel-Perfect Collision
Because our Boss sprites are large 200x200 images with transparent empty corners, a standard box collision would cause arrows to hit "invisible air." We wrote an algorithm that mathematically maps the arrow's screen coordinates to the enemy's texture, checks the Alpha (transparency) channel of that specific pixel, and only deals damage if the pixel is solid.
D. Secure Save/Load System
To track the player's progress (Level, Missed Enemies, Difficulty), we implemented a Save/Load system. To comply with modern security standards and prevent C4996 buffer overflow vulnerabilities, we strictly used Microsoft's secure C-library functions (fopen_s, fscanf_s, and fprintf_s). The game automatically wipes the save file if the player dies or beats the game, preserving the hardcore roguelike integrity.
[📸 Insert Screenshot Here: Show your Main Menu with the Continue button]
5. Game States
The game flow is controlled by a central GameState enum in main.cpp. This ensures that UI, gameplay, and cutscenes never overlap.
code
Text
[ MENU_SETTINGS ] (Audio Sliders)
              ^
              | 
              v
         [ MENU ] --------> (Click Play) --------> [ DIFFICULTY ]
              ^                                          |
              |                                          v
      (Game Over / Win)                       [ CUTSCENE (Text) ]
     (Save File Wiped)                                   |
              |                                          v
              +<---------- [ PAUSE ] <------------[ GAMEPLAY ]
                         (Press Escape)         (Player moves/fights)
6. AI Usage Acknowledgment
In accordance with assignment requirements, we openly disclose the use of AI tools during the development of this project. AI was used as an educational and structural assistant in the following ways:
Architecture & Refactoring: Assisted in decoupling our initial monolithic code into modular files (player.cpp, enemy.cpp, audio.cpp) via header inclusions.
Algorithm Math: Provided the mathematical basis for the Pixel-Perfect Collision algorithm, the Screen Scaling (Letterboxing) math, and the independent Axis Wall-Sliding logic.
Debugging Assistance: Helped diagnose and resolve complex Visual Studio C++ errors, such as LNK1104 missing library paths, LNK2005 duplicate symbol conflicts, and Stack Overflow (C6262) warnings by guiding us to use Heap pointers (new).
Dialogue Formatting: Assisted in translating and formatting our original story concepts into structured text files compatible with our custom C++ typewriter cutscene system.
Note: All core lore, level designs, artwork selection, and gameplay balancing were directed and implemented by the human development team.
7. Build & Run Instructions
To run the game:
Extract the submitted .zip folder.
Ensure the assets/ folder is located exactly next to the .exe file.
Double-click Warmroom_The_Last_Hunter.exe.
(Note: The game was compiled using static libraries and the /SUBSYSTEM:windows linker command in Release mode, so no background console will appear and no extra DLLs are required).
8. Build & Run Instructions
To run the game:
Extract the submitted .zip folder.
Ensure the assets/ folder is located exactly next to the .exe file.
Double-click Warmroom_The_Last_Hunter.exe.
(Note: The game was compiled using static libraries and the /SUBSYSTEM:windows linker command in Release mode, so no background console will appear and no extra DLLs are required).
