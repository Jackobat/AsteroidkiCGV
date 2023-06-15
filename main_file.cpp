#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#define _CRT_SECURE_NO_WARNINGS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include <iostream>


#include <random>
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"


ShaderProgram *sp;
GLuint tex0;

typedef struct model {
	float*		verts;
	float*		tex_coords;
	float*		normals;
	int			count;
	GLuint		color_tex;
	GLuint		spec_tex;
}model;

typedef struct astro{
	float		x;
	float		y;
	float		vx;
	float		vy;
	float		angle;
	glm::vec3	rot_vec;
	float		rot_speed;
	float		scale;
	float		speed;
}astro;

float aspectRatio = 1;
float dt = 0.0;
int score = 0;

//Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;

//Mouse
float lastX = 400.0f, lastY = 0.0f;
bool firstMouse = true;

//Ship
float ship_thrusters = 0.0f;
float ship_speed = 3.7f;
float ship_x = 0.0f;
float ship_y = 0.0f;
float ship_angle = 0.0f;
float ship_rot_speed = 0;
float ship_velocity_x = 0.0f;
float ship_velocity_y = 0.2f;
float ship_acceleration = 5.0f;

//Laser
bool laser_out = false;
float laser_speed = 9.0f;
float laser_x = 0.0f;
float laser_y = 0.0f;
float laser_vx = 0.0f;
float laser_vy = 0.0f;
float laser_angle = 0.0f;

//Models
model ship;
model plane;
model asteroid;
model laser;
model score0;
model score1;
model score2;
model score3;
model score4;
model score5;
model score6;
model score7;
model score8;
model score9;
model score10;

//Asteroids positions
astro* asteroids;
int astro_len = 10;

//Plane arrays
float plane_verts[] = {
		1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 1.0f
};
float plane_tex_coords[] = {
	1.0f,1.0f, 0.0f,0.0f, 0.0f,1.0f,
	1.0f,1.0f, 1.0f,0.0f, 0.0f,0.0f
};
float plane_normals[] = {
	0.0f, 0.0f,-1.0f,0.0f,
	0.0f, 0.0f,-1.0f,0.0f,
	0.0f, 0.0f,-1.0f,0.0f,
	0.0f, 0.0f,-1.0f,0.0f,
	0.0f, 0.0f,-1.0f,0.0f,
	0.0f, 0.0f,-1.0f,0.0f
};
int plane_vertex_count = 6;

float random_float(float mn, float mx);
int random_sign();
astro new_astroid(float ax, float ay, float scale, float speed);
void loadMesh(fastObjMesh* mesh, int* sz, float** vertices, float** tex_crds, float** normls, int init_alloc);
GLuint readTexture(const char* filename);
model newModel(const char mesh_path[], const char color_tex_path[], const char spec_tex_path[]);

