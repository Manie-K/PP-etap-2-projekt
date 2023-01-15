#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "configuration.h"
#include "functions.h"
extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, quit = 0, frames = 0, mapDrawHeight = 0, playerHorizontalSpeed = 0;
	double deltaTime, worldTime = 0, fpsTimer = 0, fps = 0, mapSpeed = DEFAULT_SPEED, distance = 0, playerHorizontalBuff = 0, oneFrameDistance = 0;
	Rectangle_t grassAreas[GRASS_AREAS_COUNT];
	SDL_Event event; 
	SDL_Surface* screen = nullptr, * charset = nullptr, * map = nullptr, * playerCar = nullptr, * mapTEMP = nullptr, * enemyCar = nullptr;
	SDL_Surface* friendCarSurface = nullptr, * bulletBMP = nullptr, *powerupBMP = nullptr;
	SDL_Surface* surfaces[SURFACES_COUNT] = { screen, charset, map, playerCar, enemyCar, mapTEMP, friendCarSurface , bulletBMP, powerupBMP};
	SDL_Texture *screenTexture; 
	SDL_Window *window; 
	SDL_Renderer *renderer;
	Car_t player = initPlayer();
	PowerUp_t powerup;
	Colors_t colors; 
	Bullet_t bullet = initBullet(player, mapDrawHeight);
	NPC_t friendCar = setFriendCar(); 
	NPC_t enemies[ENEMIES_COUNT];
	bool killPlayer = false;

	if (initSDL(window, renderer, screen, screenTexture) == -1) return -1;
	initializeVariables(colors, screen, grassAreas, enemies, powerup);

	if (!setSurfaces(charset, playerCar, enemyCar, friendCarSurface, bulletBMP, powerupBMP)){
		destroySurfaces(surfaces);
		destroyBaseSDL(screenTexture, window, renderer);
		return -1;
	}
	map = generateMap(screen);
	mapTEMP = generateMap(screen);

	t1 = SDL_GetTicks();
	while (!quit) {
		SDL_FillRect(screen, NULL, colors.black);
		updateTime(t1, deltaTime, worldTime, fpsTimer, fps, frames);
		updateDistance(mapDrawHeight, oneFrameDistance, distance, deltaTime, mapSpeed);
		updateBullet(bullet, deltaTime, player, mapDrawHeight);
		updatePlayer(player, mapDrawHeight, playerHorizontalBuff, playerHorizontalSpeed, deltaTime, oneFrameDistance);
		updateFriend(friendCar, deltaTime, grassAreas, bullet, player);
		updateEnemies(enemies, deltaTime, grassAreas, bullet, player);
		updatePowerUp(player, powerup, bullet);
		SDL_BlitSurface(map, NULL, mapTEMP, NULL);

		if(powerup.exist)
			DrawSurface(map, powerupBMP, powerup.pos.x, MAP_HEIGHT - powerup.pos.y); //Wyswietlanie powerupa
		DrawSurface(map, friendCarSurface, friendCar.position.x, MAP_HEIGHT - friendCar.position.y); //Wyswietlanie neutralnego
		for (int i = 0; i < ENEMIES_COUNT; i++) 
			DrawSurface(map, enemyCar, enemies[i].position.x, MAP_HEIGHT - enemies[i].position.y); //Wyswietlanie wrogow
		DrawSurface(screen, map, SCREEN_WIDTH/2, - MAP_HEIGHT/2 + SCREEN_HEIGHT + mapDrawHeight);//Wyswietlenie tla
		SDL_BlitSurface(mapTEMP, NULL, map, NULL);
		DrawSurface(screen, playerCar,player.position.x, SCREEN_HEIGHT / 2);//Wyswietlenie samochodu gracza
		if (bullet.isShot)
			DrawSurface(screen, bulletBMP, bullet.screenPosition.x, bullet.screenPosition.y);//Wysweitlanie pocisku
  		drawMenu(screen, charset, colors, { player.score, fps, worldTime, distance,bullet }, player);//Wysweitlenie menu
		updateScreen(renderer, screen, screenTexture);//Update ekranu
		
		killPlayer = checkForNpcColissions(friendCar, enemies, player);
		if (checkForCollisions(player.boxModel, grassAreas) || killPlayer) {
			if (worldTime > TIME_THRESHOLD && player.carsLeft <= 0) {
				restartGame(player, mapDrawHeight, playerHorizontalSpeed, mapSpeed, distance, playerHorizontalBuff,bullet,powerup, false);
				restartTime(t1, frames, worldTime, fpsTimer, fps);
				lossInfo(quit, mapSpeed, t1, colors, { renderer,screen,charset,screenTexture });
			}
			else {
				restartGame(player, mapDrawHeight, playerHorizontalSpeed, mapSpeed, distance, playerHorizontalBuff,bullet,powerup, true);
				player.carsLeft = worldTime > TIME_THRESHOLD ? player.carsLeft - 1 : player.carsLeft;
			}
		}
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if (event.key.keysym.sym == SDLK_UP) mapSpeed = FAST_SPEED;
					else if (event.key.keysym.sym == SDLK_DOWN) mapSpeed = SLOW_SPEED;
					else if (event.key.keysym.sym == SDLK_LEFT) playerHorizontalSpeed = -DEFAULT_HORIZONTAL_SPEED;
					else if (event.key.keysym.sym == SDLK_RIGHT)playerHorizontalSpeed = DEFAULT_HORIZONTAL_SPEED;
					else if (event.key.keysym.sym == SDLK_n) {
						restartGame(player, mapDrawHeight, playerHorizontalSpeed, mapSpeed, distance, playerHorizontalBuff,bullet,powerup, false);
						restartTime(t1, frames, worldTime, fpsTimer, fps);
					}
					else if (event.key.keysym.sym == SDLK_p) pauseGame(quit, mapSpeed, t1, colors, { renderer,screen,charset,screenTexture });
					else if (event.key.keysym.sym == SDLK_SPACE) shoot(bullet, player, mapDrawHeight, powerup);
					break;
				case SDL_KEYUP:
					mapSpeed = DEFAULT_SPEED;
					playerHorizontalSpeed = 0;
					break;
				case SDL_QUIT:
					quit = 1;
					break;
			};
		};
		frames++;
	};

	destroySurfaces(surfaces);
	destroyBaseSDL(screenTexture, window, renderer);
	return 0;
};
