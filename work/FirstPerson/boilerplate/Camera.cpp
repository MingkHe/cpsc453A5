#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace std;
using namespace glm;

const float MAX_VERT = 0.95f;

glm::mat4 Camera::viewMatrix() const{
	//mat4 cRotation = transpose(mat4(vec4(right, 0), vec4(up, 0), vec4(-dir, 0), vec4(0, 0, 0, 1)));
	//mat4 cTranslation = glm::translate(mat4(1.f), -pos);
	//return cRotation*cTranslation;
	
	float theta = acos(pos.y/radius);
	float phi = asin(pos.x/radius);
	cout<<"Theta: "<<theta<<endl;
	cout<<"Phi: "<<phi<<endl;	
	
	return lookAt(pos,vec3(0,0,0),up);
}

void Camera::rotateVertical(float radians){
	//mat4 rotationMatrix = glm::rotate(mat4(1.f), radians, right);
	//vec3 newDir = normalize(vec3(rotationMatrix*vec4(dir, 0)));

	

	pos.y = radius*sin(radius*asin(pos.y/radius)+radians);
	pos.z = radius*cos(radius*asin(pos.y/radius)+radians);
}

void Camera::rotateHorizontal(float radians){
	//mat4 rotationMatrix = glm::rotate(mat4(1.f), radians, vec3(0, 1, 0));
	//dir = normalize(vec3(rotationMatrix*vec4(dir, 0)));
	//right = normalize(cross(dir, vec3(0, 1, 0)));
	//up = normalize(cross(right, dir));
	
	float angle = radius*asin(pos.x/radius)+radians;
	pos.z = radius*cos(angle);
	pos.x = radius*sin(angle);
}

void Camera::move2(float deltaTheta, float deltaPhi){
	float theta = acos(pos.y/radius)+deltaTheta;
	float phi = asin(pos.x/radius)+deltaPhi;
	//pos = vec3(radius*sin(theta)*cos(phi),radius*cos(theta),radius*sin(theta)*sin(phi));
	pos.x = radius*sin(theta)*cos(phi);
	pos.y = radius*cos(theta);
	pos.z = radius*sin(theta)*sin(phi);
	//cout<<"Theta: "<<theta<<endl;
	//cout<<"Phi: "<<phi<<endl;
	//cout<<"x: "<<pos.x<<" y: "<<pos.y<<" z: "<<pos.z<<endl;
	//cout<<pos.x<<endl;
}

void Camera::scale(float level){
	
	pos.x = level*sin(asin(pos.x/radius));
	pos.z = level*cos(asin(pos.x/radius));
	pos.y = level*sin(asin(pos.y/radius));
	radius = level;
}


void Camera::move(vec3 movement){
	pos += movement.x*right + movement.y*up + movement.z*dir;
}




