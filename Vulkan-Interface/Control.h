#pragma once
#include <GLFW/glfw3.h>
#include"Camera.h"
#include <chrono>

class Control
{
public:
	static Control* makeControl(GLFWwindow* window);
	static void destroyControl();
	void update_camera(Camera* camera, double mouse_sensitivity, float movement_velocity);

	int width, height;
	double last_x_position, last_y_position;
	double x_mouse_movement, y_mouse_movement;
	bool mouse_normilized{ false };
	bool w, a, s, d;

private:
	static std::unique_ptr<Control> control;
	GLFWwindow* window;


	Control(GLFWwindow* window);

	static void mouse_movement(GLFWwindow* window, double xpos, double ypos);


	static void key_input(GLFWwindow* window, int key, int scancode, int action, int mods);


	static void window_resize(GLFWwindow* window, int width, int height);


	void bind();

};
