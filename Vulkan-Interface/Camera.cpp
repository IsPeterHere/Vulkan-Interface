#include "Camera.h"


void Camera::euler_update(double delta_yaw, double delta_pitch)
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

void Camera::position_update(glm::vec3 delta_position) 
{
	position += front * delta_position.x;
	position += right * delta_position.y;
}

void Camera::update_look_at()
{
	glm::mat3x3 rotation(glm::orientate3(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0.0)));
	front = glm::vec3(0.0, 1.0, 0.0) * rotation;
	right = glm::cross(front, true_up);
	look_at = glm::lookAt(position, position + front, true_up);
}
