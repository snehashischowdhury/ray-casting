//WADS to move player, E open door after picking up the key, SPACE to shoot

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <GL/glut.h>
#include <math.h>

#include "../Textures/All_Textures.ppm"
#include "../Textures/sky.ppm"
#include "../Textures/title.ppm"
#include "../Textures/won.ppm"
#include "../Textures/lost.ppm"
#include "../Textures/sprites.ppm"
#ifndef M_PI
#define M_PI 3.14
#endif

int showBossBar=0;
float degToRad(float a) { return a*M_PI/180.0;}
float FixAng(float a){ if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}
float distance(int ax, int ay, int bx, int by, float ang){ return cos(degToRad(ang))*(bx-ax)-sin(degToRad(ang))*(by-ay);}
float px,py,pdx,pdy,pa;
float frame1,frame2,fps;
int gameState=0, timer=0; //game state. init, start screen, game loop, win/lose
float fade=0;             //the 3 screens can fade up from black

typedef struct
{
 int w,a,d,s,space;                     //button state on off - added space for shooting
}ButtonKeys; ButtonKeys Keys;

//Bullet system
typedef struct
{
 int active;         //bullet is active/inactive
 float x,y;          //bullet position
 float dx,dy;        //bullet direction
 float speed;        //bullet speed
 int life;           //bullet lifetime
}bullet; 

bullet bullets[10];  //allow up to 10 bullets at once
int bulletCount=0;   //current number of active bullets
int shootCooldown=0; //prevent rapid fire

//---------------------------MAP----------------------------------------------
#define mapX  9     //map width
#define mapY  9     //map height
#define mapS 64     //map cube size

                    //Edit these 3 arrays with values 0-4 to create your own level! 
int mapW[]=         //walls - now 9x9 = 81 elements
{
 1,1,1,1,1,2,2,2,2,
 6,0,0,1,0,0,0,0,2,
 1,0,0,4,0,2,0,0,2,
 1,5,4,5,0,0,0,0,2,
 2,0,0,0,0,0,0,0,1,
 2,0,0,0,0,1,0,0,1,
 2,0,0,0,0,0,0,0,1,
 2,0,0,0,0,0,0,0,1,
 1,1,1,1,1,1,1,1,1,	
};

int mapF[]=         //floors - now 9x9 = 81 elements
{
 0,0,0,0,0,0,0,0,0,
 0,0,0,0,2,2,2,2,0,
 0,0,0,0,6,0,2,0,0,
 0,0,8,0,2,7,6,0,0,
 0,0,2,0,0,0,0,0,0,
 0,0,2,0,2,0,0,0,0,
 0,1,1,1,1,0,2,0,0,
 0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,	 
};

int mapC[]=         //ceiling - now 9x9 = 81 elements
{
 0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,
 0,4,2,4,0,0,0,0,0,
 0,0,2,0,0,0,0,0,0,
 0,0,2,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,	
};


typedef struct      //All veriables per sprite
{
 int type;          //static, key, enemy
 int state;         //on off
 int map;           //texture to show
 float x,y,z;       //position
 int hp;
}sprite;
#define distinctSprite 3
unsigned int spriteCount[distinctSprite] = 
{   
  1,  // ex. Total Number of Keys 
  2,  // Total Number of lights ... etc.
  1
};
sprite **sp;
int depth[120];     //hold wall line depth to compare for sprite depth

//Initialize bullet system
void initBullets()
{
 int i;
 for(i=0;i<10;i++)
 {
  bullets[i].active=0;
  bullets[i].x=0;
  bullets[i].y=0;
  bullets[i].dx=0;
  bullets[i].dy=0;
  bullets[i].speed=0.8;
  bullets[i].life=0;
 }
}

//Create a new bullet
void shootBullet()
{
 if(shootCooldown<=0)
 {
  int i;
  for(i=0;i<10;i++)
  {
   if(bullets[i].active==0)
   {
    bullets[i].active=1;
    bullets[i].x=px;
    bullets[i].y=py;
    bullets[i].dx=pdx;
    bullets[i].dy=pdy;
    bullets[i].life=1000; //bullet lives for 1000 frames
    shootCooldown=20; //20 frame cooldown
    break;
   }
  }
 }
}

