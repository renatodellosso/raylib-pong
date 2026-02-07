/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

const bool DEBUG_MODE = false;

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

float clamp(float val, float minVal, float maxVal) {
	return max(min(val, maxVal), minVal);
}

float randRange(float min, float max) {
	int num = rand();
	float val = ((float)num) / RAND_MAX;
	return val * (max - min) + min;
}

const Vector2 paddleSize = {
	10, 250
};

const float BALL_RADIUS = 10;
const float INITIAL_PADDLE_SPEED = 250;
const float INITIAL_BALL_SPEED = 150;

typedef struct {
	int width, height;

	Rectangle leftPaddle;
	Rectangle rightPaddle;
	float paddleSpeed;
	
	Vector2 ballPos;
	Vector2 ballVelocity;
	
	int score;

	bool collidedLastFrame;

	float predictedImpactY;
} Game;

void initWindow(Game* game) {
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(100, 100, "Pong");
	SetWindowPosition(GetScreenWidth() / 4, GetScreenHeight() / 4);

	int monitor = GetCurrentMonitor();
	game->width = GetMonitorWidth(monitor) * 4 / 5;
	game->height = GetMonitorHeight(monitor) * 4 / 5;

	SetWindowSize(game->width, game->height);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");
}

void serveBall(Game* game) {
	game->ballPos = (Vector2){
		game->width / 2, game->height / 2
	};
	
	float theta = randRange(PI / 8, PI * 3 / 8);
	int xSign = randRange(0, 1) < 0.5 ? 1 : -1;
	int ySign = randRange(0, 1) < 0.5 ? 1 : -1;

	game->ballVelocity = (Vector2){
		INITIAL_BALL_SPEED * cos(theta * ySign) * xSign, INITIAL_BALL_SPEED * sin(theta * ySign)
	};
}

void initGame(Game* game) {

	game->leftPaddle = (Rectangle){
		10, game->height / 2 - paddleSize.y / 2,
		paddleSize.x, paddleSize.y
	};

	game->rightPaddle = (Rectangle){
		game->width - paddleSize.x - 10, game->height / 2 - paddleSize.y / 2,
		paddleSize.x, paddleSize.y
	};

	game->paddleSpeed = INITIAL_PADDLE_SPEED;

	game->score = 0;

	serveBall(game);
}

void render(Game* game) {
	// drawing
	BeginDrawing();

	// Setup the back buffer for drawing (clear color and depth buffers)
	ClearBackground(BLACK);

	// draw some text using the default font

	DrawText(TextFormat("Score: %i", game->score), game->width / 2 - 50, 0, 20, WHITE);
	if (DEBUG_MODE)
		DrawText(TextFormat("Ball Velocity: %i, %i", (int)round(game->ballVelocity.x), (int)round(game->ballVelocity.y)), game->width / 2 - 50, 25, 15, WHITE);

	DrawRectangleRec(game->leftPaddle, WHITE);
	DrawRectangleRec(game->rightPaddle, WHITE);

	DrawCircleV(game->ballPos, BALL_RADIUS, YELLOW);

	if (DEBUG_MODE)
		DrawCircle(game->rightPaddle.x, game->predictedImpactY, BALL_RADIUS / 2, RED);

	// end the frame and get ready for the next one  (display frame, poll input, etc...)
	EndDrawing();
}

void handleInput(Game* game) {
	float deltaTime = GetFrameTime();

	if (IsKeyDown(KEY_S))
		game->leftPaddle.y = min(game->leftPaddle.y + deltaTime * game->paddleSpeed, game->height - paddleSize.y);
	else if (IsKeyDown(KEY_W))
		game->leftPaddle.y = max(game->leftPaddle.y - deltaTime * game->paddleSpeed, 0);
}

void handleCollisions(Game* game) {
	bool collided;
	Rectangle paddle;
	if (CheckCollisionCircleRec(game->ballPos, BALL_RADIUS, game->leftPaddle)) {
		collided = true;
		paddle = game->leftPaddle;
		game->score++;
	} else if (CheckCollisionCircleRec(game->ballPos, BALL_RADIUS, game->rightPaddle)) {
		collided = true;
		paddle = game->rightPaddle;
	} else collided = false;

	if (!game->collidedLastFrame && collided) {
		float diffY = paddle.y + paddleSize.y / 2 - game->ballPos.y;

		TraceLog(LOG_INFO, TextFormat("Collision with ball (%f, %f) and paddle (%f, %f, %f, %f). Y Diff: %f", 
			game->ballPos.x, game->ballPos.y, paddle.x, paddle.y, paddle.x + paddleSize.x, paddle.y + paddleSize.y, diffY));

		game->ballVelocity.y += diffY;

		game->ballVelocity.x *= -1.2;
		game->paddleSpeed *= 1.1;

		game->ballPos.x = clamp(game->ballPos.x, paddle.x - BALL_RADIUS, paddle.x + paddle.width + BALL_RADIUS);
	}

	game->collidedLastFrame = collided;

	if (game->ballPos.y > game->height || game->ballPos.y < 0) {
		game->ballPos.y = max(min(game->ballPos.y, game->height), 0);
		game->ballVelocity.y *= -1;
	}

	if (game->ballPos.x < 0) {
		// Reset
		initGame(game);
	} else if (game->ballPos.x > game->width) {
		// Right paddle missed
		game->score += 10;

		game->rightPaddle.y = game->height / 2 - game->rightPaddle.height / 2;

		serveBall(game);
		game->paddleSpeed = INITIAL_PADDLE_SPEED;
	}
}

void moveBall(Game* game) {
	float deltaTime = GetFrameTime();
	Vector2 scaledVelocity = {
		game->ballVelocity.x * deltaTime,
		game->ballVelocity.y * deltaTime
	};
	game->ballPos.x += scaledVelocity.x;
	game->ballPos.y += scaledVelocity.y;
}

void moveRightPaddle(Game* game) {
	if (game->ballVelocity.x < 0)
		return;

	float diffY = game->ballPos.y - game->rightPaddle.y;
	float diffX = game->rightPaddle.x - game->ballPos.x - BALL_RADIUS;
	float time = diffX / game->ballVelocity.x;

	float impactY = game->ballPos.y + game->ballVelocity.y * time;
	game->predictedImpactY = impactY;

	float movement = clamp(impactY - (game->rightPaddle.y + game->rightPaddle.height / 2), -game->paddleSpeed, game->paddleSpeed);
	game->rightPaddle.y += movement * GetFrameTime();
	game->rightPaddle.y = clamp(game->rightPaddle.y, 0, game->height - game->rightPaddle.height);
}

int main ()
{
	srand(time(NULL));
	rand(); // First number is always within a small range

	Game game = {};	

	initWindow(&game);
	initGame(&game);

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		render(&game);
		moveBall(&game);
		moveRightPaddle(&game);
		handleInput(&game);
		handleCollisions(&game);
	}

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
