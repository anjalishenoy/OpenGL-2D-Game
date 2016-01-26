#include <iostream>
#include <stdio.h>
#include <cstring>
#include <string.h>
#include <cstdlib>
#include <stdlib.h>
#include <math.h>
#include <time.h>			//for clock
#include <unistd.h>			//for sleep
#include <vector>
#include <cstdarg>			//for va_list
#include <GL/freeglut.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "imageloader.h"
#include "text3d.h"

using namespace std;

#define LEN 8192			//for printing texture
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define ON 1.0f 	//Boolean Values, float to maintain generality
#define OFF 0.0f
/* RWC --> Real world coordinates */

typedef struct point
{
	float x,y,z;
}point;

point COM = { -10.0f, -4.1f, -15.98f};			//COM of cannon
point bombInit= { 1.95f, 0.90f, 0.0f };		//Initial position of Bomb at mouth of Cannon in Cannon's Matrix
point prevBomb;
point bomb= { 1.95f, 0.90f, 0.0f };			//Bomb's coord in Cannon's matrix
point coins[3]={ 3.5f, -3.10f, -15.98f, 5.0f, -2.0f, -15.98f, 6.5f, -3.10f, -15.98f };
point badcoins={5.0f, -3.8f, -15.98f};

float mouseX=-10000, mouseY=-10000;
float windowWidth, windowHeight;

//cannon angle=initial angle ASSUMING starting is 0 degrees  (22.4 degrees drawn by hardcode >.<)
float cannonAngle=0.0f;     //cannonAngle=0.0ff;		
float angle, initVelocity=2.0f, vel=2.0f;

float projectileMotion=OFF;				//Flag to check if projectile is in flight
float floorCollision=OFF;				//if it hit the floor
float wallCollision=OFF;
float GameOver=OFF;
float t=0.0f;							//timer of bomb

float _targetAngle=0.0f;

int score, turn=0;
int collected[3]={0,0,0};		//to check if coins are collected
float bombRadius=0.45f;			//Initial radius

float zoomFactor=1.0f, panFactor=0.0f;

void loadBomb(float x, float y, float z);

/*------------------Only for printing scores------------*/
//--------------------------------------------------------

const char* STRS[2] = {"Score:"};

float computeScale(const char* strs[1]) {
	float maxWidth = 0;
	for(int i = 0; i < 1; i++) {
		float width = t3dDrawWidth(strs[i]);
		if (width > maxWidth) {
			maxWidth = width;
		}
	}	
	return 2.6f / maxWidth;
}

void cleanup() {
	t3dCleanup();
}

void Score()
{
	float _scale;
	_scale = computeScale(STRS);

	for(int i = 0; i < 1; i++) {
		glPushMatrix();
			glScalef(_scale, _scale, _scale);
			glTranslatef(9, 7, -16.5f / _scale);
			t3dDraw3D(STRS[i], 0, 0, 0.2f);
		glPopMatrix();
	}

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "%d", score);
	string str(buffer);

	glPushMatrix();
		glColor3f(0, 0, 1.0f);
		glTranslatef(8, 4.35, -15);
		t3dDraw3D(str, 0, 0, 0.1f);
	glPopMatrix();

	const char* Speed[1] = {"Velocity:"};
	_scale = computeScale(Speed);

	for(int i = 0; i < 1; i++) {
		glPushMatrix();
			glScalef(_scale, _scale, _scale);
			glTranslatef(0, 9.75, -16.5f / _scale);
			t3dDraw3D(Speed[i], 0, 0, 0.2f);
		glPopMatrix();
	}

	snprintf(buffer, sizeof(buffer), "%.3f", vel);
	string velbuff(buffer);
	glPushMatrix();
		glColor3f(0, 0, 0.0f);
		glScalef(0.55, 0.55, 1);
		glTranslatef(5, 8.5, -16.5);
		t3dDraw3D(velbuff, 0, 0, 0.2f);
	glPopMatrix();

}
//-----------------------------------------------------------
/*----------------------------------------------------------*/


