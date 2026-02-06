/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/
#include <stdio.h>
#include "raylib.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

float min(float x, float y) {
	if (x < y)
		return x;
	return y;
}

float max(float x, float y) {
	if (x > y)
		return x;
	return y;
}

const Vector2 paddleSize = {
	10, 250
};

const float BALL_RADIUS = 10;
const float PADDLE_SPEED = 250;

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);


	// Create the window and OpenGL context
	InitWindow(100, 100, "Pong");
	SetWindowPosition(GetScreenWidth() / 4, GetScreenHeight() / 4);

	int monitor = GetCurrentMonitor();
	int width = GetMonitorWidth(monitor) * 4 / 5, height = GetMonitorHeight(monitor) * 4 / 5;

	char str[20];

	SetWindowSize(width, height);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	Rectangle leftPaddle = {
		10, height / 2 - paddleSize.y / 2,
		paddleSize.x, paddleSize.y
	};

	Rectangle rightPaddle = {
		width - paddleSize.x - 10, height / 2 - paddleSize.y / 2,
		paddleSize.x, paddleSize.y
	};

	Vector2 ballPos = {
		width / 2, height / 2
	};
	Vector2 ballVelocity = {
		150, 150
	};

	int score = 0;
	bool prevCollided;

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		// draw some text using the default font

		DrawText(TextFormat("Score: %i", score), width / 2 - 50, 0, 20, WHITE);

		DrawRectangleRec(leftPaddle, WHITE);
		DrawRectangleRec(rightPaddle, WHITE);

		DrawCircleV(ballPos, BALL_RADIUS, YELLOW);

		float deltaTime = GetFrameTime();
		Vector2 scaledVelocity = {
			ballVelocity.x * deltaTime,
			ballVelocity.y * deltaTime
		};
		ballPos.x += scaledVelocity.x;
		ballPos.y += scaledVelocity.y;

		if (IsKeyDown(KEY_S))
			leftPaddle.y = min(leftPaddle.y + deltaTime * PADDLE_SPEED, height - paddleSize.y);
		else if (IsKeyDown(KEY_W))
			leftPaddle.y = max(leftPaddle.y - deltaTime * PADDLE_SPEED, 0);

		if (IsKeyDown(KEY_DOWN))
			rightPaddle.y = min(rightPaddle.y + deltaTime * PADDLE_SPEED, height - paddleSize.y);
		else if (IsKeyDown(KEY_UP))
			rightPaddle.y = max(rightPaddle.y - deltaTime * PADDLE_SPEED, 0);

		bool collided;
		Rectangle paddle;
		if (CheckCollisionCircleRec(ballPos, BALL_RADIUS, leftPaddle)) {
			collided = true;
			paddle = leftPaddle;
		} else if (CheckCollisionCircleRec(ballPos, BALL_RADIUS, rightPaddle)) {
			collided = true;
			paddle = rightPaddle;
		} else collided = false;

		if (!prevCollided && collided) {
			float diffY = paddle.y + paddleSize.y / 2 - ballPos.y;

			TraceLog(LOG_INFO, TextFormat("Collision with ball (%f, %f) and paddle (%f, %f, %f, %f). Y Diff: %f", 
				ballPos.x, ballPos.y, paddle.x, paddle.y, paddle.x + paddleSize.x, paddle.y + paddleSize.y, diffY));

			ballVelocity.y += diffY / 10000;

			ballVelocity.x *= -1.1;
			score++;

			ballPos.x = min(max(ballPos.x, paddle.x - BALL_RADIUS), paddle.x + paddle.width + BALL_RADIUS);
		}

		prevCollided = collided;

		if (ballPos.y > height || ballPos.y < 0) {
			ballPos.y = max(min(ballPos.y, height), 0);
			ballVelocity.y *= -1;
		}

		if (ballPos.x < 0 || ballPos.x > width)
			ballPos.x = width / 2;
		
		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
