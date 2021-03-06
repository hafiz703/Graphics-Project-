
#include "ParticleSpawner.h"
#include <iostream>
#include <cmath>

#include <cstdlib> 
#include <cassert>
#include <limits>
ParticleSpawner::ParticleSpawner(int numParticles) :ParticleSystem(numParticles)
{
	m_numParticles = numParticles;
	vector<Vector3f> initialState;
	vector<int> initialLifetime;
	vector<vector<int>> allInBox;

	// range of bounding boxes: X = (-3.4520, 4), Y = (-1, 1), Z = (-1, 1)
	// Each entry has 2 Vector3f, which contains the min and max values of x, y and z
	float d = 60 * this->particleRadius;
	float length = 160 * this->particleRadius;
	float increment = length - d;
	radiusOfConsideration = d / 2;

	xCounter = ceil(((4 + 3.4520) - increment) / increment) + 1;
	yCounter = ceil(((0.75 + 0.75) - increment) / increment) + 1;
	zCounter = ceil(((0.25 + 0.25) - increment) / increment) + 1;

	for (int i = 0; i < xCounter; i++) {
		for (int j = 0; j < yCounter; j++) {
			for (int k = 0; k < zCounter; k++) {
				vector<Vector3f> boundary_values;
	
				boundary_values.push_back(Vector3f(4.0f - i * increment, 0.75f - j * increment, 0.25f - k * increment));
				boundary_values.push_back(Vector3f((4.0f - i * increment) - length, (0.75f - j * increment) - length, (0.25f - k * increment) - length));

				box_boundaries.push_back(boundary_values);

				/*cout << "Box " << box_boundaries.size() - 1 << endl;
				cout << "Upper Limit:" << endl;
				Vector3f(4.0f - i * increment, 1.0f - j * increment, 1.0f - k * increment).print();
				cout << "Lower Limit:" << endl;
				Vector3f((4.0f - i * increment) - length, (1.0f - j * increment) - length, (1.0f - k * increment) - length).print();*/
			}
		}
	}

	// boxes stores the particles that are currently in this particular box
	// particleBoxes stores the boxes this particular particle is in
	vector<vector<int>> boxes(xCounter * yCounter * zCounter);
	//cout << boxes.size() << endl;

	int k;
	int lifeSpan = this->particleLifetime;
	// fill in code for initializing the state based on the number of particles
	for (int i = 0; i < m_numParticles; i++) {
		Vector2f temp;
		vector<int> inBox;

		// for this system, we care about the position and the velocity		
		Vector3f position = Vector3f(4, random(-0.75f, 0.75f), random(-0.25f, 0.25f));
		Vector3f velocity = Vector3f(0.0f, 0.0f, 0.0f);
		initialState.push_back(position);
		initialState.push_back(velocity);
		initialLifetime.push_back(lifeSpan);
		impact_counter.push_back(0.0f);

		for (int j = 0; j < yCounter * zCounter; j++) {

			/*cout << "upper limit:" << endl;
			box_boundaries[j][0].print();
			cout << "lower limit:" << endl;
			box_boundaries[j][1].print();
			cout << "pos:" << endl;
			position.print();*/

			if (position.x() <= box_boundaries[j][0].x() && position.x() >= box_boundaries[j][1].x() &&
				position.y() <= box_boundaries[j][0].y() && position.y() >= box_boundaries[j][1].y() &&
				position.z() <= box_boundaries[j][0].z() && position.z() >= box_boundaries[j][1].z()) {

				//cout << "Particle: " << i << " added to box " << j << endl;
			
				boxes[j].push_back(i);
				inBox.push_back(j);
			}
		}

		//cout << inBox.size() << endl;
		allInBox.push_back(inBox);

	}

	setState(initialState);
	setLifetime(initialLifetime);
	setBoxes(boxes);
	setParticleBoxes(allInBox);

	isCloth = false;
	if (isCloth) {
		cloth = new ClothSystem2(6);
	}

	o = new Ball(0.0, r1, m2);

	if (getSysType() == 2) {
		o2 = new Ball(1.2, r2, m2);
	}
}

