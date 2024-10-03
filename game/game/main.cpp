#define _USE_MATH_DEFINES

#include <windows.h>
#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdlib.h>

using namespace std;
using namespace std::chrono;

float radians(float degrees) {
	return degrees * (M_PI / 180);
}

struct rect {
	float x, y, vx, vy, speed;
	int w, h;

	rect(float x, float y, int w, int h, float speed = 0) : x(x), y(y), w(w), h(h), speed(speed), vx(cos(radians(45)) * speed), vy(-sin(radians(45)) * speed) {}

	rect() : x(-1), y(-1), w(-1), h(-1), speed(0), vx(0), vy(0) {}
};

void setWindow(int w, int h) {
	_COORD coord;
	coord.X = w;
	coord.Y = h;
	_SMALL_RECT Rect;
	Rect.Top = 0;
	Rect.Left = 0;
	Rect.Bottom = h - 1;
	Rect.Right = w - 1;
	HANDLE Handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleScreenBufferSize(Handle, coord);
	HWND consoleWindow = GetConsoleWindow();
	SetWindowPos(consoleWindow, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	SetConsoleWindowInfo(Handle, TRUE, &Rect);
}

void drawRect(wchar_t* screen, rect r, int w, char pixel) {
	for (int i = floor(r.y); i < floor(r.y + r.h); i++) {
		for (int j = floor(r.x); j < floor(r.x + r.w); j++) {
			screen[w * i + j] = pixel;
		}
	}
}

void clearScreen(wchar_t* screen, int width, int height) {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			screen[width * i + j] = ' ';
		}
	}
}

void init(rect* players, rect* powerups, rect* bricks, int brickW, int brickH, int width, int height, int powerupsCount) {
	for (int i = 0; i < powerupsCount; i++) {
		players[i].x = -1;
	}

	players[0] = rect(width / 2, height / 2, 3, 2, 0.3);

	for (int i = 0; i < powerupsCount; i++) {
		powerups[i].x = -1;
	}

	for (int i = 0; i < height / brickH / 3; i++) {
		for (int j = 0; j < width / brickW; j++) {
			bricks[width / brickW * i + j] = rect(j * brickW, i * brickH, brickW, brickH);
		}
	}
}

int sign(float val) {
	return (val == 0) ? 0 : (val > 0) ? 1 : -1;
}

