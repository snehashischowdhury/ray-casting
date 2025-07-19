# Ray-Casting
3D raycasting-based game written in C using OpenGL (GLUT), similar to classic DOOM-style rendering.

# ScreenShots
 <img width="958" height="589" alt="image" src="https://github.com/user-attachments/assets/f9700b58-aee6-40b0-b925-8677700c265a" />

 <img width="959" height="596" alt="image" src="https://github.com/user-attachments/assets/befdda27-b389-455c-a0bd-a0d65e426260" />

 <img width="936" height="631" alt="image" src="https://github.com/user-attachments/assets/b09a0f49-884b-4527-bff0-33a4e72a5596" />

 <img width="957" height="634" alt="image" src="https://github.com/user-attachments/assets/9fe4c2d4-7084-4d3b-9032-ee2c910f8ea2" />

# Controls
- W: Move forward
- S: Move backward
- A: Turn left
- D: Turn right
- E: Open doors(You must first pick up the red key)
- Spacebar: Shoot 
# Objective
 Reach the exit door after defeating the enemy ("Orb King") to win.
# Map
- Defined as 9x9 grid arrays:
- mapW: Wall texture map (1-6 are walls/doors).
- mapF: Floor texture map.
- mapC: Ceiling texture map.
# Player  
- float px, py;      // player position
- float pdx, pdy;    // direction vector
- float pa;          // angle the player is facing

# Bullet System
### Each bullet is stored in a bullet struct:
 ```console  
 typedef struct
 {
  int active;         //bullet is active/inactive
  float x,y;          //bullet position
  float dx,dy;        //bullet direction
  float speed;        //bullet speed
   int life;           //bullet lifetime
 }bullet; 
 ```
## Max 10 bullets.
- shootBullet() spawns one.
- updateBullets() moves and checks collisions with walls and enemy.
   
# Sprites (Objects in the game world)
```console
 typedef struct      //All veriables per sprite
  {
    int type;          //static, key, enemy
    int state;         //on off
    int map;           //texture to show
    float x,y,z;       //position
    int hp;
  }sprite;
```
### 3 types:
- type=1: key
- type=2: lights
- type=3: enemy (Orb King)
     
- The enemy (sp[2][0]) moves toward the player when alive.
- It moves faster when the player is not looking at it.
- If enemy collides with player → game over (state 4).
   
# Doors
- Can only open if key is picked up (proximity-based).
- mapW[index] = 4 denotes a door tile.
- If mapW[index] == 4 && key collected, door opens and showBossBar = 1.

# Rendering
## drawRays2D()
### Raycasting algorithm:
- Shoots 120 rays in FOV from player's POV.
- Finds nearest wall intersection per ray.
- Handles fish-eye correction.
   
## Calculates:
- Wall shading.
- Floor/ceiling textures.
- Depth buffer for sprite rendering.
   
## drawSprite()
- Key
- Lights
- Enemy
  Based on player's rotation and depth.
   
## drawBullets()
Draws yellow circles representing bullets in 3D space.
   
## drawSky()
A static background skybox based on pa (player angle).
   
## drawBossHealthBar()
Shows a red health bar for the boss (enemy) at the top-center.
   
# Screens
### screen(v)
- v == 1: Title screen
- v == 2: Win screen
- v == 3: Lose screen
- All fade in using the fade variable.
    
# Main Loop= display()
### Handles the entire game frame by frame:
- Movement, rotation.
- Bullet update.
- Drawing sky, walls, sprites.
- Game progression and state handling.
    
# Entry Point= main()
###  Initializes the GLUT window, sets up callbacks:
- display(): frame rendering
- ButtonDown(), ButtonUp(): input handling
- resize(): window resize handling
  
# Textures
### Included from:
```console
  #include "../Textures/All_Textures.ppm"
  #include "../Textures/sky.ppm"
  #include "../Textures/title.ppm"
  #include "../Textures/won.ppm"
  #include "../Textures/lost.ppm"
  #include "../Textures/sprites.ppm"
```  
 All textures are expected to be raw .ppm images.
