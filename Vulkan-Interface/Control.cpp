#include"Control.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include<iostream>

std::unique_ptr<Control> Control::control;

Control::Control(GLFWwindow* window) : window(window) 
{
	glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_DISABLED);
	glfwGetWindowSize(window, &width, &height);
	bind();
}


Control* Control::makeControl(GLFWwindow* window)
{
	if (Control::control == nullptr)
		Control::control = std::unique_ptr<Control>(new Control(window));
	return Control::control.get();
}
void Control::destroyControl()
{
	Control::control = nullptr;
}



void Control::update_camera(Camera* camera, double mouse_sensitivity, float movement_velocity)
{
	static auto lastTime = std::chrono::high_resolution_clock::now();

	float time_ellapsed1 = 0;
	float time_ellapsed2 = 0;
	float time_ellapsed3 = 0;

	auto current_time = std::chrono::high_resolution_clock::now();
	float time_ellapsed = std::chrono::duration<float, std::chrono::seconds::period>(current_time - lastTime).count();

	time_ellapsed3 = time_ellapsed2;
	time_ellapsed2 = time_ellapsed1;
	time_ellapsed1 = time_ellapsed;

	lastTime = current_time;

	float avg_time_ellapsed = (time_ellapsed + time_ellapsed1 + time_ellapsed2 + time_ellapsed3) / (float) 4;

	double dx = x_mouse_movement * mouse_sensitivity * avg_time_ellapsed;
	double dy = y_mouse_movement * mouse_sensitivity * avg_time_ellapsed;

	x_mouse_movement = 0;
	y_mouse_movement = 0;

	camera->euler_update(dx, dy);

	glm::vec3 delta_position{0,0,0};
	if (w)
		delta_position.x += movement_velocity* avg_time_ellapsed;
	if (s)
		delta_position.x += -movement_velocity* avg_time_ellapsed;
	if (a)
		delta_position.y += -movement_velocity* avg_time_ellapsed;
	if (d)
		delta_position.y += movement_velocity* avg_time_ellapsed;

	camera->position_update(delta_position);
}

void Control::mouse_movement(GLFWwindow* window, double xpos, double ypos)
{
	if (control->mouse_normilized)
	{
		control->x_mouse_movement = xpos - control->last_x_position;
		control->y_mouse_movement = ypos - control->last_y_position;
	}

	control->last_x_position = xpos;
	control->last_y_position = ypos;

	control->mouse_normilized = true;
}

void Control::key_input(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	else
	{
		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case GLFW_KEY_W:
				control->w = true;
				break;
			case GLFW_KEY_A:
				control->a = true;
				break;
			case GLFW_KEY_S:
				control->s = true;
				break;
			case GLFW_KEY_D:
				control->d = true;
				break;
			default:
				break;
			}
		}

		else if (action == GLFW_RELEASE)
		{
			switch (key)
			{
			case GLFW_KEY_W:
				control->w = false;
				break;
			case GLFW_KEY_A:
				control->a = false;
				break;
			case GLFW_KEY_S:
				control->s = false;
				break;
			case GLFW_KEY_D:
				control->d = false;
				break;
			default:
				break;
			}
		}
	}
}

void Control::window_resize(GLFWwindow* window, int width, int height)
{
	control->width = width;
	control->height = height;
	control->mouse_normilized = false;
}

void Control::bind()
{
	glfwSetKeyCallback(window, Control::key_input);
	glfwSetCursorPosCallback(window, Control::mouse_movement);
	glfwSetWindowSizeCallback(window, Control::window_resize);
}