//Update bullets each frame
void updateBullets()
{
 int i;
 for(i=0;i<10;i++)
 {
  if(bullets[i].active==1)
  {
   //Move bullet
   bullets[i].x += bullets[i].dx * bullets[i].speed * fps;
   bullets[i].y += bullets[i].dy * bullets[i].speed * fps;
   
   //Check wall collision
   int mx=(int)(bullets[i].x)>>6;
   int my=(int)(bullets[i].y)>>6;
   int mp=my*mapX+mx;
   
   if(mp<0 || mp>=mapX*mapY || mapW[mp]>0)
   {
    bullets[i].active=0; //bullet hits wall
    continue;
   }
   
   //Check enemy collision
   if(sp[2][0].state==1) //if enemy is alive
   {
    float dx=bullets[i].x-sp[2][0].x;
    float dy=bullets[i].y-sp[2][0].y;
    float dist=sqrt(dx*dx+dy*dy);
    if(dist<30) //hit enemy
    {
     sp[2][0].hp -= 1;  //reduce HP
	 bullets[i].active = 0; //deactivate bullet
	 if(sp[2][0].hp <= 0)
	 {
	 	sp[2][0].state = 0; //enemy dies when HP reaches 0
	 }	
	 
     continue;
    }
   }
   
   //Decrease bullet life
   bullets[i].life--;
   if(bullets[i].life<=0)
   {
    bullets[i].active=0;
   }
  }
 }
 
 //Decrease shoot cooldown
 if(shootCooldown>0) shootCooldown--;
}

void drawCircle(float cx, float cy, float r, int num_segments)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy); // center of circle
    for (int i = 0; i <= num_segments; i++) {
        float theta = 2.0f * M_PI * i / num_segments;
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(cx + x, cy + y);
    }
    glEnd();
}

//Draw bullets in 3D space
void drawBullets()
{
 int i;
 for(i=0;i<10;i++)
 {
  if(bullets[i].active==1)
  {
   float sx=bullets[i].x-px;
   float sy=bullets[i].y-py;
   
   float CS=cos(degToRad(pa)), SN=sin(degToRad(pa));
   float a=sy*CS+sx*SN;
   float b=sx*CS-sy*SN;
   sx=a; sy=b;
   
   if(b>0) //bullet is in front of player
   {
    sx=(sx*108.0/sy)+(120/2);
    sy=(20*108.0/sy)+(80/2); //bullets at height 20
    
    if(sx>0 && sx<120 && sy>0 && sy<80)
    {
     glColor3ub(255, 255, 0); // yellow
     drawCircle(sx*8, sy*8, 10, 16); // radius 6 pixels, 16 segments
    }
   }
  }
 }
}

void drawSprite()
{
  int x,y,s,t;
  if(px<sp[0][0].x+30 && px>sp[0][0].x-30 && py<sp[0][0].y+30 && py>sp[0][0].y-30){ sp[0][0].state=0;} //pick up key 	
  if(px<sp[2][0].x+30 && px>sp[2][0].x-30 && py<sp[2][0].y+30 && py>sp[2][0].y-30 && sp[2][0].state==1){ gameState=4;} //enemy kills (only if alive)

  //enemy attack - UPDATED for 9x9 map - only move if enemy is alive
  if(sp[2][0].state == 1)
  {
    // Vector from player to enemy
    float dx = sp[2][0].x - px;
    float dy = sp[2][0].y - py;
    float angleToEnemy = atan2(-dy, dx) * 180.0 / M_PI;
    if(angleToEnemy < 0) angleToEnemy += 360;

    // Angle difference
    float angleDiff = fabs(FixAng(pa) - angleToEnemy);
    if(angleDiff > 180) angleDiff = 360 - angleDiff;

    // If player is NOT looking at enemy (over 45 degrees away), increase speed
    float speed = (angleDiff > 45) ? 0.09 * fps : 0.04 * fps;

    int spx=(int)sp[2][0].x>>6,          spy=(int)sp[2][0].y>>6;
    int spx_add=((int)sp[2][0].x+15)>>6, spy_add=((int)sp[2][0].y+15)>>6;
    int spx_sub=((int)sp[2][0].x-15)>>6, spy_sub=((int)sp[2][0].y-15)>>6;

    if(sp[2][0].x > px && mapW[spy*mapX + spx_sub]==0){ sp[2][0].x -= speed; }
    if(sp[2][0].x < px && mapW[spy*mapX + spx_add]==0){ sp[2][0].x += speed; }
    if(sp[2][0].y > py && mapW[spy_sub*mapX + spx]==0){ sp[2][0].y -= speed; }
    if(sp[2][0].y < py && mapW[spy_add*mapX + spx]==0){ sp[2][0].y += speed; }
  }
  for(s=0;s<distinctSprite;s++)
  {
  for (t=0;t<spriteCount[s]; t++){
  float sx=sp[s][t].x-px; //temp float variables
  float sy=sp[s][t].y-py;
  float sz=sp[s][t].z;

  float CS=cos(degToRad(pa)), SN=sin(degToRad(pa)); //rotate around origin
  float a=sy*CS+sx*SN; 
  float b=sx*CS-sy*SN; 
  sx=a; sy=b;

  sx=(sx*108.0/sy)+(120/2); //convert to screen x,y
  sy=(sz*108.0/sy)+( 80/2);

  int scale=32*80/b;   //scale sprite based on distance
  if(scale<0){ scale=0;} if(scale>120){ scale=120;}  

  //texture
  float t_x=0, t_y=31, t_x_step=31.5/(float)scale, t_y_step=32.0/(float)scale;

  for(x=sx-scale/2;x<sx+scale/2;x++)
  {
   t_y=31;
   for(y=0;y<scale;y++)
   {
    if(sp[s][t].state==1 && x>0 && x<120 && b<depth[x])
    {
     int pixel =((int)t_y*32+(int)t_x)*3+(sp[s][t].map*32*32*3);
     int red   =sprites[pixel+0];
     int green =sprites[pixel+1];
     int blue  =sprites[pixel+2];
     if(red!=255, green!=0, blue!=255) //dont draw if purple
     {
      glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,sy*8-y*8); glEnd(); //draw point 
     }
     t_y-=t_y_step; if(t_y<0){ t_y=0;}
    }
   }
   t_x+=t_x_step;
  }
 }
}
}