void ParticleSpawner::addParticles()
{
	int init_numParticles = m_numParticles;
	m_numParticles += particlesPerTick;


	int k;
	int lifeSpan = this->particleLifetime;

	for (int i = 0; i < particlesPerTick; i++) {
		Vector2f temp;
		vector<int> inBox;

		// for this system, we care about the position and the velocity		
		Vector3f position = Vector3f(4, random(-0.75f, 0.75f), random(-0.25f, 0.25f));
		Vector3f velocity = Vector3f(0.0f, 0.0f, 0.0f);
		m_vVecState.push_back(position);
		m_vVecState.push_back(velocity);
		impact_counter.push_back(0.0f);

		m_vLifetime.push_back(lifeSpan);

		for (int j = 0; j < yCounter * zCounter; j++) {
			if (position.x() <= box_boundaries[j][0].x() && position.x() >= box_boundaries[j][1].x() &&
				position.y() <= box_boundaries[j][0].y() && position.y() >= box_boundaries[j][1].y() &&
				position.z() <= box_boundaries[j][0].z() && position.z() >= box_boundaries[j][1].z()) {

				boxes[j].push_back(init_numParticles + i);
				inBox.push_back(j);
			}
		}
		particleBoxes.push_back(inBox);

	}
}

void ParticleSpawner::delParticles()
{
	m_numParticles -= particlesPerTick;

	m_vLifetime.erase(m_vLifetime.begin(), m_vLifetime.begin() + particlesPerTick);
	impact_counter.erase(impact_counter.begin(), impact_counter.begin() + particlesPerTick);
}

vector<Vector3f> ParticleSpawner::collisionDetector_ball(Object* ball, Vector3f particlePos, Vector3f particleVel)
{
	 
	Vector3f ballPos = ball->getState()[0];
	Vector3f ballVel = ball->getState()[1];	 

	Vector3f dxyz = ballPos - particlePos;
	vector<Vector3f> res;
	float distance = sqrt(Vector3f::dot(dxyz,dxyz));
	if (distance <= ball->radius + (2 * particleRadius))  {		
		Vector3f normal = (particlePos - ballPos).normalized();

		const float cor = 1.0f;


		float impact_angle = acos(Vector3f::dot(normal, particleVel) / (sqrt(normal.absSquared()) * sqrt(particleVel.absSquared())));
		Vector3f impact_dVel = (sqrt(particleVel.absSquared()) * cos(impact_angle)) * normal * (1.0f + cor);
		res.push_back(impact_dVel);
		res.push_back(impact_dVel);
		return res;

	}
	else {
		res.push_back(Vector3f(0, 0, 0));
		res.push_back(Vector3f(0, 0, 0));
		return res;
	}
}
Vector3f ParticleSpawner::collisionDetector_cloth(Vector3f particlePos, Vector3f particleVel) {

	Vector3f results = Vector3f(0);
	float cor = 1.0f;

	if ((particlePos - cloth->getState()[12]).abs() < 0.5f){
		for (unsigned i = 0; i < cloth->getState().size(); i += 2) {
			//int i = 4;
			Vector3f ballPos = cloth->getState()[i];

			Vector3f ballVel = cloth->getState()[i + 1];

			Vector3f dxyz = ballPos - particlePos;
			//float dist = sqrt(Vector3f::dot(dxyz, dxyz));
			float distance = dxyz.abs();

			Vector3f ray_dir = particleVel.normalized();
			Vector3f ray_ori = particlePos - ballPos;

			float a = Vector3f::dot(ray_dir, ray_dir);
			float b = 2.0f * Vector3f::dot(ray_dir, ray_ori);
			float c = Vector3f::dot(ray_ori, ray_ori) - (0.2f * 0.2f);

			bool t1 = false;
			bool t2 = false;

			if ((b*b - 4 * a*c) >= 0) {
				if ((-1.0f * b - sqrt(b*b - 4 * a*c)) / (2.0f * a) >= 0) {
					t1 = true;
				}
				if ((-1.0f * b + sqrt(b*b - 4 * a*c)) / (2.0f * a) >= 0) {
					t2 = true;
				}
			}

			if (distance <= (0.28f) + (2 * particleRadius) || t1 || t2 ) {


				Vector3f normal = (particlePos - ballPos).normalized();
				float new_x = normal.x();
				if (normal.x() < 0) {
					new_x = 0;
				}
				Vector3f new_normal = Vector3f(new_x, normal.y(), normal.z());
				float impact_angle = acos(Vector3f::dot(new_normal, particleVel) / (sqrt(new_normal.absSquared()) * sqrt(particleVel.absSquared())));
				Vector3f impact_dVel = (sqrt(particleVel.absSquared()) * cos(impact_angle)) * new_normal * (1.0f + cor);
				//impact_dVel.print();
				results += impact_dVel;
			}
			/*else {
				return Vector3f(0);
			}*/
		}
	}

	return results;

}

