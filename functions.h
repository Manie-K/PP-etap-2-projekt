#pragma once
#include "configuration.h"
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

struct Point_t {
	int x, y;
};
struct Rectangle_t {
	Point_t topLeft;
	int w, h;
};
struct Car_t {
	Point_t position;
	Rectangle_t boxModel;
	int score, carsLeft, recentKill, recentDeath;
	double scoreBuff;
	int enemyInflictedSideSpeed;
	bool ghost;
	int scoreForNewCar;
};
struct Colors_t {
	int black, green, red, blue;
};
struct GraphicsUpdate_t
{
	SDL_Renderer* renderer;
	SDL_Surface* screen, *charset;
	SDL_Texture* screenTexture;
};
struct NPC_t {
	Point_t position;
	double speed;
	Rectangle_t boxModel;
	int sideSpeed;
	bool ghost;
};
struct Bullet_t {
	Point_t mapPosition;
	Point_t screenPosition;
	int speed;
	bool isShot;
	Rectangle_t bulletBoxModel;
	int specialAmmo;
};
struct PowerUp_t {
	Point_t pos;
	Rectangle_t powerUpBoxModel;
	bool exist;
};
struct MenuData_t
{
	int score;
	double fps, worldTime, distance;
	Bullet_t bullet;
};

void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset);
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y);
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color);
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color);
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor);

int initSDL(SDL_Window* &window, SDL_Renderer* &renderer, SDL_Surface* &screen, SDL_Texture* &screenTexture);
void updateScreen(SDL_Renderer* &renderer, SDL_Surface* &screen, SDL_Texture* &screenTexture);
Car_t initPlayer();
int updatePlayerHorizontalPosition(double& buffer, int speed, double delta);
void setGrassAreas(Rectangle_t* grassAreas);
Rectangle_t setBoxModel(Point_t position);
void updatePlayer(Car_t &player ,int mapDrawHeight, double &playerHorizontalBuffer, int playerHorizontalSpeed, double deltaTime, double oneFrameDistance);
bool checkForCollisions(Rectangle_t player, Rectangle_t* grassAreas);
bool rectanglesCollide(Point_t A_topLeft, Point_t A_bottomRight, Point_t B_topLeft, Point_t B_bottomRight);
void destroyBaseSDL(SDL_Texture* &screenTexture, SDL_Window*& window, SDL_Renderer*& renderer);
void destroySurfaces(SDL_Surface** surfaces);
Colors_t setColors(SDL_Surface* screen);
bool setSurfaces(SDL_Surface* &charset, SDL_Surface* &playerCar, SDL_Surface*&enemyCar,
	SDL_Surface*& friendCarSurface, SDL_Surface*& bullet, SDL_Surface*& powerup);
void updateTime(int &t1, double &deltaTime,double  &worldTime, double &fpsTimer, double &fps, int &frames);
void drawPauseInfo(SDL_Surface* screen, SDL_Surface* charset, Colors_t colors);
void pauseGame(int& quit, double& mapSpeed, int &t1, Colors_t colors, GraphicsUpdate_t g);
void drawFunctionalities(SDL_Surface* screen, SDL_Surface* charset, Colors_t colors);
void drawMenu(SDL_Surface* screen, SDL_Surface* charset, Colors_t colors, MenuData_t data, Car_t& player);
void restartGame(Car_t& player, int& mapDrawHeight, int& phs, double& mapSpeed, double& dis, double& phb, Bullet_t& bullet, PowerUp_t& powerup, bool keepAlive);
void restartTime(int& t1, int& frames, double& worldTime, double& fpsTimer, double& fps);
void drawLossInfo(SDL_Surface* screen, SDL_Surface* charset, Colors_t colors);
void lossInfo(int& quit, double& mapSpeed, int& t1, Colors_t colors, GraphicsUpdate_t g);
void updateDistance(int& mapDrawHeight, double& oneFrameDistance, double& distance, double deltaTime, double mapSpeed);
NPC_t setFriendCar();
void killNPC(NPC_t& npc, Car_t& player);
void updateFriend(NPC_t &friendCar, double deltaTime, Rectangle_t* grassAreas, Bullet_t& bullet, Car_t& player);
void setEnemies(NPC_t* enemies);
void updateEnemies(NPC_t* enemies, double deltaTime, Rectangle_t* grassAreas, Bullet_t& bullet, Car_t& player);
bool checkFriendCollission(NPC_t& friendCar, Car_t &player);
bool checkEnemyCollission(NPC_t& enemy, Car_t& player);
bool checkForNpcColissions(NPC_t& friendCar, NPC_t* enemies, Car_t& player);
void initializeVariables(Colors_t &colors,SDL_Surface* screen,Rectangle_t* grassAreas, NPC_t *enemies, PowerUp_t& powerup);
Rectangle_t setBulletBoxModel(Point_t pos);
Bullet_t initBullet(Car_t player, int mapY);
void updateBullet(Bullet_t& bullet, double deltaTime, Car_t player, int mapY);
void shoot(Bullet_t& bullet, Car_t player, int mapY, PowerUp_t& powerup);
void regeneratePowerUp(PowerUp_t&powerup);
void updatePowerUp(Car_t player, PowerUp_t& powerup, Bullet_t& bullet);
SDL_Surface* generateMap(SDL_Surface* screen);