//---------------------------Draw Rays and Walls--------------------------------
void drawRays2D()
{	
 int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
 
 ra=FixAng(pa+30);                                                              //ray set back 30 degrees
 
 for(r=0;r<120;r++)
 {
  int vmt=0,hmt=0;                                                              //vertical and horizontal map texture number 
  //---Vertical--- 
  dof=0; side=0; disV=100000;
  float Tan=tan(degToRad(ra));
       if(cos(degToRad(ra))> 0.001){ rx=(((int)px>>6)<<6)+64;      ry=(px-rx)*Tan+py; xo= 64; yo=-xo*Tan;}//looking left
  else if(cos(degToRad(ra))<-0.001){ rx=(((int)px>>6)<<6) -0.0001; ry=(px-rx)*Tan+py; xo=-64; yo=-xo*Tan;}//looking right
  else { rx=px; ry=py; dof=8;}                                                  //looking up or down. no hit  

  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ vmt=mapW[mp]-1; dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  vx=rx; vy=ry;

  //---Horizontal---
  dof=0; disH=100000;
  Tan=1.0/Tan; 
       if(sin(degToRad(ra))> 0.001){ ry=(((int)py>>6)<<6) -0.0001; rx=(py-ry)*Tan+px; yo=-64; xo=-yo*Tan;}//looking up 
  else if(sin(degToRad(ra))<-0.001){ ry=(((int)py>>6)<<6)+64;      rx=(py-ry)*Tan+px; yo= 64; xo=-yo*Tan;}//looking down
  else{ rx=px; ry=py; dof=8;}                                                   //looking straight left or right
 
  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ hmt=mapW[mp]-1; dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  
  float shade=1;
  glColor3f(0,0.8,0);
  if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV; glColor3f(0,0.6,0);}//horizontal hit first
    
  int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca));                            //fix fisheye 
  int lineH = (mapS*640)/(disH); 
  float ty_step=32.0/(float)lineH; 
  float ty_off=0; 
  if(lineH>640){ ty_off=(lineH-640)/2.0; lineH=640;}                            //line height and limit
  int lineOff = 320 - (lineH>>1);                                               //line offset

  depth[r]=disH; //save this line's depth
  //---draw walls---
  int y;
  float ty=ty_off*ty_step;//+hmt*32;
  float tx;
  if(shade==1){ tx=(int)(rx/2.0)%32; if(ra>180){ tx=31-tx;}}  
  else        { tx=(int)(ry/2.0)%32; if(ra>90 && ra<270){ tx=31-tx;}}
  for(y=0;y<lineH;y++)
  {
   int pixel=((int)ty*32+(int)tx)*3+(hmt*32*32*3);
   int red   =All_Textures[pixel+0]*shade;
   int green =All_Textures[pixel+1]*shade;
   int blue  =All_Textures[pixel+2]*shade;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,y+lineOff); glEnd();
   ty+=ty_step;
  }
 
  //---draw floors---
 for(y=lineOff+lineH;y<640;y++)
 {
  float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
  tx=px/2 + cos(deg)*158*2*32/dy/raFix;
  ty=py/2 - sin(deg)*158*2*32/dy/raFix;
  int mp=mapF[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
  int pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  int red   =All_Textures[pixel+0]*0.7;
  int green =All_Textures[pixel+1]*0.7;
  int blue  =All_Textures[pixel+2]*0.7;
  glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,y); glEnd();

 //---draw ceiling---
  mp=mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
  pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  red   =All_Textures[pixel+0];
  green =All_Textures[pixel+1];
  blue  =All_Textures[pixel+2];
  if(mp>0){ glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,640-y); glEnd();}
 }
 
 ra=FixAng(ra-0.5);                                                               //go to next ray, 60 total
 }
}//-----------------------------------------------------------------------------


