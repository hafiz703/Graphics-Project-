#ifndef OBJECT_H
#define OBJECT_H


#include <vecmath.h>
#include <vector>
#ifdef _WIN32
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
#endif

using namespace std;

class Object {

public:
	Object() {};

	void setState(const vector<Vector3f> &state) {

		this->ostate = state;
	};

	vector<Vector3f> getState() { return this->ostate; };

	virtual vector<Vector3f> evalF() = 0;

	virtual void objectDraw() = 0;/*{
								  glutSolidCone(2.0f,4.0f,1.0f,1.0f);
								  std::cout << "notinherited" << std::endl;
								  }*/


protected:
	vector<Vector3f> ostate;
	float mass;
	float radius;

	float gravity = 9.8f;

	//drag coeff
	float cd = 2.0f;
	float steel_density = 2.0f;
	float projected_area = 2.0f;
	//float drag = (cd * steel_density * projected_area) / 2.0f;


};

#endif