int main() {
	int width = 120 * 2;
	int height = 30 * 2;
	setWindow(width, height);

	char symbols[] = { '4', '$', 'W', '@' };

	float fps = 240;

	int arrSize = width * height;
	wchar_t* screen = new wchar_t[arrSize];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	int powerupsCount = 20;

	rect* players = new rect[powerupsCount];
	float angle = radians(45);

	rect platform(0, height - 5, 25, 1);
	int platformSpeed = 0;
	float prevX = platform.x;

	int brickW = 10, brickH = 2;
	int brickCount = (width / brickW) * (height / brickH / 3);
	rect* bricks = new rect[brickCount];

	rect* powerups = new rect[powerupsCount];

	init(players, powerups, bricks, brickW, brickH, width, height, powerupsCount);

	while (true) {
		system_clock::time_point a = system_clock::now();

		clearScreen(screen, width, height);

		POINT mousePos;
		GetCursorPos(&mousePos);
		//float platformX = (float)width * ((float)mousePos.x / 1920);
		float miny1 = 0, minx1 = 0, miny2 = 0, minx2 = 0;
		for (int i = 0; i < powerupsCount; i++) {
			if (players[i].x != -1) {
				if (players[i].y > miny1) {
					miny1 = players[i].y;
					minx1 = players[i].x;
				}
			}
			if (powerups[i].x != -1) {
				if (powerups[i].y > miny2) {
					miny2 = powerups[i].y;
					minx2 = powerups[i].x;
				}
			}
		}
		float platformX = fmax(miny1, miny2) == miny1 ? minx1 : minx2;
		if (platformX - platform.w / 2 < 0) {
			platform.x = 0;
		}
		else if (platformX + platform.w / 2 > width) {
			platform.x = width - platform.w;
		}
		else {
			platform.x = platformX - platform.w / 2;
		}
		platformSpeed = platform.x - prevX;
		prevX = platform.x;

		for (int i = 0; i < powerupsCount; i++) {
			if (powerups[i].x != -1) {
				powerups[i].y += 0.13;
				if (powerups[i].x + 1 > platform.x && powerups[i].x < platform.x + platform.w && powerups[i].y + powerups[i].h - platform.y > 0 && powerups[i].y + powerups[i].h - platform.y < 0.3) {
					for (int j = 0; j < powerupsCount; j++) {
						if (players[j].x == -1) {
							players[j] = rect(powerups[i].x, powerups[i].y - 2, 3, 2, 0.3);
							break;
						}
					}
					powerups[i].x = -1;
				}
				else if(powerups[i].y + 2 > height)
					powerups[i].x = -1;
			}
		}

		for (int p = 0; p < powerupsCount; p++) {
			if (players[p].x != -1) {
				players[p].x += players[p].vx * (11.0f / 24.0f) * ((float)width / height);
				players[p].y += players[p].vy;

				if (players[p].x + 1 + players[p].w > width || players[p].x < 1) {
					players[p].vx *= -1;
				}
				else if (players[p].y < 1) {
					players[p].vy *= -1;
				}
				else if (players[p].y + 1 + players[p].h > height) {
					players[p].x = -1;
					int count = 0;
					for (int j = 0; j < powerupsCount; j++) {
						if (players[j].x != -1) {
							count++;
							break;
						}
					}
					if(count == 0)
						init(players, powerups, bricks, brickW, brickH, width, height, powerupsCount);
				}

				else if (players[p].x + players[p].w > platform.x && players[p].x < platform.x + platform.w && players[p].y + players[p].h - platform.y > 0 && players[p].y + players[p].h - platform.y <= players[p].vy) {
					players[p].vy *= -1;
					if (platformSpeed != 0) {
						if (sign(players[p].vx) != sign(platformSpeed)) {
							if (angle < radians(50))
								angle += radians(3);
							if (players[p].speed > 0.2)
								players[p].speed -= 0.03;
						}
						else {
							if (angle > radians(40))
								angle -= radians(3);
							if (players[p].speed < 0.5)
								players[p].speed += 0.03;
						}
						players[p].vy = -sin(angle) * players[p].speed;
						players[p].vx = cos(angle) * sign(players[p].vx) * players[p].speed;
					}
				}

				else {
					int bricksAlive = 0;
					for (int i = brickCount - 1; i << brickCount >= 0; i--) {
						if (bricks[i].x >= 0) {
							bricksAlive++;
							if (players[p].x + players[p].w > bricks[i].x && players[p].x < bricks[i].x + brickW && players[p].y + players[p].h > bricks[i].y && players[p].y < bricks[i].y + brickH) {
								float a1 = fmin(players[p].x + players[p].w - bricks[i].x, bricks[i].x + brickW - players[p].x);
								float a2 = fmin(players[p].y + players[p].h - bricks[i].y, bricks[i].y + brickH - players[p].y);

								if (a1 < a2) {
									players[p].vx *= -1;
								}
								else players[p].vy *= -1;

								if (rand() % 3 == 0) {
									for (int j = 0; j < powerupsCount; j++) {
										if (powerups[j].x == -1) {
											powerups[j] = rect(bricks[i].x + brickW / 2 - 0.5, bricks[i].y, 1, 1);
											break;
										}
									}
								}

								bricks[i].x = -1;

								break;
							}
						}
					}

					if (bricksAlive == 0) {
						init(players, powerups, bricks, brickW, brickH, width, height, powerupsCount);
					}
				}

				if(players[p].x != -1)
					drawRect(screen, players[p], width, '@');
			}
		}

		for (int i = 0; i < brickCount; i++) {
			if(bricks[i].x >= 0)
				drawRect(screen, bricks[i], width, symbols[i % 4]);
		}

		for (int i = 0; i < powerupsCount; i++) {
			if(powerups[i].x != -1)
				drawRect(screen, powerups[i], width, 'W');
		}

		drawRect(screen, platform, width, '@');

		WriteConsoleOutputCharacter(hConsole, screen, width * height, { 0, 0 }, &dwBytesWritten);

		system_clock::time_point b = system_clock::now();
		duration<double, milli> workTime = b - a;

		duration<double, milli> deltaMs(1000.0 / fps - workTime.count());
		auto deltaMsDur = duration_cast<milliseconds>(deltaMs);

		this_thread::sleep_for(milliseconds(deltaMsDur.count()));
	}

	return 0;
}