void drawSky()     //draw sky and rotate based on player rotation
{int x,y;
 for(y=0;y<40;y++)
 {
  for(x=0;x<120;x++)
  {
   int xo=(int)pa*2-x; if(xo<0){ xo+=120;} xo=xo % 120; //return 0-120 based on player angle
   int pixel=(y*120+xo)*3;
   int red   =sky[pixel+0];
   int green =sky[pixel+1];
   int blue  =sky[pixel+2];
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
  }	
 }
}

void screen(int v) //draw any full screen image. 120x80 pixels
{
 int x,y;
 int *T;
 if(v==1){ T=title;}
 if(v==2){ T=won;}
 if(v==3){ T=lost;}
 for(y=0;y<80;y++)
 {
  for(x=0;x<120;x++)
  {
   int pixel=(y*120+x)*3;
   int red   =T[pixel+0]*fade;
   int green =T[pixel+1]*fade;
   int blue  =T[pixel+2]*fade;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
  }	
 }	
 if(fade<1){ fade+=0.001*fps;} 
 if(fade>1){ fade=1;}
}


void init()//init all variables when game starts
{
  glClearColor(0.3,0.3,0.3,0);
  px=150; py=400; pa=90;
  pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));                                 //init player

  // Updated door positions for 9x9 map
  mapW[21]=4; mapW[29]=4; // position (2,3) and (3,2) respectively

  // Updated sprite positions for 9x9 map - key moved to open area
  sp = malloc(distinctSprite * sizeof(sprite*));
  sp[0] = malloc(1*sizeof(sprite));
  sp[1] = malloc(2*sizeof(sprite));
  for (int i = 0; i<distinctSprite;i++){
    sp[i] = malloc( spriteCount[i] * sizeof(sprite) );
  }
  printf("%i\n",sizeof(sprite));
  sp[0][0].type=1; sp[0][0].state=1; sp[0][0].map=0; sp[0][0].x=7*64;   sp[0][0].y=2*64;   sp[0][0].z=20;  //key
  sp[1][0].type=2; sp[1][0].state=1; sp[1][0].map=1; sp[1][0].x=1.5*64; sp[1][0].y=4.5*64; sp[1][0].z=0;   //light 1 (fixed z)
  sp[1][1].type=2; sp[1][1].state=1; sp[1][1].map=1; sp[1][1].x=3.5*64; sp[1][1].y=4.5*64; sp[1][1].z=0;   //light 2 (fixed z)
  sp[2][0].type=3; sp[2][0].state=1; sp[2][0].map=2; sp[2][0].x=2.5*64; sp[2][0].y=2*64;   sp[2][0].z=20;  sp[2][0].hp=20;  //enemy
  
  initBullets(); //initialize bullet system
}

void drawText(int x, int y, const char *text)
{
  glColor3ub(255, 85, 0); // orange text
  glRasterPos2i(x, y);
  while(*text)
  {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
    text++;
  }
}

void drawBossHealthBar()
{
 //printf("Boss Bar Visible: %d\n",showBossBar);	
 if(sp[2][0].state == 1 && showBossBar == 1) // only show if enemy is alive
 {
  int maxHP = 20;
  float hpRatio = sp[2][0].hp / (float)maxHP;
  int barWidth = 300 * hpRatio; // max width = 300 pixels

  // --- Draw bar background (dark gray) ---
  glColor3ub(40, 40, 40);
  glBegin(GL_QUADS);
   glVertex2i(330, 20);
   glVertex2i(630, 20);
   glVertex2i(630, 40);
   glVertex2i(330, 40);
  glEnd();

  // --- Draw red health bar ---
  glColor3ub(200, 0, 0);
  glBegin(GL_QUADS);
   glVertex2i(330, 20);
   glVertex2i(330 + barWidth, 20);
   glVertex2i(330 + barWidth, 40);
   glVertex2i(330, 40);
  glEnd();

  // --- Draw "Orb" label above the bar ---
  drawText(450 - 15, 15, "ORB KING");
 }
}

