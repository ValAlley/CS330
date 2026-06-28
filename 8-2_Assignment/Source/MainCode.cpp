#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
#include <cmath> // For sqrt

using namespace std;

const float DEG2RAD = 3.14159f / 180.0f;

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
	float red, green, blue;
	float x, y, width, height;
	int health; // Requires multiple hits to destroy
	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float hh, float rr, float gg, float bb, int hp = 1)
	{
		brick_type = bt;
		x = xx; y = yy;
		width = ww; height = hh;
		red = rr; green = gg; blue = bb;
		health = hp;
		onoff = ON;
	}

	void drawBrick()
	{
		if (onoff == ON)
		{
			float halfWidth = width / 2;
			float halfHeight = height / 2;

			// Change color if damaged (1 health left)
			if (brick_type == DESTRUCTABLE && health == 1) {
				glColor3d(red * 0.4, green * 0.4, blue * 0.4); // Darken texture to simulate damage
			}
			else {
				glColor3d(red, green, blue);
			}

			glBegin(GL_POLYGON);
			glVertex2d(x + halfWidth, y + halfHeight);
			glVertex2d(x + halfWidth, y - halfHeight);
			glVertex2d(x - halfWidth, y - halfHeight);
			glVertex2d(x - halfWidth, y + halfHeight);
			glEnd();
		}
	}
};

class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float vx, vy;

	Circle(float xx, float yy, float rr, float initial_vx, float initial_vy, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		vx = initial_vx;
		vy = initial_vy;
		red = r;
		green = g;
		blue = b;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->onoff == OFF) return;

		float hw = brk->width / 2.0f;
		float hh = brk->height / 2.0f;

		if ((x > brk->x - hw && x <= brk->x + hw) && (y > brk->y - hh && y <= brk->y + hh))
		{
			vy = -vy;
			y += vy * 2.0f; // Nudge ball out of the brick

			if (brk->brick_type == DESTRUCTABLE)
			{
				brk->health--;
				if (brk->health <= 0) {
					brk->onoff = OFF;
				}
			}
		}
	}

	void MoveOneStep()
	{
		x += vx;
		y += vy;

		if (x + radius >= 1.0f) {
			x = 1.0f - radius;
			vx = -vx;
		}
		if (x - radius <= -1.0f) {
			x = -1.0f + radius;
			vx = -vx;
		}
		if (y + radius >= 1.0f) {
			y = 1.0f - radius;
			vy = -vy;
		}
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};

vector<Circle> world;
vector<Brick> bricks;
Brick paddle(REFLECTIVE, 0.0f, -0.9f, 0.4f, 0.1f, 0.8f, 0.8f, 0.8f);

void processInput(GLFWwindow* window, Brick& paddle);

// NEW: Function to check if all destructible bricks are destroyed
bool checkWinCondition(const vector<Brick>& brickList) {
	for (size_t i = 0; i < brickList.size(); i++) {
		// If even one destructible brick is still ON, the game isn't won yet
		if (brickList[i].brick_type == DESTRUCTABLE && brickList[i].onoff == ON) {
			return false;
		}
	}
	return true; // All destructible bricks are OFF!
}

int main(void) {
	srand((unsigned int)time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(600, 600, "8-2 Assignment", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Generate an engaging brick layout (Grid structure)
	float startX = -0.8f;
	float startY = 0.8f;
	for (int row = 0; row < 3; row++) {
		for (int col = 0; col < 6; col++) {
			float r = (float)rand() / RAND_MAX;
			float g = (float)rand() / RAND_MAX;
			float b = (float)rand() / RAND_MAX;

			bricks.push_back(Brick(DESTRUCTABLE, startX + (col * 0.32f), startY - (row * 0.15f), 0.3f, 0.1f, r, g, b, 2));
		}
	}

	bool gameWon = false;

	while (!glfwWindowShouldClose(window)) {
		//Setup View
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		// Check for win condition if we haven't won yet
		if (!gameWon) {
			gameWon = checkWinCondition(bricks);
			if (gameWon) {
				// We just won! Change title and print to console.
				glfwSetWindowTitle(window, "8-2 Assignment - CONGRATULATIONS YOU WIN!!!");
				cout << "\n==================================\n";
				cout << "    CONGRATULATIONS! YOU WIN!!!   \n";
				cout << "==================================\n";
			}
		}

		// Draw Screen based on Win State
		if (gameWon) {
			glClearColor(0.0f, 0.3f, 0.0f, 1.0f); // Dark Green background for winning
			glClear(GL_COLOR_BUFFER_BIT);
			paddle.drawBrick(); // Just draw the paddle for aesthetic
		}
		else {
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Normal black background
			glClear(GL_COLOR_BUFFER_BIT);

			processInput(window, paddle);

			// Circle interactions & Movements
			for (size_t i = 0; i < world.size(); i++)
			{
				if (world[i].y < -1.1f) {
					world.erase(world.begin() + i);
					i--;
					continue;
				}

				for (size_t b = 0; b < bricks.size(); b++) {
					world[i].CheckCollision(&bricks[b]);
				}

				world[i].CheckCollision(&paddle);
				world[i].MoveOneStep();

				for (size_t j = i + 1; j < world.size(); j++) {
					float dx = world[i].x - world[j].x;
					float dy = world[i].y - world[j].y;
					float distance = sqrt(dx * dx + dy * dy);

					if (distance < world[i].radius + world[j].radius) {
						swap(world[i].vx, world[j].vx);
						swap(world[i].vy, world[j].vy);

						world[i].red = 1.0f; world[i].green = 1.0f; world[i].blue = 1.0f;
						world[j].red = 1.0f; world[j].green = 1.0f; world[j].blue = 1.0f;

						world[i].x += world[i].vx;
						world[i].y += world[i].vy;
					}
				}
				world[i].DrawCircle();
			}

			for (size_t b = 0; b < bricks.size(); b++) {
				bricks[b].drawBrick();
			}
			paddle.drawBrick();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window, Brick& paddle)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (paddle.x - (paddle.width / 2) > -1.0f) paddle.x -= 0.015f;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (paddle.x + (paddle.width / 2) < 1.0f) paddle.x += 0.015f;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		float r = (float)rand() / RAND_MAX;
		float g = (float)rand() / RAND_MAX;
		float b = (float)rand() / RAND_MAX;

		float vx = ((float)(rand() % 100) / 6000.0f) - 0.008f;
		float vy = 0.008f;

		Circle C(paddle.x, paddle.y + 0.15f, 0.03f, vx, vy, r, g, b);
		world.push_back(C);
		Sleep(200);
	}
}