//-------------FINAL SCENE----------------------------------
void finalScene()
{
	const char* word[5] = {"Game   ", "Over!  ", "Your ", "Score", "is: "};
	float _scale;
	_scale = computeScale(word);

	for(int i = 0; i < 5; i++) {
		glPushMatrix();
			glColor3f(0.0, 0, 1.0f);
			glScalef(_scale, _scale, _scale);
			glTranslatef(-7+3.5*i, 1, -15);
			t3dDraw3D(word[i], 0, 0, 0.2f);
		glPopMatrix();
	}

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "%d", score);
	string str(buffer);

	glPushMatrix();
		glColor3f(1.0, 0, 0.0f);
		glTranslatef(-1, -2, -15);
		t3dDraw3D(str, 0, 0, 0.1f);
	glPopMatrix();

}

void handleKeypress(unsigned char key, int x, int y) {
	float var1,var2, moveAngle;
	switch (key) {
		case 32:		//SpaceBar
			if(projectileMotion==OFF)
			{
				angle=cannonAngle+24.4;							//22.4 degree initially,
				projectileMotion=ON;		//Firing
				bombInit=bomb;				//To know where bomb was last
				
				bomb.x+=COM.x;				//Translate bomb coord into RWC
				bomb.y+=COM.y;
				bomb.z+=COM.z;
			}
			break;
		case 'a':
			if(cannonAngle+5.0f<65.0f && projectileMotion==OFF)				//Cannot move cannon mouth when bomb in air
			{
				moveAngle=-5.0f;			//Needs to be +5 for CCW, but goes in opposite direction so it is -
				cannonAngle+=5.0f;			//This is for makeCannon to use

				/* To calculate Matrix co-ordinates of bomb after rotating */
				var1 = bomb.y*cos(DEG2RAD(moveAngle)) - bomb.x*sin(DEG2RAD(moveAngle));
				var2= bomb.y*sin(DEG2RAD(moveAngle)) + bomb.x*cos(DEG2RAD(moveAngle));
				
				bomb.x=var2;
				bomb.y=var1;
				
			}
				break;
		case 'd':
			if(cannonAngle-5.0f>-6 && projectileMotion==OFF)					////Cannot move cannon mouth when bomb in air
			{	
				moveAngle=+5.0f;			//Needs to be -5 for CW, but +5 works so ..
				cannonAngle-=5.0f;			//For makecannon to use

				/* To calculate Matrix co-ord of bomb after rotating */
				var1 = bomb.y*cos(DEG2RAD(moveAngle)) - bomb.x*sin(DEG2RAD(moveAngle));
				var2= bomb.y*sin(DEG2RAD(moveAngle)) + bomb.x*cos(DEG2RAD(moveAngle));
				bomb.x=var2;
				bomb.y=var1;
			}
			break;
		
		case 27: //Escape key
			cleanup();
			exit(0);

		case 'f':{
			if(projectileMotion==ON)			//Disable f when in projectile motion
				return;
			initVelocity+=0.1f;
			vel=initVelocity;
			break;
		}
		case 's':{
			if(projectileMotion==ON)		//No changing speed while in air
				return;
			if(initVelocity>0)
			{
				initVelocity-=0.1f;
				vel=initVelocity;
			}
			break;
		}
	}
}

void handleKeypress2(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_LEFT :{
			if(projectileMotion==OFF && panFactor<=7.0f)
				panFactor+=0.09f;
		 break;
		}
		case GLUT_KEY_RIGHT: {
			if(projectileMotion==OFF && panFactor>=-7.0f)
				panFactor-=0.09f;
			break;
		}
		case GLUT_KEY_UP:{
			if(projectileMotion==OFF && zoomFactor<=2.0f)
				zoomFactor+=0.01;
			break;
		}
		case GLUT_KEY_DOWN: {
			if(projectileMotion==OFF && zoomFactor>1)
			{
				zoomFactor-=0.01;
			}
			break;
		}
	}
}

void checkMouseCoordinates(int x,int y){
    mouseX=(float)x-(float)windowWidth/2;		//Getting pixels into coordinates
    mouseY=(float)windowHeight/2-(float)y;
    glutPostRedisplay();
}

