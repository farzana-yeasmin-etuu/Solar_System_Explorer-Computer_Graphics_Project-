// solar_neon_tunnel_fixed.cpp
// Fixed & final: Solar System Explorer + Neon Tunnel Runner (HQ)
// Fixes: duplicate case errors, narrowing warnings, animation menu mapping
// Compile: g++ solar_neon_tunnel_fixed.cpp -o Program -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32

#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>

#define PI 3.14159265358979323846f

// ---------- Screen states ----------
enum Screen {
    MAIN_MENU,
    ANIMATION_MENU,
    ANIMATION_VIEW,
    PLANET_MENU,
    PLANET_DETAIL,
    GAME_MODE,
    GAME_OVER
};
Screen currentScreen = MAIN_MENU;

// ---------- Window ----------
int windowWidth = 1200, windowHeight = 750;

// ---------- Camera for scene (spherical) ----------
float camRadius = 40.0f;
float camTheta = 1.2f;
float camPhi = 0.7f;

// ---------- Animation time ----------
float globalTime = 0.0f;
int lastTimeMs = 0;
float animTime = 0.0f;

// ---------- Planets ----------
struct Planet {
    std::string name;
    float orbitRadius;
    float size;
    float orbitSpeed;
    float rotation;
    float rotationSpeed;
    std::string info;
};
std::vector<Planet> planets;
int earthIndex = 3;
int selectedPlanet = -1;

// ---------- Stars ----------
std::vector<float> stars;

// ---------- Utility ----------
float deg2rad(float d) { return d * PI / 180.0f; }
float clampf(float v, float a, float b) { if (v<a) return a; if (v>b) return b; return v; }

void drawText2D(float x, float y, const std::string &text) {
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}
void drawTextSmall(int x, int y, const std::string &text) {
    glRasterPos2i(x, y);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
}

// ---------- Init planets & stars ----------
void initPlanets() {
    planets.clear();
    planets.push_back({"Sun", 0.0f, 3.2f, 0.0f, 0.0f, 0.0f, "The Sun: center of solar system."});
    planets.push_back({"Mercury", 6.0f, 0.28f, 4.8f, 0.0f, 0.6f, "Mercury: smallest planet."});
    planets.push_back({"Venus", 8.5f, 0.6f, 3.5f, 0.0f, 0.3f, "Venus: thick atmosphere, very hot."});
    planets.push_back({"Earth", 11.0f, 0.65f, 2.5f, 0.0f, 1.2f, "Earth: our home planet."});
    planets.push_back({"Moon", 11.8f, 0.16f, 8.0f, 0.0f, 0.8f, "Moon: Earth's natural satellite."});
    planets.push_back({"Mars", 14.5f, 0.35f, 1.9f, 0.0f, 0.9f, "Mars: the Red Planet."});
    planets.push_back({"Jupiter", 19.0f, 1.4f, 1.0f, 0.0f, 0.6f, "Jupiter: gas giant."});
    planets.push_back({"Saturn", 24.0f, 1.1f, 0.7f, 0.0f, 0.5f, "Saturn: famous rings."});
    planets.push_back({"Uranus", 28.0f, 0.9f, 0.5f, 0.0f, 0.4f, "Uranus: tilted axis."});
    planets.push_back({"Neptune", 31.5f, 0.85f, 0.4f, 0.0f, 0.45f, "Neptune: distant ice giant."});
}

void initStars(int count=400) {
    stars.clear();
    for (int i=0;i<count;i++){
        float x = (rand()%2000 - 1000) / 12.0f;
        float y = (rand()%2000 - 1000) / 12.0f;
        float z = (rand()%2000 - 1000) / 12.0f;
        stars.push_back(x); stars.push_back(y); stars.push_back(z);
    }
}

// ---------- Drawing helpers ----------
void drawPlanetSphere(float size, float r, float g, float b) {
    glColor3f(r,g,b);
    glutSolidSphere(size, 36, 36);
}

