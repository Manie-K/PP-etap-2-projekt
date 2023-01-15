#include "functions.h"

void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
}
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
}
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
}
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
}
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
}

int initSDL(SDL_Window*& window, SDL_Renderer*& renderer, SDL_Surface*& screen, SDL_Texture*& screenTexture)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) 
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return -1;
	}

	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer) != 0)
	{
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return -1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_SetWindowTitle(window, WINDOW_TITLE);

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	return 1;
}
void updateScreen(SDL_Renderer*& renderer, SDL_Surface*& screen, SDL_Texture*& screenTexture) 
{
	SDL_UpdateTexture(screenTexture, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
	SDL_RenderPresent(renderer);
}
Car_t initPlayer()
{
	Car_t temp;
	temp.position.x = SCREEN_WIDTH / 2 ;
	temp.position.y = SCREEN_HEIGHT / 2;
	temp.boxModel = setBoxModel(temp.position);
	temp.score = 0;
	temp.scoreBuff = 0;
	temp.carsLeft = STARTING_CARS;
	temp.enemyInflictedSideSpeed = 0;
	temp.ghost = false;
	temp.recentKill = 0;
	temp.recentDeath = 0;
	temp.scoreForNewCar = 0;
	return temp;
}
int updatePlayerHorizontalPosition(double& buffer, int speed, double delta)
{
	buffer += speed * delta * HORIZONTAL_SPEED_MULTIPLIER;
	if (buffer >= 1) {
		buffer--;
		return 1;
	}
	if (buffer <= -1) {
		buffer++;
		return -1;
	}
	return 0;
}
void setGrassAreas(Rectangle_t* grassAreas)
{
	int h = SCREEN_HEIGHT;
	int w = TILE_WIDTH;
	int i = 0;
	grassAreas[i++] = { 7 * w,3 * h,2 * w,h };
	grassAreas[i++] = {0,h,5*w,h};
	grassAreas[i++] = {11*w,h,5*w,h};
	grassAreas[i++] = {0,7*h,5*w,3*h};
	grassAreas[i++] = {11*w,7*h,5*w,3*h};
	grassAreas[i++] = {0,4*h,3*w,3*h};
	grassAreas[i++] = {13*w,4*h,3 * w,3*h};
}
Rectangle_t setBoxModel(Point_t position)
{
	Rectangle_t temp;
	temp.topLeft.x = position.x - CAR_WIDTH / 2;
	temp.topLeft.y = position.y + CAR_HEIGHT / 2;
	temp.h = CAR_HEIGHT;
	temp.w = CAR_WIDTH;
	return temp;
}
bool rectanglesCollide(Point_t A_topLeft, Point_t A_bottomRight, Point_t B_topLeft, Point_t B_bottomRight) {
	if (A_bottomRight.y > B_topLeft.y || B_bottomRight.y > A_topLeft.y) return false;
	if (A_topLeft.x > B_bottomRight.x || B_topLeft.x > A_bottomRight.x) return false;
	return true;
}
void updatePlayer(Car_t &player, int mapDrawHeight, double &playerHorizontalBuffer, int playerHorizontalSpeed, double deltaTime, double oneFrameDistance)
{
	player.position.y = mapDrawHeight + SCREEN_HEIGHT/2;
	if (player.enemyInflictedSideSpeed!=0)playerHorizontalSpeed = player.enemyInflictedSideSpeed;
	player.position.x += updatePlayerHorizontalPosition(playerHorizontalBuffer, playerHorizontalSpeed, deltaTime);
	player.boxModel = setBoxModel(player.position);
	player.scoreBuff += oneFrameDistance * SCORE_MULTIPLIER;
	if (player.scoreBuff > SCORE_TRESHOLD){
		player.score += SCORE_TRESHOLD;
		player.scoreForNewCar += SCORE_TRESHOLD;
		player.scoreBuff -= SCORE_TRESHOLD;
	}
	if (player.scoreForNewCar >= SCORE_PER_NEW_CAR) {
		player.carsLeft++;
		player.scoreForNewCar -= SCORE_PER_NEW_CAR;
	}
	
}
bool checkForCollisions(Rectangle_t player, Rectangle_t* grassAreas)
{
	for (int i = 0; i < GRASS_AREAS_COUNT; i++)
	{
		Point_t grassTopLeft = grassAreas[i].topLeft;
		Point_t grassBottomRight = { grassAreas[i].topLeft.x+ grassAreas[i].w, grassAreas[i].topLeft.y - grassAreas[i].h};
		if (rectanglesCollide(player.topLeft, { player.topLeft.x + player.w,player.topLeft.y - player.h }, grassTopLeft, grassBottomRight)) return true;
	}
	return false;
}
void destroyBaseSDL(SDL_Texture* &screenTexture, SDL_Window* &window, SDL_Renderer*& renderer) 
{
	SDL_DestroyTexture(screenTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
void destroySurfaces(SDL_Surface**surfaces)
{
	int i = 0;
	while(i < SURFACES_COUNT)
		SDL_FreeSurface(surfaces[i++]);
}
Colors_t setColors(SDL_Surface* screen)
{
	Colors_t temp;
	temp.black = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	temp.green = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	temp.red = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	temp.blue = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	return temp;
}
bool setSurfaces(SDL_Surface* &charset, SDL_Surface* &playerCar, SDL_Surface*& enemyCar, SDL_Surface*&friendCarSurface,SDL_Surface*& bullet, SDL_Surface*& powerup)
{
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset != NULL)SDL_SetColorKey(charset, true, 0x000000);
	playerCar = SDL_LoadBMP("./playerCar.bmp");
	enemyCar = SDL_LoadBMP("./enemyCar.bmp");
	friendCarSurface = SDL_LoadBMP("./friendCar.bmp");
	bullet = SDL_LoadBMP("./bullet.bmp");
	powerup = SDL_LoadBMP("./powerUp.bmp");
	return (charset != NULL && playerCar != NULL && enemyCar != NULL && friendCarSurface != NULL && bullet != NULL && powerup != NULL);
}
void updateTime(int& t1, double& deltaTime, double& worldTime, double& fpsTimer, double& fps, int& frames)
{
	int t2 = SDL_GetTicks();
	deltaTime = (t2 - t1) * 0.001;
	t1 = t2;
	worldTime += deltaTime;
	fpsTimer += deltaTime;
	if (fpsTimer > 0.5)
	{
		fps = frames * 2;
		frames = 0;
		fpsTimer -= 0.5;
	}
}
void drawPauseInfo(SDL_Surface* screen, SDL_Surface* charset, Colors_t colors)
{
	char text[TEXT_BUFFER_LENGTH];
	const int y = 20;
	DrawRectangle(screen, (SCREEN_WIDTH - PAUSE_INFO_SIZE) / 2, (SCREEN_HEIGHT - PAUSE_INFO_SIZE)/2, PAUSE_INFO_SIZE, PAUSE_INFO_SIZE, colors.green, colors.black);
	sprintf(text, "ZAPAUZOWANO");
	DrawString(screen, (SCREEN_WIDTH - strlen(text) * CHARACTER_WIDTH) / 2, SCREEN_HEIGHT / 2 - y, text, charset);
	sprintf(text, "ESC - Wyjdz");
	DrawString(screen, (SCREEN_WIDTH - strlen(text) * CHARACTER_WIDTH) / 2, SCREEN_HEIGHT / 2, text, charset);
	sprintf(text, "P - Wznow");
	DrawString(screen, (SCREEN_WIDTH - strlen(text) * CHARACTER_WIDTH) / 2, SCREEN_HEIGHT / 2 + y, text, charset);
}
void pauseGame(int& quit, double& mapSpeed, int& t1, Colors_t colors, GraphicsUpdate_t g)
{
	mapSpeed = 0;
	SDL_Event event;
	bool p = true;
	drawPauseInfo(g.screen, g.charset, colors);
	updateScreen(g.renderer, g.screen, g.screenTexture);
	while (p) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p)
				p = false;
			else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
			{
				p = false;
				quit = 1;
			}
		}
	}
	t1 = SDL_GetTicks();
}
void drawFunctionalities(SDL_Surface* screen, SDL_Surface* charset, Colors_t colors)
{
	char text[TEXT_BUFFER_LENGTH];
	DrawRectangle(screen, FUNC_MENU_X, FUNC_MENU_Y, FUNC_MENU_W, FUNC_MENU_H, colors.red, colors.blue);
	sprintf(text, "A, B, C, D, E, F");
	DrawString(screen, FUNC_MENU_X + (FUNC_MENU_W - strlen(text) * CHARACTER_WIDTH) / 2, FUNC_MENU_Y + MENU_PADDING, text, charset);
	sprintf(text, "H I K L M N");
	DrawString(screen, FUNC_MENU_X + (FUNC_MENU_W - strlen(text) * CHARACTER_WIDTH) / 2, FUNC_MENU_Y + 2.5 * MENU_PADDING, text, charset);
}
void drawMenu(SDL_Surface* screen, SDL_Surface* charset, Colors_t colors, MenuData_t data, Car_t& player)
{
	drawFunctionalities(screen, charset, colors);
	char text[TEXT_BUFFER_LENGTH];
	int i = 1;
	int insideColor = colors.black, outsideColor = colors.blue;
	if (data.worldTime <= TIME_THRESHOLD)outsideColor = colors.green;
	if (player.recentDeath-- > 0 || player.scoreBuff < 0)
		insideColor = colors.red;
	else if (player.recentKill-- > 0)
		insideColor = colors.green;

	DrawRectangle(screen, MENU_X, MENU_Y, SCREEN_WIDTH - (MENU_X * 2), MENU_H, outsideColor, insideColor);
	sprintf(text, WINDOW_TITLE);
	DrawString(screen, (SCREEN_WIDTH - strlen(text) * CHARACTER_WIDTH) / 2, MENU_Y + i * MENU_PADDING, text, charset);
	i += 1.5;
	sprintf(text, "Dystans = %.1lf, Wynik = %d, Czas = %.1lf s, %.0lf klatek/s", data.distance,data.score, data.worldTime, data.fps);
	DrawString(screen, (SCREEN_WIDTH - strlen(text) * CHARACTER_WIDTH) / 2, MENU_Y + i * MENU_PADDING, text, charset);
	i += 1.5;
	sprintf(text, "Esc - wyjscie, P - pauza, N - nowa gra ,  Pozostalo samochodow: %d", player.carsLeft);
	DrawString(screen, (SCREEN_WIDTH - strlen(text) * CHARACTER_WIDTH) / 2, MENU_Y + i * MENU_PADDING, text, charset);
	i += 1.5;
	sprintf(text, "%s przyspieszenie, %s zwolnienie, %s lewo, %s prawo, Spacja - strzal", ARROW_UP_CHAR, ARROW_DOWN_CHAR, ARROW_LEFT_CHAR, ARROW_RIGHT_CHAR);
	DrawString(screen, (SCREEN_WIDTH - strlen(text) * CHARACTER_WIDTH) / 2, MENU_Y + i * MENU_PADDING, text, charset);
	for (int i = 0; i < data.bullet.specialAmmo; i++) {
		DrawRectangle(screen, AMMO_INFO_OFFSET + i * 1.5 * AMMO_INFO_SIZE, SCREEN_HEIGHT - AMMO_INFO_OFFSET-AMMO_INFO_SIZE, AMMO_INFO_SIZE, AMMO_INFO_SIZE, colors.black, colors.red);
	}
}
void restartGame(Car_t& player, int& mapDrawHeight, int& phs, double& mapSpeed, double& dis, double& phb,Bullet_t &bullet,PowerUp_t &powerup ,bool keepAlive)
{
	double temp1 = player.score, temp2 = dis;
	int temp3 = player.carsLeft, temp4 = player.scoreForNewCar;
	player = initPlayer();
	bullet.specialAmmo = 0;
	regeneratePowerUp(powerup);
	dis = 0;
	mapDrawHeight = 0;
	phs = 0;
	mapSpeed = DEFAULT_SPEED;
	phb = 0;
	player.recentDeath = KILL_INFO_TIME;
	if(keepAlive){
		player.score = temp1;
		dis = temp2;
		player.position.x = SAFE_POSITION_X;
		player.carsLeft = temp3;
		player.scoreForNewCar = temp4;
	}
}
void restartTime(int& t1,int& frames, double& worldTime, double& fpsTimer, double& fps)
{
	frames = 0;
	worldTime = 0;
	fpsTimer = 0;
	fps = 0;
	t1 = SDL_GetTicks();
}
void drawLossInfo(SDL_Surface* screen, SDL_Surface* charset, Colors_t colors)
{
	char text[TEXT_BUFFER_LENGTH];
	const int y = 20;
	DrawRectangle(screen, (SCREEN_WIDTH - PAUSE_INFO_SIZE) / 2, (SCREEN_HEIGHT - PAUSE_INFO_SIZE) / 2, PAUSE_INFO_SIZE, PAUSE_INFO_SIZE, 
		colors.red, colors.black);
	sprintf(text, "PRZEGRANA");
	DrawString(screen, (SCREEN_WIDTH - strlen(text) * CHARACTER_WIDTH) / 2, SCREEN_HEIGHT / 2 - y, text, charset);
	sprintf(text, "ESC - Wyjdz");
	DrawString(screen, (SCREEN_WIDTH - strlen(text) * CHARACTER_WIDTH) / 2, SCREEN_HEIGHT / 2, text, charset);
	sprintf(text, "N - Start");
	DrawString(screen, (SCREEN_WIDTH - strlen(text) * CHARACTER_WIDTH) / 2, SCREEN_HEIGHT / 2 + y, text, charset);
}
void lossInfo(int& quit, double& mapSpeed, int& t1, Colors_t colors, GraphicsUpdate_t g)
{
	mapSpeed = 0;
	SDL_Event event;
	bool p = true;
	drawLossInfo(g.screen, g.charset, colors);
	updateScreen(g.renderer, g.screen, g.screenTexture);
	while (p) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_n)
				p = false;
			else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
			{
				p = false;
				quit = 1;
			}
		}
	}
	t1 = SDL_GetTicks();
}
void updateDistance(int& mapDrawHeight, double& oneFrameDistance, double& distance, double deltaTime, double mapSpeed)
{
	oneFrameDistance = (mapSpeed * SPEED_MULTIPLIER) * deltaTime;
	distance += oneFrameDistance;
	mapDrawHeight = ceil(distance);
	mapDrawHeight %= MAP_TILES;
}
NPC_t setFriendCar()
{
	NPC_t temp;
	temp.position = { FRIEND_DEFAULT_X, FRIEND_DEFAULT_Y };
	temp.boxModel = setBoxModel(temp.position);
	temp.speed = FRIEND_SPEED;
	temp.sideSpeed = 0;
	temp.ghost = false;
	return temp;
}
void updateFriend(NPC_t& friendCar, double deltaTime, Rectangle_t* grassAreas, Bullet_t& bullet, Car_t& player)
{
	friendCar.position.y += ceil(deltaTime*SPEED_MULTIPLIER*friendCar.speed);
	friendCar.position.x += ceil(deltaTime*SPEED_MULTIPLIER*friendCar.sideSpeed);
	friendCar.position.y %= MAP_TILES;
	friendCar.boxModel = setBoxModel(friendCar.position);
	if (checkForCollisions(friendCar.boxModel, grassAreas))
		friendCar = setFriendCar();
	if (friendCar.ghost || !bullet.isShot) return;
	Point_t friendBottomRight = { friendCar.boxModel.topLeft.x + friendCar.boxModel.w,friendCar.boxModel.topLeft.y - friendCar.boxModel.h };
	Point_t bulletBottomRight = { bullet.bulletBoxModel.topLeft.x + bullet.bulletBoxModel.w,bullet.bulletBoxModel.topLeft.y - bullet.bulletBoxModel.h };
	if (rectanglesCollide(friendCar.boxModel.topLeft,friendBottomRight, bullet.bulletBoxModel.topLeft , bulletBottomRight))
	{
		bullet.isShot = false;
		player.scoreBuff -= SCORE_PENALTY_CIVILIANS;
		friendCar = setFriendCar();
	}
}
void setEnemies(NPC_t* enemies)
{
	enemies[0].position = { ENEMY_1_DEFAULT_X, ENEMY_1_DEFAULT_Y };
	enemies[0].speed = ENEMY_1_SPEED;
	enemies[1].position = { ENEMY_2_DEFAULT_X, ENEMY_2_DEFAULT_Y };
	enemies[1].speed = ENEMY_2_SPEED;
	for (int i = 0; i < ENEMIES_COUNT; i++) {
		enemies[i].ghost = false;
		enemies[i].sideSpeed = 0;
		enemies[i].boxModel = setBoxModel(enemies[i].position);
	}
}
void updateEnemies(NPC_t* enemies, double deltaTime, Rectangle_t* grassAreas, Bullet_t& bullet, Car_t &player)
{
	for (int i = 0; i < ENEMIES_COUNT; i++) {
		enemies[i].position.y += ceil(deltaTime * SPEED_MULTIPLIER * enemies[i].speed);
		enemies[i].position.y %= MAP_TILES;
		double temp = deltaTime * SPEED_MULTIPLIER * enemies[i].sideSpeed;
		enemies[i].position.x += enemies[i].sideSpeed>0 ? ceil(temp) : floor(temp);
		enemies[i].boxModel = setBoxModel(enemies[i].position);
	}
	Point_t enemyBottomRight = { enemies[0].boxModel.topLeft.x + enemies[0].boxModel.w,enemies[0].boxModel.topLeft.y - enemies[0].boxModel.h};
	Point_t bulletBottomRight = { bullet.bulletBoxModel.topLeft.x + bullet.bulletBoxModel.w,bullet.bulletBoxModel.topLeft.y - bullet.bulletBoxModel.h };
	bool x = (!bullet.isShot || enemies[0].ghost) ? false : rectanglesCollide(bullet.bulletBoxModel.topLeft, bulletBottomRight, enemies[0].boxModel.topLeft, enemyBottomRight);
	if (checkForCollisions(enemies[0].boxModel, grassAreas) || x)
	{
		if (x) {
			player.scoreBuff += SCORE_AWARD_KILL;
			player.recentKill = KILL_INFO_TIME;
			if(SHOOT_DEBUG_MODE)printf("Trafiono 1. wroga\n");
		}
		enemies[0].position.x = ENEMY_1_DEFAULT_X;
		enemies[0].position.y = ENEMY_1_DEFAULT_Y;
		enemies[0].ghost = false;
		enemies[0].sideSpeed = 0;
	}
	enemyBottomRight = { enemies[1].boxModel.topLeft.x + enemies[1].boxModel.w,enemies[1].boxModel.topLeft.y - enemies[1].boxModel.h };
	x = (!bullet.isShot || enemies[1].ghost) ? false : rectanglesCollide(bullet.bulletBoxModel.topLeft, bulletBottomRight, enemies[1].boxModel.topLeft, enemyBottomRight);
	if (checkForCollisions(enemies[1].boxModel, grassAreas) || x)
	{
		if (x) {
			player.scoreBuff += SCORE_AWARD_KILL;
			player.recentKill = KILL_INFO_TIME;
			if (SHOOT_DEBUG_MODE)printf("Trafiono 2. wroga\n");
		}
		enemies[1].position.x = ENEMY_2_DEFAULT_X;
		enemies[1].position.y = ENEMY_2_DEFAULT_Y;
		enemies[1].ghost = false;
		enemies[1].sideSpeed = 0;
	}
}
void killNPC(NPC_t& npc, Car_t& player)
{
	npc.ghost = true;
	if (player.boxModel.topLeft.x <= npc.boxModel.topLeft.x)npc.sideSpeed = 3;
	else npc.sideSpeed = -3;
}
bool checkFriendCollission(NPC_t& friendCar, Car_t& player)
{
	if (friendCar.ghost || player.ghost) return false;
	Point_t A, B;
	A = { friendCar.boxModel.topLeft.x + friendCar.boxModel.w,friendCar.boxModel.topLeft.y - friendCar.boxModel.h };
	B= { player.boxModel.topLeft.x + player.boxModel.w,player.boxModel.topLeft.y - player.boxModel.h };
	if (rectanglesCollide(friendCar.boxModel.topLeft, A, player.boxModel.topLeft, B)) {
		if (abs(friendCar.boxModel.topLeft.y - player.boxModel.topLeft.y <= SIDE_COLLISION * CAR_HEIGHT)) {//horizontal
			killNPC(friendCar, player);
			player.scoreBuff -= SCORE_PENALTY_CIVILIANS;
			return false;
		}
		else { //vertical
			if (friendCar.boxModel.topLeft.y > player.boxModel.topLeft.y)
				return true;
			else {
				killNPC(friendCar, player);
				player.scoreBuff -= SCORE_PENALTY_CIVILIANS;
				return false;
			}
		}
	}
	return false;
}
bool checkEnemyCollission(NPC_t& enemy, Car_t& player)
{
	if (enemy.ghost || player.ghost) return false;
	Point_t A, B;
	B = { player.boxModel.topLeft.x + player.boxModel.w,player.boxModel.topLeft.y - player.boxModel.h };
	A = { enemy.boxModel.topLeft.x + enemy.boxModel.w,enemy.boxModel.topLeft.y - enemy.boxModel.h};
	if (rectanglesCollide(enemy.boxModel.topLeft, A, player.boxModel.topLeft, B)) {
		if (abs(enemy.boxModel.topLeft.y - player.boxModel.topLeft.y) <= SIDE_COLLISION * CAR_HEIGHT)
		{	//horizontal
			int playerKillsRandomiser = ceil(player.scoreBuff); //pseudo randomise if player or enemy die during horizontal collision
			playerKillsRandomiser %= 5;
			if (playerKillsRandomiser >= 3)
			{
				killNPC(enemy, player);
				player.scoreBuff += SCORE_AWARD_KILL;
				player.recentKill = KILL_INFO_TIME;
				return false;
			}
			else
			{
				player.ghost = true;
				player.enemyInflictedSideSpeed = ENEMY_STRENGTH;
				if (player.boxModel.topLeft.x < enemy.boxModel.topLeft.x)player.enemyInflictedSideSpeed *= -1;
				return false;
			}
		}
		else
		{	//vertical
			if (enemy.boxModel.topLeft.y > player.boxModel.topLeft.y)
			{	
				player.ghost = true;
				return true;
			}
			else
			{
				killNPC(enemy, player);
				player.scoreBuff += SCORE_AWARD_KILL;
				player.recentKill = KILL_INFO_TIME;
				return false;
			}
		}
	}
	return false;
}
bool checkForNpcColissions(NPC_t& friendCar, NPC_t* enemies, Car_t& player)
{
	bool killPlayer = checkFriendCollission(friendCar, player);
	for (int i = 0; i < ENEMIES_COUNT; i++)
		if (!killPlayer)
			killPlayer = checkEnemyCollission(enemies[i], player);
	return killPlayer;
}
void initializeVariables(Colors_t& colors, SDL_Surface* screen, Rectangle_t* grassAreas, NPC_t* enemies, PowerUp_t& powerup)
{
	colors = setColors(screen);
	setGrassAreas(grassAreas);
	setEnemies(enemies);
	regeneratePowerUp(powerup);
}
Rectangle_t setBulletBoxModel(Point_t pos)
{
	Rectangle_t temp;
	temp.topLeft.x = pos.x - BULLET_WIDTH / 2;
	temp.topLeft.y = pos.y + BULLET_HEIGHT / 2;
	temp.h = BULLET_HEIGHT;
	temp.w = BULLET_WIDTH;
	return temp;
}
Bullet_t initBullet(Car_t player, int mapY)
{
	Bullet_t temp;
	temp.isShot = false;
	temp.screenPosition = { player.position.x, SCREEN_HEIGHT/2};
	temp.speed = DEFAULT_BULLET_SPEED;
	temp.mapPosition = { player.position.x, player.position.y};
	temp.bulletBoxModel = setBulletBoxModel(temp.mapPosition);
	temp.specialAmmo = 0;
	return temp;
}
void updateBullet(Bullet_t &bullet, double deltaTime, Car_t player, int mapY)
{
	if (bullet.isShot) {
		bullet.mapPosition.y += ceil(deltaTime * SPEED_MULTIPLIER * bullet.speed);
		bullet.screenPosition.y -= ceil(deltaTime * SPEED_MULTIPLIER * bullet.speed); // -= beacuese the Y decreases while going to the top of the screen
		bullet.bulletBoxModel = setBulletBoxModel(bullet.mapPosition);
	}
	Rectangle_t tempScreenBox = setBulletBoxModel(bullet.screenPosition);	
	if (tempScreenBox.topLeft.y >= SCREEN_HEIGHT - 1 || tempScreenBox.topLeft.y - tempScreenBox.h <= 1) {
		int temp = bullet.specialAmmo;
		bullet = initBullet(player, mapY);
		bullet.specialAmmo = temp;
	}
}
void shoot(Bullet_t& bullet, Car_t player, int mapY, PowerUp_t&powerup)
{
	if (!bullet.isShot) {
		int temp = bullet.specialAmmo;
		bullet = initBullet(player, mapY);
		bullet.isShot = true;
		bullet.specialAmmo = temp;
		int x = 1;
		if (bullet.specialAmmo > 0) {
			bullet.specialAmmo--;
			x = SPECIAL_BULLET_SPEED_MULTIPLIER;
			if (bullet.specialAmmo <= 0) {
				regeneratePowerUp(powerup);
			}
		}
		bullet.speed = DEFAULT_BULLET_SPEED * x;
	}
}
Rectangle_t setPowerUpBoxModel(Point_t pos)
{
	Rectangle_t temp;
	temp.topLeft.x = pos.x - POWER_UP_SIZE / 2;
	temp.topLeft.y = pos.y + POWER_UP_SIZE / 2;
	temp.h = POWER_UP_SIZE;
	temp.w = POWER_UP_SIZE;
	return temp;
}
void regeneratePowerUp(PowerUp_t& powerup)
{
	powerup.exist = true;
	powerup.pos.y = POWER_UP_Y;
	srand(time(NULL));
	powerup.pos.x = (rand() % POWER_UP_RANDOM_OFFSET)+SCREEN_WIDTH/2 - POWER_UP_RANDOM_OFFSET/2;
	powerup.powerUpBoxModel = setPowerUpBoxModel(powerup.pos);
}
void updatePowerUp(Car_t player, PowerUp_t& powerup, Bullet_t& bullet)
{
	Point_t A = { player.boxModel.topLeft.x + player.boxModel.w, player.boxModel.topLeft.y - player.boxModel.h };
	Point_t B = { powerup.powerUpBoxModel.topLeft.x + POWER_UP_SIZE, powerup.powerUpBoxModel.topLeft.y - POWER_UP_SIZE };
	bool x = rectanglesCollide(player.boxModel.topLeft, A, powerup.powerUpBoxModel.topLeft, B);
	if (x && powerup.exist)
	{
		powerup.exist = false;
		powerup.pos = { 5, 5 }; //to not collide with anything
		powerup.powerUpBoxModel = { 5, 5, 1 ,1 };
		bullet.specialAmmo += POWER_UP_AMMO_BONUS;
	}
}
SDL_Surface* generateMap(SDL_Surface* screen)
{
	SDL_Surface* temp;
	int roadColor = SDL_MapRGB(screen->format, 70, 73, 76);
	int grassColor = SDL_MapRGB(screen->format, 12, 245, 116);

	temp = SDL_CreateRGBSurface(0, SCREEN_WIDTH, MAP_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	DrawRectangle(temp, 0, 0, SCREEN_WIDTH, MAP_HEIGHT, grassColor, grassColor);

	const int w = 40, h = 480;
	DrawRectangle(temp, 5*w, 0, 6*w,3*h, roadColor, roadColor);
	DrawRectangle(temp, 3*w, 3 * h, 10*w,h, roadColor, roadColor);
	DrawRectangle(temp, 3*w, 4 * h, 4*w, h, roadColor, roadColor);
	DrawRectangle(temp, 9*w, 4*h, 4*w, h, roadColor, roadColor);
	DrawRectangle(temp, 3*w,5*h, 10*w, h, roadColor, roadColor);
	DrawRectangle(temp, 5*w, 6*h, 6*w, h, roadColor, roadColor);

	return temp;
}