void handleMouseclick(int button, int state, int z, int y)
{
	float X,Y, arc, currAngle, diff, var1, var2;

	if(state==GLUT_DOWN && button==GLUT_LEFT_BUTTON)			//down state of mouse
	{
		X=((mouseX*12.7*2)/windowWidth)-COM.x-panFactor;
		Y=((mouseY*6.7*2)/windowHeight)-COM.y;							//on windowWidth/2 pixels, it is mouseX, so on half length of background, this much
		arc=(atan2(Y,X)*180)/PI;			//Angle made in degrees
		currAngle=cannonAngle+22.4;

		if(arc>90.0f || arc < 19.0f || projectileMotion==ON)
			return ;
		if(currAngle<arc)			//going up
		{
			diff=(arc-currAngle);
			cannonAngle+=diff;
			diff*=-1;
			var1 = bomb.y*cos(DEG2RAD(diff)) - bomb.x*sin(DEG2RAD(diff));
			var2= bomb.y*sin(DEG2RAD(diff)) + bomb.x*cos(DEG2RAD(diff));
				
			bomb.x=var2;
			bomb.y=var1;

		}
		else if(currAngle>arc)			//going down
		{
			diff=(currAngle-arc);
			cannonAngle-=diff;
			var1 = bomb.y*cos(DEG2RAD(diff)) - bomb.x*sin(DEG2RAD(diff));
			var2= bomb.y*sin(DEG2RAD(diff)) + bomb.x*cos(DEG2RAD(diff));
				
			bomb.x=var2;
			bomb.y=var1;
		}

	}
	else if(state==GLUT_DOWN && button==GLUT_RIGHT_BUTTON)
	{
		X=((mouseX*12.7*2)/windowWidth);
		if(X<=6.7f && X>=-6.7f)
		{
			panFactor=(-1)*X;			//Translate by 
		}
	}
	else if(button == 3 || button == 4)			//it's a mouse wheel event
	{
		if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events
		if(button==3 && zoomFactor<=2.0f)
			zoomFactor+=0.01;
		else if ( button==4 && zoomFactor>1 )
			zoomFactor-=0.01;
	}
}

//Makes the image into a texture, and returns the id of the texture
GLuint loadTexture(Image* image) {
	GLuint textureId;
	glGenTextures(1, &textureId); //Make room for our texture
	glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
	//Map the image to the texture
	glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
				 0,                            //0 for now
				 GL_RGB,                       //Format OpenGL uses for image
				 image->width, image->height,  //Width and height
				 0,                            //The border of the image
				 GL_RGB, //GL_RGB, because pixels are stored in RGB format
				 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
				                   //as unsigned numbers
				 image->pixels);               //The actual pixel data
	return textureId; //Returns the id of the texture
}

GLuint _textureId; //The id of the texture

void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	score=0;
	t3dInit();
	Image* image = loadBMP("vtr.bmp");
	_textureId = loadTexture(image);
	delete image;
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)w / (float)h, 1.0, 200.0);
	windowHeight=h;			//setting new window width and height
	windowWidth=w;
}

void collectCoins(float x, float y)
{
	float dist;
	for(int i=0; i<3; i++)
	{
		dist=pow((x-coins[i].x)*(x-coins[i].x)+(y-coins[i].y)*(y-coins[i].y), 0.5);
		if(dist<=bombRadius && collected[i]==0)
		{
			score+=10;
			collected[i]=1;
		}
	}
	dist=pow((x-badcoins.x)*(x-badcoins.x)+(y-badcoins.y)*(y-badcoins.y), 0.5);
	if(dist<=bombRadius)
		score-=5;
}

void drawCircle(float rad)
{
	glBegin(GL_TRIANGLE_FAN	);
    for(int i=0 ; i<360 ; i++)
       	glVertex2f(rad * cos(DEG2RAD(i)), rad * sin(DEG2RAD(i)));
    	
   	 glEnd();
}

void renderBackground()
{
	 //For background
    glEnable(GL_TEXTURE_2D);		//enable texture
    glBindTexture(GL_TEXTURE_2D, _textureId);	//render texturing of bmp image
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	//from code
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPushMatrix();		
		
		glTranslatef(0.0f, 0.0f, -17.50f);		//Pushing background 16 infront of world
		glColor3f(1.0f, 1.0f, 1.0f);			//Important while adding textures
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);			//render co-ordn of texture, found by trial and error
			glVertex3f(-12.7f, -6.7f, 0.0f);		//one vertext of the rectangle
			
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(12.7f, -6.7f, 0.0f);

			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(12.7f, 6.7f, 0.0f);
			
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-12.7f, 6.7f, 0.0f);
		
		glEnd();

	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);		//In real world, texture should be restored.
	glDisable(GL_TEXTURE_2D);				//Disable texture
}

