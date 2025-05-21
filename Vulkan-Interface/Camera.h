#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

class Camera
{
public:
	Camera()
	{
	}

	glm::mat4x4 get_look_at() { update_look_at(); return look_at; }

	void euler_update(double delta_yaw, double delta_pitch)
	{
		yaw += delta_yaw;
		pitch += delta_pitch;

		
		while (pitch > 89)
			pitch = 89;
		while (pitch < -89)
			pitch = -89;

		while (yaw > 360)
			yaw -= 360;
		while (yaw < 0)
			yaw += 360;
	}

	void position_update(glm::vec3 delta_position) { 
		position += front * delta_position.x;
		position += right * delta_position.y;
	}

private:
	glm::vec3 position{ 2,2,2 };
	glm::vec3 true_up{ 0,0,1 };
	glm::vec3 front;
	glm::vec3 right;
	double yaw = 0, pitch = 180;

	glm::mat4x4 look_at{ glm::lookAt(position, {0,0,0}, true_up) };

	void update_look_at()
	{
		glm::mat3x3 rotation(glm::orientate3(glm::vec3(glm::radians(pitch), glm::radians(yaw),0.0)));
		front =  glm::vec3(0.0, 1.0, 0.0) * rotation;
		right = glm::cross(front, true_up);
		look_at = glm::lookAt(position, position + front, true_up);
	}
};