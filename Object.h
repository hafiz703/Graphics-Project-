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
	float radius;
	int num_particles;
	float mass;

	void setState(const vector<Vector3f> &state) {

		this->ostate = state;
	};

	vector<Vector3f> getState() { return this->ostate; };
	virtual vector<Vector3f> evalF(vector<Vector3f> state) = 0;

	virtual void objectDraw() = 0;
	virtual string getObjectType() = 0;

protected:
	vector<Vector3f> ostate;
	
	

	float gravity = 9.8f;

	//drag coeff
	float cd = 2.0f;
	float steel_density = 2.0f;
	float projected_area = 2.0f;
	//float drag = (cd * steel_density * projected_area) / 2.0f;


};

#endif