bool ParticleSpawner::collisionDetector_cube(Object* box, Vector3f particlePos, Vector3f particleVel) {
	 
	//float eps = particleRadius;
	Vector3f boxPos = box->getState()[0];
	Vector3f boxVel = box->getState()[1];
	float halflen = box->radius / 2.0f;

	 
	float max = -9999999;
	 
	Vector3f box2circle = Vector3f(particlePos[0] - boxPos[0], particlePos[1] - boxPos[1], particlePos[2] - boxPos[2]);
	vector<Vector3f> corners = { boxPos+Vector3f(halflen,-halflen,-halflen),boxPos + Vector3f(halflen,-halflen,halflen),boxPos + Vector3f(halflen,halflen,-halflen),boxPos + Vector3f(halflen,halflen,halflen),boxPos + Vector3f(-halflen,-halflen,-halflen),boxPos + Vector3f(-halflen,-halflen,halflen),boxPos + Vector3f(-halflen,halflen,-halflen),boxPos + Vector3f(-halflen,halflen,halflen) };

		//get the maximum
	for (unsigned i = 0; i < 7;++i )
		{
			Vector3f current_box_corner = corners[i];
				Vector3f v = Vector3f(
					current_box_corner[0] - boxPos[0],
					current_box_corner[1] - boxPos[1],
					current_box_corner[2] - boxPos[2]);
				float current_proj = Vector3f::dot(v, box2circle);
				
				(max < current_proj) ? max = current_proj : max;
		}
	if (box2circle.absSquared() - max - particleRadius > 0 && box2circle.absSquared() > 0) {

	}
	else {
		cout << "collision" << endl;
	} 
	return true;
}

 
// TODO: implement evalF
// for a given state, evaluate f(X,t)
vector<Vector3f> ParticleSpawner::evalF(vector<Vector3f> state)
{

	vector<Vector3f> f;

	
	return f;

}

