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

vec3 transformToHyperbolic(vec3 v) {
	return vec3(v.x, v.y, sqrt(v.x * v.x + v.y * v.y + 1));
}

vec3 transformToPoincare(vec3 v) {
	return vec3(v.x / (v.z + 1), v.y / (v.z + 1), 0);
}

class Atom {
public: float charge;
	  float quantity = 10 + rand() % 10;
	  vec3 center;
	  const int nTesselatedVertices = 50;
	  std::vector<vec3> tarfoPoints;
	  std::vector<vec3> points;
	  unsigned int vbo;

	  Atom(float c, float x1 = 0.0f, float y1 = 0.0f, float z1 = 0.0f) {
		  charge = 1 / c;
		  center = vec3(x1, y1, z1);

		  for (int i = 0; i < nTesselatedVertices; i++) {
			  float phi = i * 2.0f * M_PI / nTesselatedVertices;
			  points.push_back(vec3((cosf(phi) + center.x) * (1 / quantity), (sinf(phi) + center.y) * (1 / quantity), center.z));
			  tarfoPoints.push_back(transformToPoincare(transformToHyperbolic(points.at(i))));
		  }
		  center.x *= (1 / quantity);
		  center.y *= (1 / quantity);
	  }

public:
	vec3 getCenter() { return center; }

	float getQuantity() { return quantity; }

	void color() {
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		if (charge < 0)
			glUniform3f(location, 0.0f, 0.0f, -charge);
		else
			glUniform3f(location, charge, 0.0f, 0.0f);
	}

	void create() {
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
		glBufferData(GL_ARRAY_BUFFER, nTesselatedVertices * sizeof(vec3), &tarfoPoints[0], GL_STATIC_DRAW);
	}

	void drawAtom() {
		color();
		glDrawArrays(GL_TRIANGLE_FAN, 0, nTesselatedVertices);
	}
};

class Line {
public: vec3 a, b;
	  std::vector<vec3> tarfoPoints;
	  std::vector<vec3> points;
	  unsigned int vbo;

	  Line(Atom x, Atom y) {
		  a = x.getCenter();
		  b = y.getCenter();
	  }

public:
	void create() {
		tarfoPoints.push_back(transformToPoincare(transformToHyperbolic(vec3(a.x, a.y, 0))));
		points.push_back(vec3(a.x, a.y, 0));
		float dx = abs(a.x - b.x) * 0.01, dy = abs(a.y - b.y) * 0.01;
		float x = a.x, y = a.y;
		bool xmi = false, ymi = false;

		if (x < b.x) xmi = true;
		if (y < b.y) ymi = true;

		while (((xmi && x <= b.x) || (!xmi && x >= b.x)) && ((ymi && y <= b.y) || (!ymi && y >= b.y))) {
			if (x < b.x) x += dx;
			else x -= dx;

			if (y < b.y) y += dy;
			else y -= dy;
			tarfoPoints.push_back(transformToPoincare(transformToHyperbolic(vec3(x, y, 0))));
			points.push_back(vec3(x, y, 0));
		}

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec3), NULL);
		glBufferData(GL_ARRAY_BUFFER, tarfoPoints.size() * sizeof(vec3), &tarfoPoints[0], GL_STATIC_DRAW);
	}

	void drawLine() {
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, 1.0f, 1.0f, 1.0f);

		glDrawArrays(GL_LINE_STRIP, 0, tarfoPoints.size());
	}
};

class Molekula {
public: std::vector<Atom> atoms;
	  std::vector<Line> lines;
	  std::vector<float> charges;
	  std::vector<float> coulomb;
	  int atomNumber = (int)rand() % 6 + 2;
	  int lineNumber = 0;
	  vec3 massCenter;
	  float phi;

	  Molekula() {
		  Animate(0);
		  ChargeMaker();
		  int random;
		  for (int i = 0; i < atomNumber; i++) {
			  if (i == 0)
				  atoms.push_back(Atom(charges.at(i), CenterPointMaker(), CenterPointMaker(), 0.0f));

			  if (i > 0) {
				  random = (int)(rand() % i);
				  atoms.push_back(Atom(charges.at(i), atoms.at(random).getCenter().x + cosf((rand() % (2 * 314)) / 100) * 3, atoms.at(random).getCenter().y + sinf((rand() % (2 * 314)) / 100) * 3, atoms.at(random).getCenter().z));
				  lines.push_back(Line(atoms.at(i), atoms.at(random)));
				  lineNumber++;
			  }
		  }
	  }

public:
	void Animate(float t) {
		phi = t;
	}

	mat4 M() {
		mat4 Mrotate(cosf(phi), sinf(phi), 0, 0,
					-sinf(phi), cosf(phi), 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);

		mat4 Mtranslate(1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 0, 0,
						q, p, 0, 1);

		return Mrotate * Mtranslate;
	}

	float CenterPointMaker() { return (-150 + rand() % 300) / 10; }

	void Coulomb() {

	}

	void MassCenter() {
		vec3 v1 = atoms.at(0).getCenter(), v2;
		float q1 = atoms.at(0).getQuantity(), q2;
		float l2;
		int i = 0;
		while (i != atomNumber - 1)
		{
			v2 = atoms.at(i + 1).getCenter();
			q2 = atoms.at(i + 1).getQuantity();
			l2 = ((q1 / q2) * dot(v1, v2) / (1 + (q1 / q2)));

			//vmi ami a lineon megkeresi, hol is van a tömegközéppont
			//a lineon 2 pont jöhet szóba, az kell, ami a másik ponthoz közelebb van
			q1 = (q1 + q2) / 2;
			i++;
		}
	}

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
		mat4 MVPTransform = M();
		gpuProgram.setUniform(MVPTransform, "MVP");

		for (int i = 0; i < lineNumber; i++) {
			lines[i].create();
			lines[i].drawLine();
		}

		for (int i = 0; i < atomNumber; i++) {
			atoms[i].create();
			atoms[i].drawAtom();
		}
	}
};

std::vector<Molekula> molecules;

void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);

	molecules.push_back(Molekula());
	molecules.push_back(Molekula());

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

void onDisplay() {
	glClearColor(0.4f, 0.4f, 0.4f, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < molecules.size(); i++)
		molecules.at(i).drawMolekula();

	glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
	switch (key) {
	case 's': q += 0.1f; break;
	case 'd': q -= 0.1f; break;
	case 'x': p += 0.1f; break;
	case 'e': p -= 0.1f; break;
	case ' ': molecules.clear(); molecules.push_back(Molekula()); molecules.push_back(Molekula()); break;
	}
	glutPostRedisplay();
}

void onKeyboardUp(unsigned char key, int pX, int pY) { }

void onMouseMotion(int pX, int pY) { }

void onMouse(int button, int state, int pX, int pY) { }

void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME);
	float sec = time / 10000.0f;
	for(int i=0; i < molecules.size(); i++)
		molecules.at(i).Animate(sec);
	glutPostRedisplay();
}