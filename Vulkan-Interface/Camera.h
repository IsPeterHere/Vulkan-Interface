#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

class Camera
{
public:
	Camera(){}

	glm::mat4x4 get_look_at() { update_look_at(); return look_at; }

	void euler_update(double delta_yaw, double delta_pitch);

	void position_update(glm::vec3 delta_position);

private:
	glm::vec3 position{ 2,2,2 };
	glm::vec3 true_up{ 0,0,1 };
	glm::vec3 front;
	glm::vec3 right;
	double yaw = 0, pitch = 180;

	glm::mat4x4 look_at{ glm::lookAt(position, {0,0,0}, true_up) };

	void update_look_at();
};