vector<Vector3f> ParticleSpawner::evalFNew(vector<Vector3f> state, vector<vector<int>> boxes, vector<vector<int>> particleBoxes)
{

	vector<Vector3f> f;

	// YOUR CODE HERE
	float m = 1.0f;
	float dragConst = 25.0f;
	float reductionFactor = 0.2f;


	for (int i = 0; i < m_numParticles; i++) {
		 
		Vector3f v = state[(i * 2) + 1];
		Vector3f resForce = -m * Vector3f(0.0f, 0.0f, 0.0f) - dragConst*v;

		Vector3f windforce = Vector3f(-30, 0, 0); //+ Vector3f(random(-10, 30), random(-10, 30), random(-10, 30)); // Constant windForce + Randomness
		resForce += windforce;


		for (int j = 0; j < particleBoxes[i].size(); j++) {

			int currentBox = particleBoxes[i][j];

			// cout << "currentBox: " << currentBox << endl;
			// cout << "boxes size: " << boxes.size() << endl;

			for (int k = 0; k < boxes[currentBox].size(); k++) {
				if (boxes[currentBox][k] != i) {
					
					Vector3f difference = state[i * 2] - state[boxes[currentBox][k] * 2];
					float distance = sqrt(difference.absSquared());

					if (distance < radiusOfConsideration) {
						resForce += (difference.normalized() / distance) * reductionFactor;

					}
				}
			}
		}
		if (isCloth) {
			Vector3f newV = collisionDetector_cloth(state[i * 2], state[(i * 2) + 1]);
			newV.print();
			v += newV;
			if (newV != Vector3f(0) && impact_counter[i] < 1) {
				cout << "called" << endl;
				impact_counter[i] += 0.1;
			}
		}

		else if (o->getObjectType() == "Ball") {
			Vector3f newV = collisionDetector_ball(o, state[i * 2], state[(i * 2) + 1])[0];

			v += newV;
			if (newV != Vector3f(0) && impact_counter[i] < 1) {
				impact_counter[i] += 0.1;
			}

			/*v += newV2;
			if (newV2 != Vector3f(0) && impact_counter[i] < 1) {
				impact_counter[i] += 0.1;
			}*/
		}
		else if (o->getObjectType() == "Cube") {
			collisionDetector_cube(o, state[i * 2], state[(i * 2) + 1]);
		}
		f.push_back(v);

		//resForce.print();
		f.push_back(resForce / m);

	}


	return f;

}