void display()
{  
 //frames per second
 frame2=glutGet(GLUT_ELAPSED_TIME); fps=(frame2-frame1); frame1=glutGet(GLUT_ELAPSED_TIME); 
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

 if(gameState==0){ init(); fade=0; timer=0; gameState=1;} //init game
 if(gameState==1){ screen(1); timer+=1*fps; if(timer>2000){ fade=0; timer=0; gameState=2;}} //start screen
 if(gameState==2) //The main game loop
 {
  //buttons
  if(Keys.a==1){ pa+=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
  if(Keys.d==1){ pa-=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 

  int xo=0; if(pdx<0){ xo=-20;} else{ xo=20;}                                    //x offset to check map
  int yo=0; if(pdy<0){ yo=-20;} else{ yo=20;}                                    //y offset to check map
  int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0, ipx_sub_xo=(px-xo)/64.0;             //x position and offset
  int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0, ipy_sub_yo=(py-yo)/64.0;             //y position and offset
  if(Keys.w==1)                                                                  //move forward
  {  
   if(mapW[ipy*mapX        + ipx_add_xo]==0){ px+=pdx*0.2*fps;}
   if(mapW[ipy_add_yo*mapX + ipx       ]==0){ py+=pdy*0.2*fps;}
  }
  if(Keys.s==1)                                                                  //move backward
  { 
   if(mapW[ipy*mapX        + ipx_sub_xo]==0){ px-=pdx*0.2*fps;}
   if(mapW[ipy_sub_yo*mapX + ipx       ]==0){ py-=pdy*0.2*fps;}
  } 
  
  updateBullets(); //update bullets each frame
  
  drawSky();
  drawRays2D();
  drawSprite();
  drawBullets(); //draw bullets
  if (showBossBar)
  {
   drawBossHealthBar();
  }
  
  if( (int)px>>6==1 && (int)py>>6==1 && sp[2][0].state==0 ){ fade=0; timer=0; gameState=3;} //Entered block 1, Win game!!
 }

 if(gameState==3){ screen(2); timer+=1*fps; if(timer>2000){ fade=0; timer=0; gameState=0; showBossBar=0;}} //won screen
 if(gameState==4){ screen(3); timer+=1*fps; if(timer>2000){ fade=0; timer=0; gameState=0; showBossBar=0;}} //lost screen

 glutPostRedisplay();
 glutSwapBuffers();  
}

void ButtonDown(unsigned char key,int x,int y)                                  //keyboard button pressed down
{
 key = tolower(key); 
 
 if(key=='a'){ Keys.a=1;} 	
 if(key=='d'){ Keys.d=1;} 
 if(key=='w'){ Keys.w=1;}
 if(key=='s'){ Keys.s=1;}
 if(key==' '){ Keys.space=1; shootBullet();} //spacebar to shoot
 if(key == 'e' && sp[0][0].state == 0)
{ 
    int xo = (pdx < 0) ? -25 : 25;
    int yo = (pdy < 0) ? -25 : 25;
    int ipx = px / 64.0;
    int ipx_add_xo = (px + xo) / 64.0;
    int ipy = py / 64.0;
    int ipy_add_yo = (py + yo) / 64.0;

    int index = ipy_add_yo * mapX + ipx_add_xo;

    // Toggle door open/closed
    if(mapW[index] == 4)
    {
        mapW[index] = 0;          // Open the door
        showBossBar = 1;          // Show the health bar
    }
}

 glutPostRedisplay();
}

void ButtonUp(unsigned char key,int x,int y)                                    //keyboard button pressed up
{
 key = tolower(key);
  
 if(key=='a'){ Keys.a=0;} 	
 if(key=='d'){ Keys.d=0;} 
 if(key=='w'){ Keys.w=0;}
 if(key=='s'){ Keys.s=0;}
 if(key==' '){ Keys.space=0;}
 glutPostRedisplay();
}

void resize(int w,int h)                                                        //screen window rescaled, snap back
{
 glutReshapeWindow(960,640);
}

int main(int argc, char* argv[])
{ 
 glutInit(&argc, argv);
 glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
 glutInitWindowSize(960,640);
 glutInitWindowPosition( glutGet(GLUT_SCREEN_WIDTH)/2-960/2 ,glutGet(GLUT_SCREEN_HEIGHT)/2-640/2 );
 glutCreateWindow("Game");
 gluOrtho2D(0,960,640,0);
 init();
 glutDisplayFunc(display);
 glutReshapeFunc(resize);
 glutKeyboardFunc(ButtonDown);
 glutKeyboardUpFunc(ButtonUp);
 glutMainLoop();
}
