#include <GL/glut.h>
#include <cmath>
#include <string>
#include <vector>
#include <sstream>

// ---------- Program state ----------
enum Screen { MAIN_MENU, ANIMATION, PLANET_MENU, PLANET_DETAIL };
Screen currentScreen = MAIN_MENU;

int windowWidth = 1000, windowHeight = 700;

// camera state (spherical)
float camRadius = 40.0f;
float camTheta = 1.2f; // azimuth
float camPhi = 0.6f;   // elevation

// animation angles
float angleTime = 0.0f;

// planet structure
struct Planet {
    std::string name;
    float orbitRadius; // distance from sun (scaled)
    float size;        // radius scale
    float orbitSpeed;  // degrees per frame
    float rotation;    // self rotation angle
    float rotationSpeed;
    std::string info;  // short info
};

std::vector<Planet> planets;

// earth index (for connection)
int earthIndex = 3; // we will put planets in order; ensure Earth is index 3

// selection for detail screen
int selectedPlanet = -1;

// star field
std::vector<float> stars; // x,y,z positions

// ---------- Utility ----------
float deg2rad(float d) { return d * 3.14159265f / 180.0f; }

void drawText2D(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}

// draw small text (HUD)
void drawTextSmall(int x, int y, const std::string& text) {
    glRasterPos2i(x, y);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
}

// init planets data
void initPlanets() {
    planets.clear();
    // Sun (index 0) — handled separately, but keep a placeholder
    planets.push_back({ "Sun", 0.0f, 3.0f, 0.0f, 0.0f, 0.0f,
        "The Sun: a G-type main-sequence star at center of solar system." });
    // Mercury
    planets.push_back({ "Mercury", 6.0f, 0.25f, 4.8f, 0.0f, 0.5f,
        "Mercury: Smallest planet, very close to Sun." });
    // Venus
    planets.push_back({ "Venus", 8.5f, 0.6f, 3.5f, 0.0f, 0.3f,
        "Venus: Earth's twin with thick toxic atmosphere." });
    // Earth (index 3)
    planets.push_back({ "Earth", 11.0f, 0.65f, 2.5f, 0.0f, 1.2f,
        "Earth: Our home planet. Radius ~6371 km. 1 moon." });
    // Moon (we will treat as a special orbiting body attached to Earth)
    planets.push_back({ "Moon", 11.8f, 0.16f, 8.0f, 0.0f, 0.8f,
        "Moon: Earth's natural satellite." });
    // Mars
    planets.push_back({ "Mars", 14.5f, 0.35f, 1.9f, 0.0f, 0.9f,
        "Mars: The Red Planet. Evidence of past water." });
    // Jupiter
    planets.push_back({ "Jupiter", 19.0f, 1.4f, 1.0f, 0.0f, 0.6f,
        "Jupiter: Gas giant, largest planet with many moons." });
    // Saturn (rings drawn)
    planets.push_back({ "Saturn", 24.0f, 1.1f, 0.7f, 0.0f, 0.5f,
        "Saturn: Gas giant known for its rings." });
    // Uranus
    planets.push_back({ "Uranus", 28.0f, 0.9f, 0.5f, 0.0f, 0.4f,
        "Uranus: Ice giant with extreme axial tilt." });
    // Neptune
    planets.push_back({ "Neptune", 31.5f, 0.85f, 0.4f, 0.0f, 0.45f,
        "Neptune: Distant ice giant, strong winds." });
}

// create a simple star field
void initStars(int count = 200) {
    stars.clear();
    for (int i = 0; i < count; ++i) {
        float x = (rand() % 2000 - 1000) / 10.0f;
        float y = (rand() % 2000 - 1000) / 10.0f;
        float z = (rand() % 2000 - 1000) / 10.0f;
        stars.push_back(x); stars.push_back(y); stars.push_back(z);
    }
}

// ---------- Drawing helpers ----------
void drawAxes(float len = 5.0f) {
    glBegin(GL_LINES);
    // X red
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(len, 0, 0);
    // Y green
    glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, len, 0);
    // Z blue
    glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, len);
    glEnd();
}

// draw a planet sphere at origin with given size and color
void drawPlanetSphere(float size, float r, float g, float b) {
    glColor3f(r, g, b);
    glutSolidSphere(size, 40, 40);
}

