#version 330

uniform sampler2D textureMap0;
uniform sampler2D textureMap1;

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

in vec4 ic; 
in vec4 n;
in vec4 l;
in vec4 v;
in vec4 l2;
uniform float laser_out;
uniform float background_draw;


in vec2 iTexCoord0;

void main(void) {
    //Znormalizowane interpolowane wektory
    vec4 ml = normalize(l);
    vec4 mn = normalize(n);
    vec4 mv = normalize(v);
    //Wektor odbity
    vec4 mr = reflect(-ml, mn);

    //Drugie �wiat�o
    vec4 ml2 = normalize(l2);

    //Wektor odbity
    vec4 mr2 = reflect(-ml2, mn);

    //Parametry powierzchni
    vec4 kd = texture(textureMap0, iTexCoord0);
    vec4 ks = texture(textureMap1, iTexCoord0);

    //Obliczenie modelu o�wietlenia
    float nl = clamp(dot(mn, ml), 0, 1)*0.4*laser_out;
    float rv = pow(clamp(dot(mr, mv), 0, 1), 25)*0.4*laser_out;

    float nl2 = clamp(dot(mn, ml2), 0, 1);
    float rv2 = pow(clamp(dot(mr2, mv), 0, 1), 20);

    pixelColor = vec4(kd.rgb * (nl2*vec3(1,1,1) + vec3(1, 1, 0)*nl), kd.a) + vec4(ks.rgb*(rv+rv2), 0);
    if (background_draw == 1.0){
        pixelColor = texture(textureMap0, iTexCoord0);
    }
}