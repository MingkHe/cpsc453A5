#include <glm/glm.hpp>

class Camera{
public:
	float radius;
	glm::vec3 dir, right, up, pos;
	

	Camera():radius(1.0f), dir(glm::vec3(0, 0, -1)), right(glm::vec3(1, 0, 0)), up(glm::vec3(0, 1, 0)), pos(glm::vec3(0,0,-radius)){}
	Camera(float radius, glm::vec3 dir, glm::vec3 right, glm::vec3 up, glm::vec3 pos):radius(radius), dir(dir), right(right), up(up), pos(pos){}


	glm::mat4 viewMatrix() const;

	void rotateVertical(float radians);
	void rotateHorizontal(float radians);
	void move(glm::vec3 movement);		//Moves in rotated frame
	void scale(float level);
	void move2(float deltaTheta, float deltaPhi);
};
