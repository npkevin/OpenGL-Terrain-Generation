#include <stdio.h>
#include <iostream>
#include <math.h>
#include <gl/glut.h>

#include "QuadMesh.c"

#define DEG2RAD 3.14159f/180.0f

void initOpenGL(int, int);
void displayHandler(void);
void reshapeHandler(int, int);
void idleHandler(void);
void mouseButtonHandler(int, int, int, int);
void mouseMotionHandler(int, int);
void keyboardInputHandler(unsigned char, int, int);
void insertBlob(int, int);

int vWidth = 1000;
int vHeight = 800;

// Terrain
static QuadMesh terrain;
const int meshSize = 32; // meshSize x meshSize (quads)
const int meshWidth = 32;
const int meshLength = 32;

static GLfloat light_position[] = { 100.0F, 100.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

static GLfloat surface_ambient[] = { 0.3F, 0.5F, 0.3F, 1.0F };
static GLfloat surface_specular[] = { 0.1F, 0.35F, 0.1F, 0.5F };
static GLfloat surface_diffuse[] = { 0.1F, 0.2F, 0.1F, 1.0F };

// Other
int mainWindowID;
bool mmDown = false;

// Camera
int netDiffX = 0; // degrees for yaw
int netDiffY = 0; // degrees for pitch
int startX, startY, endX, endY, currDiffX, currDiffY;
float yaw = 0;
float pitch = 0;
float sensitivity = 1;
float cameraRadius = 30.0;


int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - vWidth) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - vHeight) / 2);
	mainWindowID = glutCreateWindow("A2 - 500627132, Kevin Nguyen");
	initOpenGL(vWidth, vHeight);

	// Callbacks
	glutDisplayFunc(displayHandler);
	glutReshapeFunc(reshapeHandler);
	glutIdleFunc(idleHandler);
	glutMouseFunc(mouseButtonHandler);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboardInputHandler);

	// Enter Main loop
	glutMainLoop();
	return 0;
}

void initOpenGL(int w, int h) {
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_FLAT);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.7F, 0.7F, 0.7F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	// Set up ground quad mesh
	Vector3D origin = NewVector3D(0.0f, 0.0f, 0.0f);
	Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	terrain = NewQuadMesh(meshSize);
	InitMeshQM(&terrain, meshSize, origin, meshLength, meshWidth, dir1v, dir2v);

	Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
	Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
	Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
	SetMaterialQM(&terrain, ambient, diffuse, specular, 0.2);

}

void reshapeHandler(int w, int h) {
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLdouble)w / h, 0.2, 300.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		// Camera rotations
		cameraRadius * cos(pitch) * sin(yaw),
		cameraRadius * sin(pitch),
		cameraRadius * cos(pitch) * cos(yaw),
		0, 0, 0, // LookAt
		0.0, 1.0, 0 // up vector
	);
}

void displayHandler(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glMaterialfv(GL_FRONT, GL_AMBIENT, surface_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, surface_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, surface_diffuse);

	glPushMatrix();
		// center mesh at origin
	glTranslatef(-meshWidth/2, 0, meshLength/2);
		// Draw ground mesh
		DrawMeshQM(&terrain, meshSize);
	glPopMatrix();

	glLoadIdentity();
	gluLookAt(
		// Camera rotations
		cameraRadius * cos(pitch) * sin(yaw),
		cameraRadius * sin(pitch),
		cameraRadius * cos(pitch) * cos(yaw),
		0, 0, 0, // LookAt
		0.0, 1.0, 0 // up vector
	);
	glutSwapBuffers();
}

void idleHandler(void) {
}

// state:0 == keyDown
void mouseButtonHandler(int button, int state, int x, int y) {
	
	// middle mouse (camera rotation)
	if (button == 1) {
		if (state == 0) { 
			mmDown = true;
			startX = x;
			startY = y;
		}
		else {
			mmDown = false;
			netDiffX += currDiffX;
			netDiffY += currDiffY;
			currDiffX = 0;
			currDiffY = 0;
		}
	}
	if (button == 3 && state == 0) cameraRadius -= 0.5;
	if (button == 4 && state == 0) cameraRadius += 0.5;
	
	glutPostRedisplay();
}

void mouseMotionHandler(int x, int y) {

	if (mmDown) {
		currDiffX = startX - x;
		currDiffY = startY - y;

		pitch = -(netDiffY + currDiffY) * DEG2RAD;
		yaw = (netDiffX + currDiffX) * DEG2RAD;
		//printf("Yaw: %f\nPitch: %f\n\n", yaw, pitch);
	}

	glutPostRedisplay();
}

void keyboardInputHandler(unsigned char key, int x, int y) {
	if (key == 27) {
		glutDestroyWindow(mainWindowID);
	}
	else {
		insertBlob(1, 2);
		glutPostRedisplay();
	}
	
	
}

void insertBlob(int x, int z) {
	terrain.vertices[0].position.y += 0.5;
	printf("X:%f Y:%f Z%f: \n", terrain.vertices[0].normal.x, terrain.vertices[0].normal.y, terrain.vertices[0].normal.z);

	//terrain.quads[31].vertices[0]->position.y += 0.5;
	//terrain.quads[31].vertices[1]->position.y += 0.5;
	//terrain.quads[31].vertices[2]->position.y += 0.5;
	//terrain.quads[31].vertices[3]->position.y += 0.5;
}