void loadBomb()
{
	glPushMatrix();
		glTranslatef(-10.0f, -4.1f, -15.98f);		//COM of cannon
		//No need to rotate like cannon mouth, we found Matrix coord after rotation in handlekeyPress
		glColor3f(0.0f, 0.0f, 0.0f);
		glPushMatrix();
			glTranslatef(bomb.x, bomb.y, 0.0f);		//Placing center of circle of bomb near mouth of cannon
			drawCircle(bombRadius);
		glPopMatrix();
	glPopMatrix();

}

void shootBomb()
{
	sleep(0.01);
	t=t+0.01;
	prevBomb=bomb;				//storing previous co-ordinates of bomb;
	bomb.x+=( initVelocity ) * cos(DEG2RAD(angle))*t ;
	bomb.y+=(( initVelocity )*sin(DEG2RAD(angle))*t)-(float)(0.5*9.8*t*t);

	glPushMatrix();
		glTranslatef(bomb.x, bomb.y, bomb.z);				//HERE bomb co-ordinates are made into RWC after space is tapped in handlekeypress
		glColor3f(0.0f, 0.0f, 0.0f);
		drawCircle(bombRadius);
	glPopMatrix();

	collectCoins(bomb.x, bomb.y);	
	if(bomb.x>=12.0f || bomb.x<=-100.0f)					//if out of boundary, start all again
	{
		bomb=bombInit;
		projectileMotion=OFF;
		wallCollision=OFF;
		t=0;
		turn++;
		bombRadius+=0.10f;
		if(turn>3)
			GameOver=ON;
	}		
	
	if(bomb.y<=0.0f && bomb.x>=(-1)*bombRadius && bomb.x<=1)		//hits wall
	{	
		initVelocity=(-1)*initVelocity;
		angle=-1*angle/10;
		wallCollision=ON;
	}
	else if(bomb.y<=-3.7f)						//if on ground, roll!
	{
		bomb.y=-3.7f;
		if(wallCollision==ON)
			bomb.x-=0.001;
		else
			bomb.x+=0.001;	
	}
}

