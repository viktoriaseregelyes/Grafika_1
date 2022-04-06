//=============================================================================================
// Mintaprogram: Z�ld h�romsz�g. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Seregélyes Viktória
// Neptun : OUK8CF
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char* const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char* const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram;
float p = 0, q = 0;
unsigned int vao;

class Atom {
public: float charge;
	  float quantity = 20 + rand() % 40;
	  float x, y, z;
	  const int nTesselatedVertices = 50;
	  std::vector<vec4> points;
	  unsigned int vbo;

	  Atom(float c, float x1 = 0.0f, float y1 = 0.0f, float z1 = 0.0f) {
		  charge = 1 / c;
		  x = x1; y = y1; z = z1;

		  for (int i = 0; i < nTesselatedVertices; i++) {
			  float phi = i * 2.0f * M_PI / 50;
			  points.push_back(vec4((cosf(phi) + x) / quantity, (sinf(phi) + y) / quantity, z / quantity, 1));
		  }
	  }

public:
	vec4 getCenter() { return vec4(x / quantity, y / quantity, z / quantity, 1); }

	float getCX() { return x / quantity; }
	float getCY() { return y / quantity; }
	float getCZ() { return z / quantity; }
	/*
	void setCX(float x1) { x = x1; }
	void setCY(float y1) { y = y1; }
	void setCZ(float z1) { z = z1; }
	*/
	void color() {
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		if (charge < 0)
			glUniform3f(location, 0.0f, 0.0f, -charge);
		else
			glUniform3f(location, charge, 0.0f, 0.0f);
	}

	void drawAtom() {
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);

		color();
		glBufferData(GL_ARRAY_BUFFER, nTesselatedVertices * sizeof(vec4), &points[0], GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLE_FAN, 0, nTesselatedVertices);
	}
};

class Line {
public: float x1, y1, x2, y2;
	  std::vector<vec4> points;
	  unsigned int vbo;

	  Line(Atom a, Atom b) {
		  x1 = a.getCX(); y1 = a.getCY();
		  x2 = b.getCX(); y2 = b.getCY();
	  }

public:
	void drawLine() {
		points.push_back(vec4(x1, y1, 0));
		float dx = abs(x1 - x2) * 0.01, dy = abs(y1 - y2) * 0.01;
		float x = x1, y = y1;
		bool xmi = false, ymi = false;

		if (x < x2) xmi = true;
		if (y < y2) ymi = true;

		while (((xmi && x <= x2) || (!xmi && x >= x2)) && ((ymi && y <= y2) || (!ymi && y >= y2))) {
			if (x < x2) x += dx;
			else x -= dx;

			if (y < y2) y += dy;
			else y -= dy;
			points.push_back(vec4(x, y, 0));
		}

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec4), NULL);

		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, 1.0f, 1.0f, 1.0f);

		glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(vec4), &points[0], GL_STATIC_DRAW);
		glDrawArrays(GL_LINE_STRIP, 0, points.size());
	}
};

class Molekula {
public: std::vector<Atom> atoms;
	  std::vector<Line> lines;
	  std::vector<float> charges;
	  int atomNumber = (int)rand() % 6 + 2;
	  int lineNumber = 0;

	  Molekula() {
		  ChargeMaker();
		  int random;
		  for (int i = 0; i < atomNumber; i++) {
			  if( i == 0 )
				atoms.push_back(Atom(charges.at(i)));

			  if (i > 0) {
				  random = (int)(rand() % i);
				  atoms.push_back(Atom(charges.at(i), atoms.at(random).getCX() + cosf((rand() % (2 * 314)) / 100) * 10, atoms.at(random).getCY() + sinf((rand() % (2 * 314)) / 100) * 10, atoms.at(random).getCZ()));

				  lines.push_back(Line(atoms.at(i), atoms.at(random)));
				  lineNumber++;
			  }
		  }
	  }

public:
	void ChargeMaker() {
		float sum;
		float num;
		do {
			charges.clear();
			sum = 0;
			for (int i = 0; i < atomNumber;i++) {
				num = (float)(-10 + (rand() % 20));
				charges.push_back(num);
				sum += num;
			}
		} while (sum != 0);
	}

	void drawMolekula() {
		for (int i = 0; i < lineNumber; i++)
			lines[i].drawLine();

		for (int i = 0; i < atomNumber; i++)
			atoms[i].drawAtom();
	}
};

std::vector<Molekula> molecules;

void onInitialization() {
	glViewport(0, 0, 600, 600);
	glutInitWindowSize(600, 600);

	molecules.push_back(Molekula());
	//molecules.push_back(Molekula());

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

void onDisplay() {
	glClearColor(0.4f, 0.4f, 0.4f, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	float MVPtransf[4][4] = { 1, 0, 0, 0,
							  0, 1, 0, 0,
							  0, 0, 1, 0,
							  q, p, 0, 1 };

	int location = glGetUniformLocation(gpuProgram.getId(), "MVP");
	glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);

	glBindVertexArray(vao);

	for (int i = 0; i < molecules.size(); i++)
		molecules[i].drawMolekula();

	glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
	switch (key) {
	case 's': q += 0.1f; break;
	case 'd': q -= 0.1f; break;
	case 'x': p += 0.1f; break;
	case 'e': p -= 0.1f; break;
	case ' ': molecules.clear(); molecules.push_back(Molekula()); /*molecules.push_back(Molekula());*/ break;
	}
	glutPostRedisplay();
}

void onKeyboardUp(unsigned char key, int pX, int pY) {
}

void onMouseMotion(int pX, int pY) {
	float cX = 2.0f * pX / windowWidth - 1;
	float cY = 1.0f - 2.0f * pY / windowHeight;
	printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}

void onMouse(int button, int state, int pX, int pY) {
	float cX = 2.0f * pX / windowWidth - 1;
	float cY = 1.0f - 2.0f * pY / windowHeight;

	char* buttonStat;
	switch (state) {
	case GLUT_DOWN: buttonStat = "pressed"; break;
	case GLUT_UP:   buttonStat = "released"; break;
	}

	switch (button) {
	case GLUT_LEFT_BUTTON:   printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);   break;
	case GLUT_MIDDLE_BUTTON: printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
	case GLUT_RIGHT_BUTTON:  printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);  break;
	}
}

void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME);
	float sec = time / 1000.0f;
	for(int i = 0; i < molecules.size(); i++)
		molecules.at(i).Animate(sec);
	glutPostRedisplay();
}