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


private:
	glm::vec3 position{ 2,2,2 };
	glm::vec3 true_up{ 0,0,1 };
	double pitch = 180, yaw = 0 ;

	glm::mat4x4 look_at{ glm::lookAt(position, {0,0,0}, true_up) };

	void update_look_at()
	{
		glm::mat3x3 rotation(glm::eulerAngleXY(pitch, yaw));
		glm::vec3 front = rotation * glm::vec3(0, 1, 0);
		//glm::vec3 right = glm::cross(front, true_up);
		look_at = glm::lookAt(position, position + front, true_up);
	}
};