void drawModel(model m, glm::mat4 M);

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void keyCallback(GLFWwindow* window,int key,int scancode,int action,int mods) {
    if (action==GLFW_PRESS) {
        if (key==GLFW_KEY_LEFT) ship_rot_speed = 2;
        if (key==GLFW_KEY_RIGHT) ship_rot_speed = -2;
		if (key == GLFW_KEY_UP) ship_thrusters = 1.0f;
        if (key==GLFW_KEY_DOWN) ship_thrusters = -0.2f;
    }
    if (action==GLFW_RELEASE) {
        if (key==GLFW_KEY_LEFT) ship_rot_speed = 0;
        if (key==GLFW_KEY_RIGHT) ship_rot_speed = 0;
        if (key==GLFW_KEY_UP && ship_thrusters > 0) ship_thrusters = 0.0f;
        if (key==GLFW_KEY_DOWN && ship_thrusters < 0) ship_thrusters = 0.0f;
		if (key == GLFW_KEY_SPACE && !laser_out) { 
			laser_x = ship_x;
			laser_y = ship_y;
			laser_vx = sinf(ship_angle) * laser_speed;
			laser_vy = cosf(ship_angle) * laser_speed;
			laser_angle = ship_angle;
			laser_out = true;
		}
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	std::cout << "lastX: " << lastX << "  lastY: " << lastY << std::endl;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	ship_rot_speed += glm::radians(xoffset);

	pitch += yoffset;
	ship_thrusters += yoffset/100;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	//std::cout << "Front x: " << cameraFront.x << "  y: " << cameraFront.y << "  z: " << cameraFront.z << std::endl;
}

void windowResizeCallback(GLFWwindow* window,int width,int height) {
    if (height==0) return;
    aspectRatio=(float)width/(float)height;
    glViewport(0,0,width,height);
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0,0,0,1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	sp=new ShaderProgram("v_simplest.glsl",NULL,"f_simplest.glsl");

	//Załaduj obiekty
	ship = newModel("viper.obj", "viper.png", "ship_specular.png");
	laser = newModel("rocket.obj", "laser_color.png", "asteroid_spec.png");   //  newModel("rocket.obj", "rocket.png", "rocket_spec.png");
	asteroid =  newModel("Cliff_Rock_One_OBJ.obj", "Cliff_Rock_One_BaseColor.png", "Cliff_Rock_One_Normal.png");
	score0 = newModel("0.obj", "white.png", "white.png");
	score1 = newModel("1.obj", "white.png", "white.png");
	score2 = newModel("2.obj", "white.png", "white.png");
	score3 = newModel("3.obj", "white.png", "white.png");
	score4 = newModel("4.obj", "white.png", "white.png");
	score5 = newModel("5.obj", "white.png", "white.png");
	score6 = newModel("6.obj", "white.png", "white.png");
	score7 = newModel("7.obj", "white.png", "white.png");
	score8 = newModel("8.obj", "white.png", "white.png");
	score9 = newModel("9.obj", "white.png", "white.png");
	score10 = newModel("10.obj", "white.png", "white.png");

	//Init plane for background
	plane.verts = plane_verts;
	plane.tex_coords = plane_tex_coords;
	plane.normals = plane_normals;
	plane.count = 6;
	plane.color_tex = readTexture("space.png");
	plane.spec_tex = plane.color_tex;
	//laser.verts = asteroid.verts; //Yes, I was that lazy
	//laser.tex_coords = asteroid.tex_coords;
	//laser.normals = asteroid.normals;
	//laser.count = asteroid.count;
	//laser.color_tex = readTexture("laser_color.png");
	//laser.spec_tex = laser.color_tex;
	
	//Generate initial astroids...
	asteroids = (astro*)malloc(astro_len * sizeof(astro));
	for (int i = 0; i < astro_len/2; i++) {
		float ax = random_float(2,4);
		float ay = random_float(2,4);		
		int sx = random_sign();
		int sy = random_sign();

		asteroids[i] = new_astroid(ax * sx, ay * sy, 1, 0);
	}
}

astro new_astroid(float ax, float ay, float scale,  float speed) {
	astro a;
	float r = 6.0f;
	float angle = random_float(0, 2 * PI);
	a.x = ax;
	a.y = ay;
	angle += PI + random_float(-0.2, 0.2);
	a.vx = sin(angle) * speed;
	a.vy = cos(angle) * speed;
	a.angle = 0.0f;
	a.rot_vec = glm::vec3(random_float(-1, 1), random_float(-1, 1), random_float(-1, 1));
	a.rot_speed = random_float(0.5f, 1.1f);
	a.scale = scale;
	return a;
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    delete sp;
}

//Czytanie tekstury
GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytam y wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

void drawScene(GLFWwindow* window) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 V=glm::lookAt(
         glm::vec3(0, 0, -13),
         glm::vec3(0,0,0),
         glm::vec3(0.0f,1.0f,0.0f)); //Wylicz macierz widoku

    glm::mat4 P=glm::perspective(50.0f*PI/180.0f, aspectRatio, 0.01f, 50.0f); //Wylicz macierz rzutowania

    glm::mat4 M=glm::mat4(1.0f);

	glm::mat4 bkg_M = glm::translate(M, glm::vec3(0.0f, 0.0f, 9.0f));
	bkg_M = glm::scale(bkg_M, glm::vec3(10.0f, 10.0f, 1.0f));

	//Calculate ship's model matrix...
	glm::mat4 ship_M = glm::translate(M, glm::vec3(ship_x, ship_y, 0.0f));
	ship_M = glm::rotate(ship_M, -PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	ship_M = glm::rotate(ship_M, PI/2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	ship_M = glm::rotate(ship_M, ship_angle,glm::vec3(0.0f,1.0f,0.0f));
	ship_M = glm::scale(ship_M, glm::vec3(0.2f, 0.2f, 0.2f));

	//Calculate position of the score
	glm::mat4 score_M = glm::scale(M, glm::vec3(5.0f, 5.0f, -100.0f));
	score_M = glm::translate(M, glm::vec3(5.0f, 5.0f, 0.0f));
	score_M = glm::rotate(score_M, PI, glm::vec3(0.0f, 1.0f, 0.0f));

	sp->use();//Aktywacja programu cieniującego
	//Przeslij parametry programu cieniującego do karty graficznej
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
	glUniform1f(sp->u("shipX"), ship_x);
	glUniform1f(sp->u("shipY"), ship_y);
	glUniform1f(sp->u("laser_out"), laser_out ? 1.0f : 0.0f);
	glUniform1f(sp->u("laser_x"), laser_x);
	glUniform1f(sp->u("laser_y"), laser_y);

	glEnableVertexAttribArray(sp->a("normal"));
	glEnableVertexAttribArray(sp->a("vertex"));
	glEnableVertexAttribArray(sp->a("texCoord0"));
	
	glUniform1f(sp->u("background_draw"), 1.0);
	drawModel(plane, bkg_M);

	glUniform1f(sp->u("background_draw"), 0.0);
	drawModel(ship, ship_M);
	if (laser_out) {
		glm::mat4 laser_M = glm::translate(M, glm::vec3(laser_x, laser_y, 0.0f));
		laser_M = glm::rotate(laser_M, -laser_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		laser_M = glm::scale(laser_M, glm::vec3(0.0005f, 0.0005f, 0.0005f));
		drawModel(laser, laser_M);
	}

	switch (score) {
	case 0: drawModel(score0, score_M);
		break;
	case 1: drawModel(score1, score_M);
		break;
	case 2: drawModel(score2, score_M);
		break;
	case 3: drawModel(score3, score_M);
		break;
	case 4: drawModel(score4, score_M);
		break;
	case 5: drawModel(score5, score_M);
		break;
	case 6: drawModel(score6, score_M);
		break;
	case 7: drawModel(score7, score_M);
		break;
	case 8: drawModel(score8, score_M);
		break;
	case 9: drawModel(score9, score_M);
		break;
	case 10: drawModel(score10, score_M);
		break;
	}

	//Draw asteroids
	for (int i = 0; i < astro_len; i++) {
		glm::mat4 astro_M = glm::translate(M, glm::vec3(asteroids[i].x, asteroids[i].y, 0.0f));
		astro_M = glm::scale(astro_M, glm::vec3(0.005f, 0.005f, 0.005f));
		astro_M = glm::rotate(astro_M, asteroids[i].angle, asteroids[i].rot_vec);
		astro_M = glm::scale(astro_M, glm::vec3(asteroids[i].scale, asteroids[i].scale, asteroids[i].scale));
		drawModel(asteroid, astro_M);
	}
	glDisableVertexAttribArray(sp->a("vertex"));  //Wyłącz przesyłanie danych do atrybutu vertex
	glDisableVertexAttribArray(sp->a("texCoord0"));  //Wyłącz przesyłanie danych do atrybutu color
	glDisableVertexAttribArray(sp->a("normal"));  //Wyłącz przesyłanie danych do atrybutu normal

	glfwSwapBuffers(window); //Przerzuć tylny bufor na przedni
}

void drawObject(fastObjMesh* mesh, GLuint tex, glm::mat4 M){
	
}

float random_float(float min_v, float max_v){
	std::random_device rd; // obtain a random number from hardware
	std::mt19937 gen(rd()); // seed the generator
	std::uniform_real_distribution<> distr(min_v, max_v);
	return distr(gen);
}

int random_sign() {
	// Seed the random number generator
	std::random_device rd;
	std::mt19937 gen(rd());

	// Define the distribution to generate numbers in the range of [0, 1]
	std::uniform_int_distribution<> dis(0, 1);

	// Generate a random number and map it to 1 or -1
	int randomNumber = dis(gen) == 0 ? -1 : 1;

	return randomNumber;
}


int main(void){
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(1000, 1000, "Asteroids 3D", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla
	glfwSetTime(0); //Zeruj timer
	bool game_over = false;
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		dt = glfwGetTime();
		//Update ship
		ship_angle += ship_rot_speed * dt; //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		//Calculate drag (even tho there should be none...)
		ship_velocity_x -= (ship_velocity_x == 0 ? 0 : (ship_velocity_x < 0 ? -1 : 1)) * 0.005;
		ship_velocity_y -= (ship_velocity_y == 0 ? 0 : (ship_velocity_y < 0 ? -1 : 1)) * 0.005;

		ship_velocity_x += sinf(ship_angle) * dt * ship_speed * ship_thrusters;
		ship_velocity_y += cosf(ship_angle) * dt * ship_speed * ship_thrusters;
		ship_x += ship_velocity_x * dt;
		ship_y += ship_velocity_y * dt;

		if (ship_velocity_x * ship_velocity_x + ship_velocity_y * ship_velocity_y <= 0.002 * 0.002)
			ship_velocity_x = ship_velocity_y = 0.0f;

		for (int i = 0; i < astro_len; i++) {
			float dx = fabsf(asteroids[i].x - ship_x);
			float dy = fabsf(asteroids[i].y - ship_y);
			if (dx * dx + dy * dy <= 0.3) {
				laser_out = false;
				game_over = true;
				break;
			}
		}
		ship_x = fmax(-5, fmin(ship_x, 5));
		ship_y = fmax(-5, fmin(ship_y, 5));

		//Update astroids
		for (int i = 0; i < astro_len; i++) {
			asteroids[i].x += asteroids[i].vx*dt;
			asteroids[i].y += asteroids[i].vy*dt;
			asteroids[i].angle += asteroids[i].rot_speed * dt;
			if (fabsf(asteroids[i].x) > 8.0 || fabsf(asteroids[i].y) > 8.0) {
				asteroids[i] = new_astroid(-5,-5, 1, 0);
			}
		}
		//Update laser
		if (laser_out) {
			laser_x += laser_vx*dt;
			laser_y += laser_vy*dt;
			for (int i = 0; i < astro_len; i++) {
				float dx = fabsf(asteroids[i].x - laser_x);
				float dy = fabsf(asteroids[i].y - laser_y);
				if (dx * dx + dy * dy <= 0.11) {
					laser_out = false;
					if (asteroids[i].scale == 1) {
						asteroids[i] = new_astroid(asteroids[i].x, asteroids[i].y, 0.5, random_float(0.3, 1.3));
						asteroids[astro_len/2 + i] = new_astroid(asteroids[i].x, asteroids[i].y, 0.5, random_float(0.3, 1.3));
					}
					else if (asteroids[i].scale == 0.5) {
						asteroids[i] = new_astroid(10, 10, 0, 0);
					}
					score++;
					break;
				}
			}
			if (fabsf(laser_x) >= 8.0 || fabsf(laser_y) > 8.0) {
				laser_out = false;
			}
		}

        glfwSetTime(0); //Zeruj timer
		drawScene(window); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
		
		if (game_over) break;
	}

	printf("GAME OVER!\nScore: %d\n", score);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
 	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	
	
	freeOpenGLProgram(window);
	exit(EXIT_SUCCESS);
}

//This loads given mesh into arrays that are then ready to be drawn with glDrawArrays!
void loadMesh(fastObjMesh* mesh, int* sz, float** vertices, float** tex_crds, float** normls, int init_alloc){
	int index = 0;
	int len = 0;
	int coords_len = 0;
	int cap = init_alloc;
	float* verts = (float*)malloc(4 * cap * sizeof(float));
	float* nrmls = (float*)malloc(4 * cap * sizeof(float));
	float* tex_cords = (float*)malloc(4 * cap * sizeof(float));
	for (int i = 0; i < mesh->face_count; i++) {
		for (int j = 0; j < mesh->face_vertices[i] - 2; j++) {
			for (int k = 0; k < 3; k++) {
				verts[len] = mesh->positions[(int)(mesh->indices[index].p) * 3 + k];
				nrmls[len++] = mesh->normals[(int)(mesh->indices[index].n) * 3 + k];
			}
			for (int k = 0; k < 2; k++) {
				tex_cords[coords_len++] = mesh->texcoords[(int)(mesh->indices[index].t) * 2 + k];
			}
			verts[len] = 1.0;
			nrmls[len++] = 0.0;
			if (len % (cap * 4) == 0) {
				verts = (float*)realloc(verts, (len + cap * 4) * sizeof(float));
				nrmls = (float*)realloc(nrmls, (len + cap * 4) * sizeof(float));
				tex_cords = (float*)realloc(tex_cords, (len + cap * 4) * sizeof(float));
			}
			for (int l = 1; l < 3; l++) {
				for (int k = 0; k < 3; k++) {
					verts[len] = mesh->positions[(int)(mesh->indices[index + j + l].p) * 3 + k];
					nrmls[len++] = mesh->normals[(int)(mesh->indices[index + j + l].n) * 3 + k];
				}
				for (int k = 0; k < 2; k++) {
					tex_cords[coords_len++] = mesh->texcoords[(int)(mesh->indices[index + j + l].t) * 2 + k];
				}
				verts[len] = 1.0;
				nrmls[len++] = 0.0;
				if (len % (cap * 4) == 0) {
					verts = (float*)realloc(verts, (len + cap * 4) * sizeof(float));
					nrmls = (float*)realloc(nrmls, (len + cap * 4) * sizeof(float));
					tex_cords = (float*)realloc(tex_cords, (len + cap * 4) * sizeof(float));
				}
			}
		}
		index += (int)(mesh->face_vertices[i]);
	}
	*sz = len;
	verts = (float*)realloc(verts, 4 * len * sizeof(float));
	nrmls = (float*)realloc(nrmls, 4 * len * sizeof(float));
	tex_cords = (float*)realloc(tex_cords, 2 * len * sizeof(float));
	*vertices = verts;
	*tex_crds = tex_cords;
	*normls = nrmls;
}

model newModel(const char mesh_path[], const char color_tex_path[], const char spec_tex_path[]) {
	model m;
	fastObjMesh* mesh = fast_obj_read(mesh_path);
	loadMesh(mesh, &(m.count), &(m.verts), &(m.tex_coords), &(m.normals), 512);
	m.color_tex = readTexture(color_tex_path);
	m.spec_tex = readTexture(spec_tex_path);
	return m;
}

void drawModel(model m, glm::mat4 M) {
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, m.verts); //Wskaż tablicę z danymi dla atrybutu vertex
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, m.tex_coords); //Wskaż tablicę z danymi dla atrybutu color
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, m.normals); //Wskaż tablicę z danymi dla atrybutu normal

	glUniform1i(sp->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m.color_tex);
	glUniform1i(sp->u("textureMap1"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m.spec_tex);
	glDrawArrays(GL_TRIANGLES, 0, m.count);
}