void makeCannon()
{
	glPushMatrix();				//For whole cannon

	//cannon's front triangle
	glPushMatrix();
		glTranslatef(-10.0f, -4.1f, -15.99f);	/*THIS IS COM OF CANNON*/
		glBegin(GL_TRIANGLES);
			glColor3f(0.20f,0.016f, 0.010f);
			
			glVertex2f(0.75f, -0.55f);		
			glVertex2f(-0.75f, -0.55f);
			glVertex2f(0.0f, 0.20f);	

		glEnd();
	glPopMatrix();

	//Cannon's circle;		
	glPushMatrix();
		glTranslatef(-10.0f, -4.1f, -15.99f);		//Center of cannon
		glColor3f(0.33,0.08f, 0.11f);
		drawCircle(0.53f);
	glPopMatrix();

	//cannon's mouth
	glPushMatrix();
		glTranslatef(-10.0f, -4.1f, -15.99f);		//Center of cannon
		glRotatef(cannonAngle, 0.0f, 0.0f, 1.0f);		//cannonAngle initially 0 (actually 22.4 due to hardcoding)
		glBegin(GL_QUADS);	
			glColor3f(0.26,0.17f, 0.18f);
			glVertex2f(-0.374f, 0.374f);
			glVertex2f(0.354f, -0.385f);
			glVertex2f(1.80f, 0.60f);
			glVertex2f(1.55f, 0.95f);

		glEnd();
	glPopMatrix();

	//Cannon's second triangle
	glPushMatrix();
		glTranslatef(-10.0f, -4.1f, -15.98f);	//Second triangle Placed infront of triangle at -15.98
		glBegin(GL_TRIANGLES);
			glColor3f(0.86,0.20f, 0.09f);
			
			glVertex2f(0.55f, -0.54f);		
			glVertex2f(-0.55f, -0.54f);
			glVertex2f(0.0f, 0.02f);	

		glEnd();
	glPopMatrix();

	glPopMatrix();				//End of cannon


}
void makeWall()
{
	float d=0.0f;
	glPushMatrix();
		glBegin(GL_QUADS);
			glColor3f(0.20f, 0.0f, 0.0f);			//Dark Brown
			glVertex3f(0.0f, 0.0f, -15.99f);
			glVertex3f(1.0f, 0.0f, -15.99f);
			glVertex3f(1.0f, -0.50f, -15.99f);
			glVertex3f(0.0f, -0.50f, -15.99f);

			glColor3f(0.60f, 0.10f, 0.10f);				//Brown
			glVertex3f(0.0f, -0.50f, -15.99f);
			glVertex3f(1.0f, -0.50f, -15.99f);
			glVertex3f(1.0f, -1.50f, -15.99f);
			glVertex3f(0.0f, -1.50f, -15.99f);

			glColor3f(0.20f, 0.0f, 0.0f);			//Dark Brown
			glVertex3f(0.0f, -1.5f, -15.99f);
			glVertex3f(1.0f, -1.50f, -15.99f);
			glVertex3f(1.0f, -2.0f, -15.99f);
			glVertex3f(0.0f, -2.0f, -15.99f);

			glColor3f(0.60f, 0.10f, 0.10f);				//Brown
			glVertex3f(0.0f, -2.0f, -15.99f);
			glVertex3f(1.0f, -2.0f, -15.99f);
			glVertex3f(1.0f, -3.0f, -15.99f);
			glVertex3f(0.0f, -3.0f, -15.99f);

			glColor3f(0.20f, 0.0f, 0.0f);			//Dark Brown
			glVertex3f(0.0f, -3.0f, -15.99f);
			glVertex3f(1.0f, -3.0f, -15.99f);
			glVertex3f(1.0f, -3.50f, -15.99f);
			glVertex3f(0.0f, -3.50f, -15.99f);

			glColor3f(0.60f, 0.10f, 0.10f);				//Brown
			glVertex3f(0.0f, -3.50f, -15.99f);
			glVertex3f(1.0f, -3.50f, -15.99f);
			glVertex3f(1.0f, -4.50f, -15.99f);
			glVertex3f(0.0f, -4.50f, -15.99f);

			glColor3f(0.20f, 0.0f, 0.0f);			//Dark Brown
			glVertex3f(-0.50f, -4.250f, -15.99f);
			glVertex3f(1.50f, -4.250f, -15.99f);
			glVertex3f(1.50f, -4.75f, -15.99f);
			glVertex3f(-0.50f, -4.75f, -15.99f);
		glEnd();

		glTranslatef(d, 0.0f, 0.0f);
	glPopMatrix();
}