void drawOrbit(float radius) {
    glBegin(GL_LINE_LOOP);
    for (int i=0;i<360;i+=4){
        float a = deg2rad(i);
        glVertex3f(cos(a)*radius, 0.0f, sin(a)*radius);
    }
    glEnd();
}

// ---------- Draw stars ----------
void drawStars() {
    glDisable(GL_LIGHTING);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glColor3f(1,1,1);
    for (size_t i=0;i<stars.size(); i+=3) {
        glVertex3f(stars[i], stars[i+1], stars[i+2]);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

// ---------- Solar system scene ----------
void drawSolarSystemScene(bool showOrbits=true) {
    // Sun
    glPushMatrix();
        glDisable(GL_LIGHTING);
        glColor3f(1.0f,0.95f,0.2f);
        glutSolidSphere(planets[0].size, 48, 48);
        glEnable(GL_LIGHTING);
    glPopMatrix();

    for (size_t i=1;i<planets.size();++i){
        Planet &p = planets[i];
        float ang = animTime * p.orbitSpeed;
        float x = cosf(deg2rad(ang)) * p.orbitRadius;
        float z = sinf(deg2rad(ang)) * p.orbitRadius;
        if (showOrbits) {
            glColor3f(0.28f,0.28f,0.36f);
            drawOrbit(p.orbitRadius);
        }
        glPushMatrix();
            glTranslatef(x,0.0f,z);
            glRotatef(p.rotation, 0,1,0);
            if (p.name=="Mercury") drawPlanetSphere(p.size, 0.6f,0.6f,0.6f);
            else if (p.name=="Venus") drawPlanetSphere(p.size, 0.93f,0.7f,0.35f);
            else if (p.name=="Earth") drawPlanetSphere(p.size, 0.2f,0.5f,1.0f);
            else if (p.name=="Moon") drawPlanetSphere(p.size, 0.85f,0.85f,0.85f);
            else if (p.name=="Mars") drawPlanetSphere(p.size, 1.0f,0.35f,0.25f);
            else if (p.name=="Jupiter") drawPlanetSphere(p.size, 0.9f,0.7f,0.5f);
            else if (p.name=="Saturn") {
                drawPlanetSphere(p.size, 0.9f,0.85f,0.6f);
                glColor3f(0.85f,0.75f,0.55f);
                glPushMatrix(); glRotatef(90,1,0,0); drawOrbit(p.size*1.6f); glPopMatrix();
            } else if (p.name=="Uranus") drawPlanetSphere(p.size, 0.6f,0.8f,0.9f);
            else if (p.name=="Neptune") drawPlanetSphere(p.size, 0.25f,0.45f,0.95f);
        glPopMatrix();
    }

    // Moon relative to Earth
    Planet &earth = planets[earthIndex];
    float eang = animTime * earth.orbitSpeed;
    float ex = cosf(deg2rad(eang))*earth.orbitRadius;
    float ez = sinf(deg2rad(eang))*earth.orbitRadius;
    Planet &moon = planets[4];
    float mang = animTime * moon.orbitSpeed;
    float mx = ex + cosf(deg2rad(mang)) * 0.8f;
    float mz = ez + sinf(deg2rad(mang)) * 0.8f;
    glPushMatrix();
        glTranslatef(mx, 0.0f, mz);
        drawPlanetSphere(moon.size, 0.85f, 0.85f, 0.85f);
    glPopMatrix();
}

// ---------- Neon Tunnel Game ----------

// simple vector
struct Vec3 { float x,y,z; };
Vec3 vadd(const Vec3&a,const Vec3&b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Vec3 vsub(const Vec3&a,const Vec3&b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec3 vmul(const Vec3&a,float s){return {a.x*s,a.y*s,a.z*s};}
float vlen(const Vec3&a){return sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);}

struct Player {
    Vec3 pos;
    float yaw;   // degrees
    float pitch; // degrees
    Vec3 vel;
    int lives;
} player;

struct Asteroid {
    Vec3 pos;
    Vec3 vel;
    float size;
    bool active;
};
std::vector<Asteroid> asteroids;

// Game params
const float TUNNEL_LENGTH = 400.0f;
const float TUNNEL_RADIUS = 8.0f;
const int MAX_AST = 60;
float gameTimeLeft = 45.0f;
bool gameRunning = false;
bool gameWon = false;
bool gameLost = false;

// neon tunnel parameters for HQ look
float tunnelScroll = 0.0f;

// spawn asteroid ahead of player within tunnel bounds
void spawnAsteroid() {
    if ((int)asteroids.size() >= MAX_AST) return;
    float angle = (rand()%360) * PI/180.0f;
    float r = ((rand()%100)/100.0f) * (TUNNEL_RADIUS - 1.0f);
    float x = cosf(angle) * r;
    float y = (rand()%100)/100.0f * (TUNNEL_RADIUS*0.8f) - (TUNNEL_RADIUS*0.4f);
    float z = player.pos.z - 120.0f - (rand()%200); // spawn ahead in negative z direction (player moves negative z)
    Vec3 pos = {x, y, z};
    Vec3 dir = { (rand()%200 - 100)/200.0f, (rand()%200 - 100)/200.0f, 1.0f }; // moving towards +z (towards player)
    float speed = 18.0f + (rand()%100)/5.0f;
    Asteroid a; a.pos = pos; a.vel = {dir.x*speed, dir.y*speed, dir.z*speed}; a.size = 0.7f + (rand()%100)/150.0f; a.active = true;
    asteroids.push_back(a);
}

// reset game
void startGame() {
    asteroids.clear();
    player.pos = {0.0f, 0.0f, 60.0f}; // start ahead
    player.yaw = 0.0f; player.pitch = 0.0f;
    player.vel = {0,0,-30.0f}; // forward velocity negative z
    player.lives = 3;
    gameTimeLeft = 45.0f;
    gameRunning = true; gameWon = false; gameLost = false;
    tunnelScroll = 0.0f;
    // spawn some initial asteroids
    for (int i=0;i<18;i++) spawnAsteroid();
}

// draw glowing neon tunnel (Tron-style)
void drawNeonTunnel() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // central glowing grid lines moving towards player (z increasing)
    int rings = 80;
    for (int i=0;i<rings;i++){
        float t = (i*6.0f + fmodf(tunnelScroll, 6.0f));
        float z = -t + player.pos.z;
        if (z > player.pos.z + 10.0f || z < player.pos.z - TUNNEL_LENGTH) continue;
        float alpha = 0.05f + 0.8f * (1.0f - fabsf((z - player.pos.z)) / 150.0f);
        alpha = clampf(alpha, 0.02f, 0.85f);
        float r = 0.05f + 0.5f * (0.5f + 0.5f * sinf((i+globalTime)*0.12f));
        float g = 0.6f + 0.4f * (0.5f + 0.5f * cosf((i+globalTime)*0.1f));
        float b = 0.9f;
        glColor4f(r, g, b, alpha*0.6f);

        glBegin(GL_LINE_LOOP);
        int seg = 64;
        for (int s=0;s<seg;s++){
            float a = 2.0f*PI*s / seg;
            float rad = TUNNEL_RADIUS * (0.6f + 0.4f * sinf(globalTime*0.05f + i*0.06f));
            float x = cosf(a) * rad;
            float y = sinf(a) * rad * 0.6f;
            glVertex3f(x, y, z);
        }
        glEnd();
    }

    // neon rails (two long lines at left/right)
    glLineWidth(4.0f);
    for (int side=-1; side<=1; side+=2){
        for (int k=0;k<3;k++){
            float offset = 1.2f + k*0.3f;
            float alpha = 0.15f + 0.25f * (1.0f - k*0.2f);
            glColor4f(0.1f, 0.8f, 1.0f, alpha);
            glBegin(GL_LINE_STRIP);
            for (float z=-TUNNEL_LENGTH + fmodf(tunnelScroll*2.0f, 8.0f); z<=player.pos.z+30.0f; z+=3.0f){
                float y = 0.0f;
                float x = side * (TUNNEL_RADIUS - offset);
                glVertex3f(x, y, z + player.pos.z - 60.0f);
            }
            glEnd();
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// draw HUD (neon sci-fi)
void drawNeonHUD() {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    // outer translucent panel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float x0 = 20, y0 = windowHeight - 20;
    float w = 360, h = 120;
    glColor4f(0.02f, 0.06f, 0.08f, 0.5f);
    glBegin(GL_QUADS);
        glVertex2f(x0-8, y0+8);
        glVertex2f(x0+w+8, y0+8);
        glVertex2f(x0+w+8, y0-h-8);
        glVertex2f(x0-8, y0-h-8);
    glEnd();

    // neon border glow
    glLineWidth(2.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int i=0;i<3;i++){
        float a = 0.15f + 0.18f*(3-i);
        glColor4f(0.0f, 0.8f, 1.0f, a);
        glBegin(GL_LINE_LOOP);
            glVertex2f(x0, y0);
            glVertex2f(x0+w, y0);
            glVertex2f(x0+w, y0-h);
            glVertex2f(x0, y0-h);
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(0.85f, 0.95f, 1.0f);
    std::ostringstream ss;
    ss << "TIME LEFT: " << (int)ceil(gameTimeLeft) << " s";
    drawText2D(x0+18, y0-28, ss.str());
    ss.str(""); ss.clear();
    ss << "LIVES: ";
    for (int i=0;i<player.lives;i++) ss << "♥ ";
    drawText2D(x0+18, y0-58, ss.str());
    ss.str(""); ss.clear();
    ss << "SPEED: " << (int)(vlen(player.vel)) << " units/s";
    drawText2D(x0+18, y0-88, ss.str());

    // center crosshair
    float cx = windowWidth*0.5f, cy = windowHeight*0.5f;
    glLineWidth(2.0f);
    glColor4f(0.05f,0.9f,1.0f,0.85f);
    glBegin(GL_LINES);
        glVertex2f(cx-40, cy); glVertex2f(cx-12, cy);
        glVertex2f(cx+40, cy); glVertex2f(cx+12, cy);
        glVertex2f(cx, cy-40); glVertex2f(cx, cy-12);
        glVertex2f(cx, cy+40); glVertex2f(cx, cy+12);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// draw asteroid
void drawAsteroid(const Asteroid &a) {
    glColor3f(0.6f, 0.5f, 0.45f);
    glPushMatrix();
    glTranslatef(a.pos.x, a.pos.y, a.pos.z);
    glutSolidSphere(a.size, 16, 12);
    glPopMatrix();
}

// collision detection
bool checkCollision(const Player &p, const Asteroid &a) {
    float dx = p.pos.x - a.pos.x;
    float dy = p.pos.y - a.pos.y;
    float dz = p.pos.z - a.pos.z;
    float d2 = dx*dx + dy*dy + dz*dz;
    float rsum = 1.0f + a.size; // approximate ship radius ~1.0
    return d2 <= rsum*rsum;
}

// ---------- Input & control flags ----------
bool keyW=false,keyS=false,keyA=false,keyD=false,keyQ=false,keyE=false,keyBoost=false;
bool arrowLeft=false, arrowRight=false, arrowUp=false, arrowDown=false;

// ---------- Start and update functions ----------
void spawnInitialAsteroids(int n=18) {
    for (int i=0;i<n;i++) spawnAsteroid();
}

void updateScene(int value); // forward

// ---------- Drawing HUD / Scene / Menus ----------

void drawMainMenuScreen() {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    glColor3f(0.0f, 0.95f, 1.0f);
    drawText2D(windowWidth*0.28f, windowHeight*0.78f, "SOLAR SYSTEM EXPLORER (3D) - Neon Tunnel Runner (HQ)");
    glColor3f(1,1,1);
    drawText2D(windowWidth*0.36f, windowHeight*0.60f, "1. Start Animation");
    drawText2D(windowWidth*0.36f, windowHeight*0.52f, "2. Planet Information");
    drawText2D(windowWidth*0.36f, windowHeight*0.44f, "3. Play Orbit Runner (Neon Tunnel)");
    drawText2D(windowWidth*0.36f, windowHeight*0.36f, "4. Exit (ESC)");
    drawTextSmall(20,24,"Controls in game: W/S A/D Q/E move, Arrows yaw/pitch, Space boost, M main menu");

    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawAnimationMenuScreen() {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    drawText2D(40, windowHeight-40, "Animation Menu: Choose a planet to view animation (press digit). Press B to go back.");
    float baseY = windowHeight - 80;
    for (size_t i=0;i<planets.size();++i){
        std::ostringstream ss; ss << i << ". " << planets[i].name;
        drawText2D(60, baseY - i*26, ss.str());
    }

    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawPlanetDetailUI(int pid) {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    float x0 = windowWidth - 520;
    float y0 = windowHeight - 60;
    drawText2D(x0, y0, ("Name: " + planets[pid].name));
    drawText2D(x0, y0-28, ("Orbit radius (scaled): " + std::to_string(planets[pid].orbitRadius)));
    drawText2D(x0, y0-56, ("Size (scaled): " + std::to_string(planets[pid].size)));
    drawText2D(x0, y0-84, "Info:");
    std::istringstream iss(planets[pid].info);
    std::string token; int line=0;
    while (std::getline(iss, token, '.')) {
        if (token.size()>0) drawText2D(x0+12, y0-112-line*20, token + ".");
        line++;
    }

    drawText2D(40, 40, "Press B to go back, M for main menu");
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawSceneAnimationView(int pid) {
    float camX = camRadius * sinf(camPhi) * cosf(camTheta);
    float camY = camRadius * cosf(camPhi);
    float camZ = camRadius * sinf(camPhi) * sinf(camTheta);
    gluLookAt(camX, camY, camZ, 0,0,0, 0,1,0);

    // lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat ambient[] = {0.06f,0.06f,0.08f,1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    GLfloat lp[] = {50.0f, 50.0f, 50.0f, 1.0f};
    GLfloat lc[] = {0.95f,0.95f,0.9f,1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lp);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lc);

    drawStars();
    drawSolarSystemScene(true);

    // highlight chosen planet
    if (pid >= 0 && pid < (int)planets.size()) {
        Planet &p = planets[pid];
        float pang = animTime * p.orbitSpeed;
        float px = (p.orbitRadius==0.0f) ? 0.0f : cosf(deg2rad(pang))*p.orbitRadius;
        float pz = (p.orbitRadius==0.0f) ? 0.0f : sinf(deg2rad(pang))*p.orbitRadius;
        glDisable(GL_LIGHTING);
        glColor3f(1,0.9f,0.3f);
        glPushMatrix(); glTranslatef(px, 0.0f, pz); glutWireSphere(p.size*1.6f, 12, 12); glPopMatrix();
        glEnable(GL_LIGHTING);
    }

    // draw UI
    drawPlanetDetailUI(pid);
}

void drawGameScene() {
    // camera: chase behind player
    float yaw = player.yaw * PI/180.0f;
    float pitch = player.pitch * PI/180.0f;
    Vec3 forward = {-sinf(yaw)*cosf(pitch), sinf(pitch), -cosf(yaw)*cosf(pitch)};
    Vec3 camPos = {player.pos.x - forward.x*12.0f, player.pos.y + 5.0f, player.pos.z - forward.z*12.0f};
    gluLookAt(camPos.x, camPos.y, camPos.z, player.pos.x, player.pos.y, player.pos.z, 0,1,0);

    // lighting: dim ambient with neon emphasis
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat ambient[] = {0.02f,0.04f,0.06f,1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    GLfloat lp[] = {50.0f, 100.0f, 50.0f, 1.0f};
    GLfloat lc[] = {0.6f,0.7f,0.9f,1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lp);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lc);

    // draw background - faint planets as far scenery
    drawStars();
    drawSolarSystemScene(false);

    // neon tunnel
    drawNeonTunnel();

    // draw asteroids
    for (size_t i=0;i<asteroids.size();++i){
        if (asteroids[i].active) drawAsteroid(asteroids[i]);
    }

    // draw player ship
    glPushMatrix();
        glTranslatef(player.pos.x, player.pos.y, player.pos.z);
        glRotatef(player.yaw, 0,1,0);
        glRotatef(player.pitch, 1,0,0);
        // ship model: simple cone+cylinder
        glColor3f(0.9f, 0.95f, 1.0f);
        glPushMatrix();
            glRotatef(180,0,1,0);
            glutSolidCone(0.6f, 1.4f, 12, 2);
        glPopMatrix();
        GLUquadric *q = gluNewQuadric();
        glPushMatrix();
            glTranslatef(0,-0.25f,0.6f);
            glRotatef(90,1,0,0);
            gluCylinder(q, 0.25, 0.25, 1.0, 12, 1);
        glPopMatrix();
        gluDeleteQuadric(q);
    glPopMatrix();

    // HUD overlays
    drawNeonHUD();
}

// ---------- Render dispatcher ----------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set up projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)windowWidth/windowHeight, 0.1, 2000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (currentScreen == MAIN_MENU) {
        gluLookAt(0, 12, 40, 0,0,0, 0,1,0);
        glEnable(GL_LIGHTING);
        drawStars();
        drawSolarSystemScene(true);
        drawMainMenuScreen();
    }
    else if (currentScreen == ANIMATION_MENU) {
        gluLookAt(0, 12, 40, 0,0,0, 0,1,0);
        glEnable(GL_LIGHTING);
        drawStars();
        drawSolarSystemScene(true);
        drawAnimationMenuScreen();
    }
    else if (currentScreen == ANIMATION_VIEW && selectedPlanet >= 0) {
        drawSceneAnimationView(selectedPlanet);
    }
    else if (currentScreen == PLANET_MENU) {
        gluLookAt(0, 12, 40, 0,0,0, 0,1,0);
        glEnable(GL_LIGHTING);
        drawStars();
        drawSolarSystemScene(true);
        drawAnimationMenuScreen();
    }
    else if (currentScreen == PLANET_DETAIL && selectedPlanet >= 0) {
        drawSceneAnimationView(selectedPlanet);
    }
    else if (currentScreen == GAME_MODE || currentScreen == GAME_OVER) {
        drawGameScene();
        if (currentScreen == GAME_OVER) {
            glDisable(GL_LIGHTING);
            glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
            gluOrtho2D(0, windowWidth, 0, windowHeight);
            glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
            if (gameWon) drawText2D(windowWidth*0.5f - 120, windowHeight*0.5f + 16, "YOU SURVIVED! M -> Main Menu");
            else if (gameLost) drawText2D(windowWidth*0.5f - 120, windowHeight*0.5f + 16, "GAME OVER! R -> Restart  M -> Menu");
            glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
            glEnable(GL_LIGHTING);
        }
    }

    glutSwapBuffers();
}

// ---------- Input handling ----------

void keyboardDown(unsigned char key, int x, int y) {
    // MAIN MENU behavior
    if (currentScreen == MAIN_MENU) {
        switch (key) {
            case '1': currentScreen = ANIMATION_MENU; break;
            case '2': currentScreen = PLANET_MENU; break;
            case '3':
                // start game only from main menu
                currentScreen = GAME_MODE;
                startGame();
                break;
            case '4': case 27: exit(0); break; // 4 or ESC
            default: break;
        }
        return;
    }

    // ANIMATION_MENU / PLANET_MENU numeric selection
    if (currentScreen == ANIMATION_MENU || currentScreen == PLANET_MENU) {
        if (key >= '0' && key <= '9') {
            int idx = key - '0';
            if (idx >= 0 && idx < (int)planets.size()) {
                selectedPlanet = idx;
                currentScreen = ANIMATION_VIEW;
            }
            return;
        }
        // other controls for animation menu
        switch (key) {
            case 'b': case 'B': currentScreen = MAIN_MENU; selectedPlanet = -1; break;
            case 'm': case 'M': currentScreen = MAIN_MENU; selectedPlanet = -1; break;
            default: break;
        }
        return;
    }

    // GAME_MODE controls
    if (currentScreen == GAME_MODE) {
        switch (key) {
            case 'w': case 'W': keyW = true; break;
            case 's': case 'S': keyS = true; break;
            case 'a': case 'A': keyA = true; break;
            case 'd': case 'D': keyD = true; break;
            case 'q': case 'Q': keyQ = true; break;
            case 'e': case 'E': keyE = true; break;
            case ' ': keyBoost = true; break;
            case 'm': case 'M': currentScreen = MAIN_MENU; break;
            case 'r': case 'R': // restart from game screen (optional)
                startGame();
                break;
            default: break;
        }
        return;
    }

    // GAME_OVER controls
    if (currentScreen == GAME_OVER) {
        switch (key) {
            case 'r': case 'R': startGame(); currentScreen = GAME_MODE; break;
            case 'm': case 'M': currentScreen = MAIN_MENU; break;
            default: break;
        }
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': case 'W': keyW = false; break;
        case 's': case 'S': keyS = false; break;
        case 'a': case 'A': keyA = false; break;
        case 'd': case 'D': keyD = false; break;
        case 'q': case 'Q': keyQ = false; break;
        case 'e': case 'E': keyE = false; break;
        case ' ': keyBoost = false; break;
        default: break;
    }
}

void specialDown(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT: arrowLeft = true; break;
        case GLUT_KEY_RIGHT: arrowRight = true; break;
        case GLUT_KEY_UP: arrowUp = true; break;
        case GLUT_KEY_DOWN: arrowDown = true; break;
    }
}
void specialUp(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT: arrowLeft = false; break;
        case GLUT_KEY_RIGHT: arrowRight = false; break;
        case GLUT_KEY_UP: arrowUp = false; break;
        case GLUT_KEY_DOWN: arrowDown = false; break;
    }
}

// ---------- Update loop ----------
void updateScene(int value) {
    int t = glutGet(GLUT_ELAPSED_TIME);
    float dt = (lastTimeMs == 0) ? 0.016f : (t - lastTimeMs) / 1000.0f;
    lastTimeMs = t;
    globalTime += dt;
    animTime += dt * 12.0f; // planet animation speed

    // update planet self-rotations
    for (size_t i=0;i<planets.size();++i){
        planets[i].rotation += planets[i].rotationSpeed * dt * 40.0f;
        if (planets[i].rotation >= 360.0f) planets[i].rotation -= 360.0f;
    }

    // camera orbit controls when viewing scenes
    if (currentScreen == ANIMATION_VIEW || currentScreen == ANIMATION_MENU || currentScreen == PLANET_MENU || currentScreen == PLANET_DETAIL) {
        if (arrowLeft) camTheta -= 0.02f;
        if (arrowRight) camTheta += 0.02f;
        if (arrowUp) camPhi -= 0.02f; if (camPhi < 0.05f) camPhi = 0.05f;
        if (arrowDown) camPhi += 0.02f; if (camPhi > 3.0f) camPhi = 3.0f;
    }

    // update game if active
    if (currentScreen == GAME_MODE && gameRunning) {
        // compute accel vector
        Vec3 accel = {0,0,0};
        float yaw = player.yaw * PI/180.0f;
        float pitch = player.pitch * PI/180.0f;
        // explicit float casts (prevent narrowing warnings)
        Vec3 forward = { (float)(-sinf(yaw)*cosf(pitch)), (float)(sinf(pitch)), (float)(-cosf(yaw)*cosf(pitch)) };
        Vec3 right = { (float)cosf(yaw), 0.0f, (float)-sinf(yaw) };
        Vec3 up = {0,1,0};
        float ACC = 160.0f;
        if (keyW) accel = vadd(accel, forward);
        if (keyS) accel = vadd(accel, vmul(forward, -1.0f));
        if (keyA) accel = vadd(accel, vmul(right, -1.0f));
        if (keyD) accel = vadd(accel, right);
        if (keyQ) accel = vadd(accel, up);
        if (keyE) accel = vadd(accel, vmul(up, -1.0f));
        if (vlen(accel) > 0.0001f) {
            float L = vlen(accel); accel = vmul(accel, 1.0f/L);
            player.vel = vadd(player.vel, vmul(accel, ACC*dt));
        } else {
            player.vel = vmul(player.vel, 0.993f);
        }
        if (keyBoost) player.vel = vmul(player.vel, 1.01f);

        // yaw/pitch control
        if (arrowLeft) player.yaw += 80.0f * dt;
        if (arrowRight) player.yaw -= 80.0f * dt;
        if (arrowUp) player.pitch += 40.0f * dt;
        if (arrowDown) player.pitch -= 40.0f * dt;
        player.pitch = clampf(player.pitch, -40.0f, 40.0f);

        // integrate pos
        player.pos = vadd(player.pos, vmul(player.vel, dt));

        // keep inside radial tunnel
        float radial = sqrtf(player.pos.x*player.pos.x + (player.pos.y*0.9f)*(player.pos.y*0.9f));
        if (radial > TUNNEL_RADIUS - 0.9f) {
            float nx = player.pos.x / radial;
            float ny = (player.pos.y*0.9f) / radial;
            player.pos.x = nx * (TUNNEL_RADIUS - 0.9f);
            player.pos.y = ny * (TUNNEL_RADIUS - 0.9f) / 0.9f;
            player.vel = vmul(player.vel, 0.6f);
        }

        // update asteroids
        for (size_t i=0;i<asteroids.size();++i){
            if (!asteroids[i].active) continue;
            asteroids[i].pos = vadd(asteroids[i].pos, vmul(asteroids[i].vel, dt));
            if (asteroids[i].pos.z > player.pos.z + 80.0f) asteroids[i].active = false;
        }
        if ((int)asteroids.size() < MAX_AST && (rand()%100) < 16) spawnAsteroid();

        // collisions
        for (size_t i=0;i<asteroids.size();++i){
            if (!asteroids[i].active) continue;
            float dx = player.pos.x - asteroids[i].pos.x;
            float dy = player.pos.y - asteroids[i].pos.y;
            float dz = player.pos.z - asteroids[i].pos.z;
            float d2 = dx*dx + dy*dy + dz*dz;
            float rsum = 1.0f + asteroids[i].size;
            if (d2 <= rsum*rsum) {
                asteroids[i].active = false;
                player.lives--;
                if (player.lives <= 0) {
                    gameRunning = false; gameLost = true; currentScreen = GAME_OVER;
                }
            }
        }

        // update timer
        gameTimeLeft -= dt;
        if (gameTimeLeft <= 0.0f && gameRunning) {
            gameRunning = false; gameWon = true; currentScreen = GAME_OVER;
        }
        tunnelScroll += vlen(player.vel) * dt * 0.02f;
    }

    lastTimeMs = t;
    glutPostRedisplay();
    glutTimerFunc(16, updateScene, 0);
}

// ---------- Initialization ----------
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0.01f, 0.02f, 0.04f, 1.0f);
    initPlanets();
    initStars(400);
    srand((unsigned)time(NULL));
}

// ---------- Reshape ----------
void reshape(int w, int h) {
    if (h==0) h=1;
    windowWidth = w; windowHeight = h;
    glViewport(0,0,w,h);
}

// ---------- main ----------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Solar System Explorer + Neon Tunnel Runner - Fixed - Farzana");

    initGL();

    // initial player
    player.pos = {0.0f, 0.0f, 60.0f};
    player.yaw = 0.0f; player.pitch = 0.0f;
    player.vel = {0,0,-30.0f};

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);

    lastTimeMs = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, updateScene, 0);

    glutMainLoop();
    return 0;
}
