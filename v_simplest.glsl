#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform float shipX;
uniform float shipY;

uniform float laser_x;
uniform float laser_y;


//Atrybuty
in vec4 vertex; //wspolrzedne wierzcholka w przestrzeni modelu
in vec2 texCoord0; //kolor zwi¹zany z wierzcho³kiem
in vec4 normal; //wektor normalny w przestrzeni modelu

//Zmienne interpolowane
out vec2 iTexCoord0;
out vec4 l;
out vec4 n;
out vec4 v;
out vec4 l2;

void main(void) {
    vec4 lp = vec4(laser_x, laser_y, 0, 1); //pozcyja œwiat³a, przestrzeñ œwiata
    l = normalize(V * lp - V*M*vertex); //wektor do œwiat³a w przestrzeni oka
    v = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //wektor do obserwatora w przestrzeni oka
    n = normalize(V * M * normal); //wektor normalny w przestrzeni oka

    vec4 lp2 = vec4(shipX, shipY, -1, 1); //pozcyja œwiat³a, przestrzeñ œwiata
    l2 = normalize(V * lp2 - V*M*vertex); //wektor do œwiat³a w przestrzeni oka
    
    iTexCoord0 = texCoord0;
    
    gl_Position=P*V*M*vertex;
}