void makeTargets()
{
	if(collected[0]==0)			//Not collected, print it;
	{
		glPushMatrix();
			glTranslatef(3.50f, -3.10f, -15.98);
			glRotatef(_targetAngle, 0.0f, 1.0f, 0.0f);
			glBegin(GL_TRIANGLES);
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(-1.0f, 0.0f, 0.0f);
				
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(1.0f, 0.0f, 0.0f);
				
				glColor3f(0.93f, 0.79f, 0.0f);
				glVertex3f(0.0f, 1.0f, 0.0f);
				
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(-1.0f, 0.0f, 0.0f);
				
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(1.0f, 0.0f, 0.0f);	
			
				glColor3f(0.93f, 0.79f, 0.0f);
				glVertex3f(0.0f, -1.0f, 0.0f);
			glEnd();	

			glBegin(GL_QUADS);
				glColor3f(0.2f, 0.0f, 0.0f);
				glVertex3f(0.0f, 0.50f, 0.0f);
				glVertex3f(0.5f, 0.0f, 0.0f);
				glVertex3f(0.0f, -0.5f, 0.0f);
				glVertex3f(-0.5f, 0.0f, 0.0f);
			glEnd();
		glPopMatrix();
	}

	if(collected[1]==0)					//If 2nd target is not collected, print it
	{
		glPushMatrix();
			glTranslatef(5.0f, -2.0f, -15.98);
			glRotatef(_targetAngle, 0.0f, 1.0f, 0.0f);
			glBegin(GL_TRIANGLES);
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(-1.0f, 0.0f, 0.0f);
				
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(1.0f, 0.0f, 0.0f);
				
				glColor3f(0.93f, 0.79f, 0.0f);
				glVertex3f(0.0f, 1.0f, 0.0f);
				
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(-1.0f, 0.0f, 0.0f);
				
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(1.0f, 0.0f, 0.0f);
			
				glColor3f(0.93f, 0.79f, 0.0f);
				glVertex3f(0.0f, -1.0f, 0.0f);
			glEnd();

			glBegin(GL_QUADS);
				glColor3f(0.2f, 0.0f, 0.0f);
				glVertex3f(0.0f, 0.50f, 0.0f);
				glVertex3f(0.5f, 0.0f, 0.0f);
				glVertex3f(0.0f, -0.5f, 0.0f);
				glVertex3f(-0.5f, 0.0f, 0.0f);
			glEnd();
		glPopMatrix();
	}

	if(collected[2]==0)
	{
		glPushMatrix();
			glTranslatef(6.50f, -3.10f, -15.98);
			glRotatef(_targetAngle, 0.0f, 1.0f, 0.0f);
			glBegin(GL_TRIANGLES);
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(-1.0f, 0.0f, 0.0f);
				
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(1.0f, 0.0f, 0.0f);
				
				glColor3f(0.93f, 0.79f, 0.0f);
				glVertex3f(0.0f, 1.0f, 0.0f);
				
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(-1.0f, 0.0f, 0.0f);
				
				glColor3f(0.93f, 0.46f, 0.0f);
				glVertex3f(1.0f, 0.0f, 0.0f);
				
				glColor3f(0.93f, 0.79f, 0.0f);
				glVertex3f(0.0f, -1.0f, 0.0f);
			glEnd();

			glBegin(GL_QUADS);
				glColor3f(0.2f, 0.0f, 0.0f);
				glVertex3f(0.0f, 0.50f, 0.0f);
				glVertex3f(0.5f, 0.0f, 0.0f);
				glVertex3f(0.0f, -0.5f, 0.0f);
				glVertex3f(-0.5f, 0.0f, 0.0f);	
			glEnd();
		glPopMatrix();
	}

	//negative target
	glPushMatrix();
		glTranslatef(5.0f, -3.80f, -15.98);
		glRotatef(_targetAngle, 0.0f, 1.0f, 0.0f);
		glColor3f(0.2f, 0.0f, 0.0f);
		drawCircle(0.45);
		glColor3f(0.93f, 0.46f, 0.0f);
		drawCircle(0.25);
	glPopMatrix();
}


void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(zoomFactor, zoomFactor, 1.0f);
    glTranslatef(panFactor, 0.0f, 0.0f);

    //glPushMatrix();
    if(GameOver==ON || score==30)
    	finalScene();
    else
    {
  		renderBackground();					//render the background
		makeCannon();						//Draw a cannon
		makeWall();							//Draw Barrier
		makeTargets();
		if(projectileMotion==OFF)		//Dont load bomb if projectile is in path
		{
			initVelocity=vel;
			loadBomb();							//Draw the bomb at initial point
		}
		if(projectileMotion==ON)
			shootBomb();

		Score();
	}
 	glutSwapBuffers();
}

void update(int value) 
{
	_targetAngle+=2.0f;
	if (_targetAngle > 360) {
		_targetAngle -= 360;
	}

	glutPostRedisplay(); //Tell GLUT that the display has changed
	
	glutTimerFunc(1, update, 0);		//Tell GLUT to call update again in 10 milliseconds

}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	windowWidth=1280;
	windowHeight=700;
	glutInitWindowSize(windowWidth, windowHeight);
	
	glutCreateWindow("Collect the Coins, By Anjali Shenoy");
	initRendering();
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(handleKeypress2);
	glutPassiveMotionFunc(checkMouseCoordinates);
	glutMouseFunc(handleMouseclick);
	glutReshapeFunc(handleResize);

	glutTimerFunc(1, update, 0); //Add a timer	
	glutMainLoop();
	
	return 0;
}