// draw ring for Saturn
void drawRing(float innerR, float outerR) {
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0;i <= 360;i += 4) {
        float a = deg2rad(i);
        float x1 = cos(a) * innerR;
        float z1 = sin(a) * innerR;
        float x2 = cos(a) * outerR;
        float z2 = sin(a) * outerR;
        glVertex3f(x1, 0.0f, z1);
        glVertex3f(x2, 0.0f, z2);
    }
    glEnd();
}

// draw orbit circle in XZ plane
void drawOrbit(float radius) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0;i < 360;i += 4) {
        float a = deg2rad(i);
        glVertex3f(cos(a) * radius, 0.0f, sin(a) * radius);
    }
    glEnd();
}

// ---------- Screens ----------

void drawMainMenu() {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    glColor3f(1, 1, 0.6f);
    drawText2D(windowWidth * 0.3f, windowHeight * 0.8f, "SOLAR SYSTEM EXPLORER (3D)");
    glColor3f(1, 1, 1);
    drawText2D(windowWidth * 0.35f, windowHeight * 0.62f, "1. Start Animation");
    drawText2D(windowWidth * 0.35f, windowHeight * 0.52f, "2. Planet Information");
    drawText2D(windowWidth * 0.35f, windowHeight * 0.42f, "ESC. Exit");

    drawTextSmall(20, 30, "Use Arrow keys to rotate camera, + / - to zoom. Press M to return to menu.");

    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawStars() {
    glDisable(GL_LIGHTING);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glColor3f(1, 1, 1);
    for (size_t i = 0;i < stars.size(); i += 3) {
        glVertex3f(stars[i], stars[i + 1], stars[i + 2]);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawSolarSystemScene(bool showOrbits = true) {
    // Sun at origin
    glPushMatrix();
    glDisable(GL_LIGHTING);
    // Sun glowing
    glColor3f(1.0f, 0.9f, 0.2f);
    glutSolidSphere(planets[0].size, 50, 50);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // Draw each planet with orbit and position
    for (size_t i = 1;i < planets.size();++i) {
        Planet& p = planets[i];
        float ang = angleTime * p.orbitSpeed;
        float x = cos(deg2rad(ang)) * p.orbitRadius;
        float z = sin(deg2rad(ang)) * p.orbitRadius;

        glPushMatrix();
        // draw orbit
        if (showOrbits) {
            glColor3f(0.3f, 0.3f, 0.3f);
            drawOrbit(p.orbitRadius);
        }

        glTranslatef(x, 0.0f, z);
        // self rotation
        glRotatef(p.rotation, 0.0f, 1.0f, 0.0f);

        // color per planet (simple mapping)
        if (p.name == "Mercury") drawPlanetSphere(p.size, 0.5f, 0.5f, 0.5f);
        else if (p.name == "Venus") drawPlanetSphere(p.size, 0.9f, 0.7f, 0.3f);
        else if (p.name == "Earth") {
            // Earth blue-green
            drawPlanetSphere(p.size, 0.2f, 0.5f, 1.0f);
            // draw a greenish overlay to hint continents (very simple)
            glColor3f(0.0f, 0.4f, 0.0f);
            glPushMatrix();
            glRotatef(p.rotation * 2, 0, 1, 0);
            glTranslatef(0.0f, 0.0f, p.size * 0.85f);
            glutSolidSphere(p.size * 0.05f, 6, 6);
            glPopMatrix();
        }
        else if (p.name == "Moon") {
            drawPlanetSphere(p.size, 0.8f, 0.8f, 0.8f);
        }
        else if (p.name == "Mars") drawPlanetSphere(p.size, 1.0f, 0.3f, 0.2f);
        else if (p.name == "Jupiter") drawPlanetSphere(p.size, 0.9f, 0.7f, 0.5f);
        else if (p.name == "Saturn") {
            drawPlanetSphere(p.size, 0.9f, 0.85f, 0.6f);
            // ring
            glColor3f(0.8f, 0.7f, 0.5f);
            drawRing(p.size * 1.4f, p.size * 2.2f);
        }
        else if (p.name == "Uranus") drawPlanetSphere(p.size, 0.6f, 0.8f, 0.9f);
        else if (p.name == "Neptune") drawPlanetSphere(p.size, 0.2f, 0.4f, 0.9f);
        glPopMatrix();
    }

    // Draw Earth Moon specially: place the moon relative to Earth
    // Find Earth coords
    Planet& earth = planets[earthIndex];
    float eang = angleTime * earth.orbitSpeed;
    float ex = cos(deg2rad(eang)) * earth.orbitRadius;
    float ez = sin(deg2rad(eang)) * earth.orbitRadius;
    // moon is planets[4] by our data
    Planet& moon = planets[4];
    float mang = angleTime * moon.orbitSpeed;
    float mx = ex + cos(deg2rad(mang)) * 0.8f;
    float mz = ez + sin(deg2rad(mang)) * 0.8f;
    // draw moon at mx,mz
    glPushMatrix();
    glTranslatef(mx, 0.0f, mz);
    drawPlanetSphere(moon.size, 0.8f, 0.8f, 0.8f);
    glPopMatrix();
}

// Draw connection (animated) between selected planet and Earth
void drawConnectionToEarth(int pid) {
    if (pid < 0 || pid >= (int)planets.size()) return;
    // compute planet position
    Planet& p = planets[pid];
    float pang = angleTime * p.orbitSpeed;
    float px = (p.orbitRadius == 0.0f) ? 0.0f : cos(deg2rad(pang)) * p.orbitRadius;
    float pz = (p.orbitRadius == 0.0f) ? 0.0f : sin(deg2rad(pang)) * p.orbitRadius;
    float py = 0.0f;

    Planet& earth = planets[earthIndex];
    float eang = angleTime * earth.orbitSpeed;
    float ex = cos(deg2rad(eang)) * earth.orbitRadius;
    float ez = sin(deg2rad(eang)) * earth.orbitRadius;
    float ey = 0.0f;

    // draw animated dashed line (by sampling points and toggling)
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.7f, 0.2f);
    // animate using angleTime to shift dash
    float tshift = fmod(angleTime * 10.0f, 1.0f);
    const int segments = 60;
    for (int i = 0;i < segments;i++) {
        float u1 = float(i) / segments;
        float u2 = float(i + 1) / segments;
        // compute point 1 and 2
        float x1 = px * (1 - u1) + ex * u1;
        float z1 = pz * (1 - u1) + ez * u1;
        float x2 = px * (1 - u2) + ex * u2;
        float z2 = pz * (1 - u2) + ez * u2;
        // decide to draw dash segment or not
        float dashPattern = fmod(u1 + tshift, 0.08f);
        if (dashPattern < 0.04f) {
            glVertex3f(x1, 0.1f, z1);
            glVertex3f(x2, 0.1f, z2);
        }
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

// ---------- Display and UI ----------
void displayScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)windowWidth / windowHeight, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float camX = camRadius * sin(camPhi) * cos(camTheta);
    float camY = camRadius * cos(camPhi);
    float camZ = camRadius * sin(camPhi) * sin(camTheta);
    gluLookAt(camX, camY, camZ, 0, 0, 0, 0, 1, 0);

    // lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat ambient[] = { 0.05f,0.07f,0.1f,1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    GLfloat lightpos[] = { 50.0f, 50.0f, 50.0f, 1.0f };
    GLfloat lightcol[] = { 0.9f,0.9f,0.85f,1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightcol);

    // stars background
    drawStars();

    // draw orbits & planets
    drawSolarSystemScene(true);

    // if in planet detail, draw connection to Earth and highlight selected
    if (currentScreen == PLANET_DETAIL && selectedPlanet >= 0) {
        drawConnectionToEarth(selectedPlanet);
        // highlight selected planet (bigger)
        Planet& p = planets[selectedPlanet];
        float pang = angleTime * p.orbitSpeed;
        float px = (p.orbitRadius == 0.0f) ? 0.0f : cos(deg2rad(pang)) * p.orbitRadius;
        float pz = (p.orbitRadius == 0.0f) ? 0.0f : sin(deg2rad(pang)) * p.orbitRadius;
        glDisable(GL_LIGHTING);
        glPushMatrix();
        glTranslatef(px, 0.0f, pz);
        glColor3f(1.0f, 1.0f, 0.5f);
        glutWireSphere(p.size * 1.6f, 16, 16);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }

    // overlay text / UI
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    if (currentScreen == ANIMATION) {
        drawText2D(20, windowHeight - 40, "Animation: Press M for Main Menu. Arrow keys rotate camera. + / - to zoom. R reset.");
    }
    else if (currentScreen == PLANET_MENU) {
        drawText2D(20, windowHeight - 40, "Planet Info Menu: Press the number of the planet to view details. B to go back, M main menu.");
        // draw planet list
        float baseY = windowHeight - 80;
        for (size_t i = 0;i < planets.size();++i) {
            std::ostringstream ss; ss << i << ". " << planets[i].name;
            drawText2D(40, baseY - i * 22, ss.str());
        }
    }
    else if (currentScreen == PLANET_DETAIL && selectedPlanet >= 0) {
        // show planet info text on right
        Planet& p = planets[selectedPlanet];
        drawText2D(20, windowHeight - 40, "Planet Detail: Press B for planet list, M for main menu.");
        float x0 = windowWidth - 380;
        float y0 = windowHeight - 60;
        drawText2D(x0, y0, ("Name: " + p.name));
        drawText2D(x0, y0 - 28, ("Orbit radius (scaled): " + std::to_string(p.orbitRadius)));
        drawText2D(x0, y0 - 56, ("Size (scaled): " + std::to_string(p.size)));
        // info lines
        std::string info = p.info;
        drawText2D(x0, y0 - 84, ("Info: "));
        // break info to multiple lines
        int line = 0;
        std::istringstream iss(info);
        std::string token;
        std::string accum;
        while (std::getline(iss, token, '.')) {
            if (token.size() > 0) {
                drawText2D(x0 + 12, y0 - 112 - line * 20, token + ".");
                line++;
            }
        }
    }
    else if (currentScreen == MAIN_MENU) {
        // nothing here - main menu is drawn in separate function
    }

    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

// draw full-screen main menu (separate)
void displayMainMenu() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    glColor3f(1, 1, 0.6f);
    drawText2D(windowWidth * 0.28f, windowHeight * 0.78f, "SOLAR SYSTEM EXPLORER (3D)");
    glColor3f(1, 1, 1);
    drawText2D(windowWidth * 0.36f, windowHeight * 0.58f, "1. Start Animation");
    drawText2D(windowWidth * 0.36f, windowHeight * 0.50f, "2. Planet Information");
    drawText2D(windowWidth * 0.36f, windowHeight * 0.42f, "ESC. Exit");

    drawTextSmall(20, 30, "Arrow keys rotate camera. + / - to zoom. M to return to main menu at any time.");

    glEnable(GL_LIGHTING);
    glutSwapBuffers();
}

// main display dispatcher
void display() {
    if (currentScreen == MAIN_MENU) displayMainMenu();
    else displayScene();
}

// ---------- Input ----------
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: exit(0); break;
    case '1':
        currentScreen = ANIMATION;
        break;
    case '2':
        currentScreen = PLANET_MENU;
        break;
    case 'm': case 'M':
        currentScreen = MAIN_MENU;
        selectedPlanet = -1;
        break;
    case 'b': case 'B':
        if (currentScreen == PLANET_DETAIL) currentScreen = PLANET_MENU;
        break;
    case '+':
        camRadius -= 2.0f; if (camRadius < 5.0f) camRadius = 5.0f;
        break;
    case '-':
        camRadius += 2.0f; if (camRadius > 300.0f) camRadius = 300.0f;
        break;
    case 'r': case 'R':
        camRadius = 40.0f; camTheta = 1.2f; camPhi = 0.6f;
        break;
    default:
        // if in PLANET_MENU, allow selecting planet by number key
        if (currentScreen == PLANET_MENU && key >= '0' && key <= '9') {
            int idx = key - '0';
            if (idx >= 0 && idx < (int)planets.size()) {
                selectedPlanet = idx;
                currentScreen = PLANET_DETAIL;
            }
        }
        break;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    const float angStep = 0.05f;
    switch (key) {
    case GLUT_KEY_LEFT: camTheta -= angStep; break;
    case GLUT_KEY_RIGHT: camTheta += angStep; break;
    case GLUT_KEY_UP: camPhi -= angStep; if (camPhi < 0.05f) camPhi = 0.05f; break;
    case GLUT_KEY_DOWN: camPhi += angStep; if (camPhi > 3.09f) camPhi = 3.09f; break;
    }
    glutPostRedisplay();
}

// ---------- Timer ----------
void update(int value) {
    // increment animation time
    angleTime += 0.5f; // global time increment; speed can be tuned

    // update self rotations lightly
    for (size_t i = 1;i < planets.size();++i) {
        planets[i].rotation += planets[i].rotationSpeed;
        if (planets[i].rotation >= 360.0f) planets[i].rotation -= 360.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ---------- Init ----------
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glClearColor(0.02f, 0.02f, 0.06f, 1.0f); // dark-blue space
    initPlanets();
    initStars(300);
}

// ---------- Window reshape ----------
void reshape(int w, int h) {
    windowWidth = w; windowHeight = h;
    glViewport(0, 0, w, h);
}

// ---------- Main ----------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Solar System Explorer (3D)");

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