vector<vector<Vector3f>> ParticleSpawner::evalFCombined(vector<Vector3f> state, vector<Vector3f> o_state, int obj_num, vector<vector<int>> boxes, vector<vector<int>> particleBoxes)
{
	vector<vector<Vector3f>> f;

	vector<Vector3f> f1;
	vector<Vector3f> f2;
	Vector3f ball_impactV = Vector3f();

	// YOUR CODE HERE
	float m = 1.0f;
	float dragConst = 25.0f;
	float reductionFactor = 0.2f;
	//float springConst = 2000.0f;
	//float restLen = 0.8f;

	//cout << state.size() << endl;
	Vector3f o_dV = Vector3f(0);
	Vector3f o_dV2 = Vector3f(0);

	for (int i = 0; i < m_numParticles; i++) {

		Vector3f v = state[(i * 2) + 1];
		Vector3f resForce = -m * Vector3f(0.0f, 0.0f, 0.0f) - dragConst*v;

		Vector3f windforce = Vector3f(-60, 0, 0); //+ Vector3f(random(-10, 30), random(-10, 30), random(-10, 30)); // Constant windForce + Randomness
		resForce += windforce;



		for (int j = 0; j < particleBoxes[i].size(); j++) {

			int currentBox = particleBoxes[i][j];


			for (int k = 0; k < boxes[currentBox].size(); k++) {
				if (boxes[currentBox][k] != i) {

					Vector3f difference = state[i * 2] - state[boxes[currentBox][k] * 2];
					float distance = sqrt(difference.absSquared());
					//cout << distance << endl;
					if (distance < radiusOfConsideration) {
						resForce += (difference.normalized() / distance) * reductionFactor;
						//resForce += Vector3f(0, 10, 0);
						//cout << "Called" << endl;
					}
				}
			}
		}
		if (isCloth) {
			Vector3f newV = collisionDetector_cloth(state[i * 2], state[(i * 2) + 1]);
			v += newV;
			if (newV != Vector3f(0) && impact_counter[i] < 1) {

				impact_counter[i] += 1;
			}
		}
		else if (o->getObjectType() == "Ball") {
			if (obj_num == 1) {
				vector<Vector3f> temp = collisionDetector_ball(o, state[i * 2], state[(i * 2) + 1]);
				v += temp[0];

				o_dV += (1.0f / o->mass)*temp[1];

				if (temp[0] != Vector3f(0) && impact_counter[i] < 1) {
					impact_counter[i] += 0.1;
				}
			}
			else if (obj_num == 2) {
				vector<Vector3f> temp = collisionDetector_ball(o2, state[i * 2], state[(i * 2) + 1]);
				v += temp[0];

				o_dV += (1.0f / o2->mass)*temp[1];
		
				if (temp[0] != Vector3f(0) && impact_counter[i] < 1) {
					impact_counter[i] += 0.1;
				}
			}
		}
		else if (o->getObjectType() == "Cube") {
			//collisionDetector_cube(o, state[i * 2], state[(i * 2) + 1]);
		}
		f1.push_back(v);

		//resForce.print();
		f1.push_back(resForce / m);

	}

	if (getSysType() == 2) {

		Vector3f ball1_Pos = o->getState()[0];
		Vector3f ball1_Vel = o->getState()[1];
		Vector3f ball2_Pos = o2->getState()[0];
		Vector3f ball2_Vel = o2->getState()[1];


		Vector3f dxyz = ball1_Pos - ball2_Pos;
		float distance = sqrt(Vector3f::dot(dxyz, dxyz));
		if (distance <= o->radius + o2->radius) {
			if (obj_num == 1) {
				Vector3f normal = (ball1_Pos - ball2_Pos).normalized();
				float impact_angle = acos(Vector3f::dot(normal, ball2_Vel) / (sqrt(normal.absSquared()) * sqrt(ball2_Vel.absSquared())));
				Vector3f impact_dVel = (sqrt(ball2_Vel.absSquared()) * cos(impact_angle)) * normal * (1.0f + 1.0f);
				ball_impactV += (-1.0f / o->mass)*impact_dVel*5000.0f;

			}
			else if (obj_num == 2) {
				Vector3f normal = (ball2_Pos - ball1_Pos).normalized();
				float impact_angle = acos(Vector3f::dot(normal, ball1_Vel) / (sqrt(normal.absSquared()) * sqrt(ball1_Vel.absSquared())));
				Vector3f impact_dVel = (sqrt(ball1_Vel.absSquared()) * cos(impact_angle)) * normal * (1.0f + 1.0f);
				ball_impactV += (-1.0f / o2->mass)*impact_dVel*5000.0f;

			}
		}
	}

	for (int i = 0; i < o_state.size(); i = i + 2) {
		
		f2.push_back(o_state[i + 1] + o_dV + ball_impactV);
		

		Vector3f gravF = Vector3f(0.0f, o->mass * -1.0f * 0.0f, 0.0f);

		Vector3f dragF = -2.0f * (o_state[i + 1] + o_dV);
		Vector3f netF = (gravF + dragF) / o->mass;

		f2.push_back(netF);

	}

	f.push_back(f1);
	f.push_back(f2);

	return f;

}

// render the system (ie draw the particles)
void ParticleSpawner::draw()
{
	//if (o->getObjectType().compare("Cube") == 0) {
	//	cout << "true" << endl;
	//}
	for (int i = 0; i < m_numParticles; i++) {
		Vector3f pos = m_vVecState[i * 2];
		glPushMatrix();
		glTranslatef(pos[0], pos[1], pos[2]);
		GLfloat color[4] = { impact_counter[i], 0.5, 0.8, 0.1 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glutSolidSphere(this->particleRadius, 10.0f, 10.0f);
		glPopMatrix();
	}



	GLfloat color[4] = { 0.2, 0.2, 1, 1.0 };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
	//cloth->draw();
	o->objectDraw();
	if (getSysType() == 2) {
		o2->objectDraw();
	}
	/*o2->objectDraw();
	o3->objectDraw();
	o4->objectDraw();*/

}

float ParticleSpawner::random(float low, float upp) {
	int low_int = low * 100;
	int upp_int = upp * 100;
	return low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (upp_int - low_int) * 100));

}