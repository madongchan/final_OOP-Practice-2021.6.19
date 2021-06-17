#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <conio.h> // console io
#include <cstring> // string.h
#include <cstdlib> // stdlib.h
#include <string> // c++ string class
#include <Windows.h>

// https://github.com/beomjoo90/OOP2021 , branch: 1학기

const int directionToLeft = 0;
const int directionToRight = 1;

struct Screen;
struct Player;
struct Enemy;
struct Bullet;
struct Bullets;

struct Screen {
	int		size;
	char* canvas;

	// constructor (생성자 함수) 메모리공간상에 적재되는 순간 호출되는
	Screen(unsigned int size)
	{
		if (size == 0) size = 85;
		this->size = size;
		canvas = new char[size + 1];
	}
	void clear()
	{
		memset(canvas, ' ', size);
	}
	void draw(int pos, const char* face)
	{
		strncpy(&(canvas[pos]), face, strlen(face));
	}
	void draw(int pos, char face)
	{
		if (pos < 0 || pos >= size) return;
		canvas[pos] = face;
	}
	void render()
	{
		canvas[size] = '\0';  // render screen
		printf("%s\r", canvas);
	}
	bool isInRange(Bullet* bullet);

	// destructor (소멸자 함수) 메모리공간상에서 없어지는 순간 호출되는 함수
	~Screen()
	{
		delete[] canvas;
		canvas = nullptr;
		size = 0;
	}
};




struct Player {
	char	face[20];
	int		pos;
	int		nRemaining;
	int		HP;
	int		screen_hp_time;
	int		blinkTermTime;
	int		blinkTime;
	bool	is_blinked;
	bool	is_screen_hp;
	bool	GameOver;

	Player(const char* face, int pos)
	{
		strcpy(this->face, face);
		this->pos = pos;
		this->nRemaining = 0;
		HP = 50;
		screen_hp_time = 5;
		blinkTermTime = 0;
		blinkTime = 0;
		is_blinked = false;
		is_screen_hp = false;
		GameOver = false;
	}
	void fire(Bullets* bullets, Enemy* enemy);
	void move(int direction)
	{
		(direction == directionToLeft) ? --pos : ++pos;
	}
	void blinkTimeHandler()//깜빡이는 시간조절 ---------
	{
		if (blinkTime > 0)
		{
			blinkTime--;
			is_blinked = true;
		}
		else
		{
			is_blinked = false;
		}
	}
	void blinkTermTimeHandler()//깜빡이는 간격조절
	{
		if (blinkTermTime > 0)
		{
			blinkTermTime--;
		}
		else if (HP <= 10)
		{
			blinkTime = 1;
			blinkTermTime = HP;
		}
	}
	void draw(Screen* screen)
	{
		if (is_blinked) return;
		screen->draw(pos, face);
	}
	void update(const char* face, Enemy* enemy);
	void screenHP_Time()
	{
		if (is_screen_hp)
		{
			screen_hp_time--;
		}
		if (screen_hp_time == 0)
		{
			is_screen_hp = false;
			screen_hp_time = 5;
		}
	}
	void screenHP(Screen* screen)
	{
		if (!is_screen_hp) return;
		char buf[10];
		sprintf(buf, "%d", HP);
		screen->draw(pos, buf);
	}
	void onEnemyHit()
	{
		strcpy(face, "\\(^_^)/");
		nRemaining = 10;
	}
	int  getPos()
	{
		return pos;
	}
	const char* getFace()
	{
		return face;
	}
	int  getHP()
	{
		return HP;
	}
	bool playerDead()
	{
		if (HP == 0)
		{
			GameOver = true;
		}
		return GameOver;
	}
};
struct Enemy{
	char	face[20];
	int		pos;
	int		nRemaining;
	int		moveToEnemyDelayTime;
	int		HP;
	int		screen_hp_time;
	int		respawn_time;
	bool	is_screen_hp;
	bool	is_dead;

	Enemy(const char* face, int pos)
	{
		strcpy(this->face, face);
		this->pos = pos;
		nRemaining = 0;
		moveToEnemyDelayTime = 10;
		HP = 50;
		screen_hp_time = 5;
		srand((unsigned)time(NULL));
		respawn_time = ((rand() % 6) + 5) * 10;
		is_screen_hp = false;
		is_dead = false;
	}
	void move(int direction)
	{
		direction == directionToLeft ? --pos : ++pos;
	}
	void draw(Screen* screen)
	{
		if (is_dead) return;
		screen->draw(pos, face);
	}
	void update(const char* face, Player* player)
	{
		screenHP_Time();
		enemyRespawn_Time();
		enemyRespawn();

		--moveToEnemyDelayTime;
		if (moveToEnemyDelayTime == 0)
		{
			if (!(player->getPos() + strlen(face) == this->pos))//플레이어와 맞닿으면 이동 금지
			{
				--pos;
			}
			moveToEnemyDelayTime = 10;
		}
		

		if (nRemaining == 0) return;
		--nRemaining;
		if (nRemaining == 0) strcpy(this->face, face);
		
	}
	bool enemyDead()
	{
		if (HP <= 0)
		{
			is_dead = true;
		}
		return is_dead;
	}
	void enemyRespawn_Time()//몬스터가 죽은후 다시 부활하는 시간 조절
	{
		if (is_dead)
		{
			respawn_time--;
		}
	}
	void enemyRespawn()//몬스터가 부활할때 위치와 HP값 변경함수
	{
		if (enemyDead())
		{
			pos = 80;
			HP = 50;
			if (respawn_time == 0)
			{
				is_dead = false;
				respawn_time = ((rand() % 6) + 5) * 10;
			}
		}
	}
	bool isHit(Bullet* bullet);
	void screenHP_Time()
	{
		if (is_screen_hp)
		{
			screen_hp_time--;
		}
		if (screen_hp_time == 0)
		{
			is_screen_hp = false;
			screen_hp_time = 5;
		}
	}
	void screenHP(Screen* screen)
	{
		if (!is_screen_hp || is_dead) return;
		char buf[10];
		sprintf(buf, "%d", HP);
		screen->draw(pos - 3, buf);
	}
	void onHit()
	{
		strcpy(face, "(T_T)");
		nRemaining = 10;
		HP = HP - 5;
		is_screen_hp = true;
		screen_hp_time = 5;
	}
	int  getPos()
	{
		return pos;
	}
	int  getHP()
	{
		return HP;
	}
};
struct Bullet {
	char    face[20];
	bool	isReady;
	int		pos;
	int		direction;

	Bullet()
	{
		strcpy(this->face, "-");
		isReady = true;
		pos = 0;
		direction = directionToLeft;
	}
	void setFire(Player* player, Enemy* enemy)
	{
		isReady = false; // inUse

		// direction 설정
		int enemy_pos = enemy->getPos();
		int player_pos = player->getPos();
		const char* player_face = player->getFace();

		direction = directionToLeft;
		if (player_pos < enemy_pos) direction = directionToRight;

		// bullet position 설정
		pos = player_pos;
		if (direction == directionToRight) pos += (strlen(player_face) - 1);
	}
	void move()
	{
		(direction == directionToLeft) ? --pos : ++pos;
	}
	void draw(Screen* screen)
	{
		if (isReady == true) return;
		screen->draw(pos, face);
	}
	void reuse()
	{
		isReady = true;
	}
	void update(Player* player, Enemy* enemy, Screen* screen)
	{
		if (isReady == true) return;

		move();
		if (enemy->isHit(this))
		{ // 적이 총알을 맞았을 때
			enemy->onHit();
			player->onEnemyHit();
			reuse();
		}
		if (!screen->isInRange(this)) reuse();
	}
	int  getPos() // function definition
	{
		return pos;
	}
	int  getDirection()
	{
		return direction;
	}
	bool isAvailable() { return isReady; }
};
struct Bullets {
	int		nBullets; // 80
	int		magazine;
	int		reloadingTime;
	int		replaceMagazineMassageRemainingTiem;
	bool	is_replaceMagazine;
	bool	is_reloading;
	Bullet* bullets;

	Bullets(unsigned int nBullets)
	{
		if (nBullets == 0) nBullets = 80;
		magazine = 5;
		reloadingTime = 100;
		is_reloading = false;
		replaceMagazineMassageRemainingTiem = 5;
		is_replaceMagazine = false;
		this->nBullets = nBullets;
		bullets = new Bullet[nBullets];
	}
	void draw(Screen* screen)
	{
		for (int i = 0; i < nBullets; i++)
		{
			Bullet* bullet = &bullets[i];
			bullet->draw(screen);
		}
	}
	void update(Player* player, Enemy* enemy, Screen* screen)
	{
		for (int i = 0; i < nBullets; i++)
		{
			Bullet* bullet = &bullets[i];
			bullet->update(player, enemy, screen);
		}
		leftAmmo();
		reload();
		replace_magazine();
	}
	void leftAmmo()
	{
		if (magazine == 4)
		{
			is_reloading = true;
		}
	}
	void reload()
	{
		if (is_reloading)
		{
			--reloadingTime;
		}
		if (reloadingTime <= 0)
		{
			magazine += 5;//총알을 추가
			is_replaceMagazine = true;
			is_reloading = false;
			reloadingTime = 100;
		}
	}
	int getmagazine()
	{
		return magazine;
	}
	void setmagazine(int magazine)
	{
		this->magazine = magazine;
	}
	void replace_magazine()
	{
		if (is_replaceMagazine)
		{
			replaceMagazineMassageRemainingTiem--;
		}
		if (replaceMagazineMassageRemainingTiem <= 0)
		{
			is_replaceMagazine = false;
			replaceMagazineMassageRemainingTiem = 5;
		}
	}
	void replace_magazine_massage_screen(Screen* screen)
	{
		if (!is_replaceMagazine) return;
		screen->draw(40, "탄창 교체");
	}
	Bullet* find_unused_bullet()
	{
		for (int i = 0; i < nBullets; i++)
		{
			Bullet* bullet = &bullets[i];
			if (bullet->isAvailable() == true) return bullet;
		}
		return nullptr;
	}
	~Bullets()
	{
		delete[] bullets;
		bullets = nullptr;
		nBullets = 0;
	}
};

// forward declaration 전방위 선언
// function prototype declaration 함수 원형 선언
bool Screen::isInRange(Bullet* bullet)
{
	int bullet_pos = bullet->getPos();
	return bullet_pos >= 0 && bullet_pos < size;
}
void Player::fire(Bullets* bullets, Enemy* enemy)
{
	if (bullets->getmagazine() == 0) return;
	Bullet* bullet = bullets->find_unused_bullet();
	if (bullet == nullptr) return;
	bullet->setFire(this, enemy);
	bullets->setmagazine(bullets->getmagazine() - 1);
}
void Player::update(const char* face, Enemy* enemy)
{
	blinkTimeHandler();
	blinkTermTimeHandler();

	if (pos + strlen(face) >= enemy->getPos() && pos + strlen(face) <= enemy->getPos() + strlen(enemy->face))//플레이어가 몬스터와 닿으면 받는 데미지
	{
		HP = HP - 1;
		is_screen_hp = true;
	}
	else
	{
		screenHP_Time();
	}

	if (nRemaining == 0) return;
	--nRemaining;
	if (nRemaining == 0) strcpy(this->face, face);
}
bool Enemy::isHit(Bullet* bullet)
{
	int bullet_direction = bullet->getDirection();
	int bullet_pos = bullet->getPos();
	return (
		(bullet_direction == directionToLeft && pos + strlen(face) - 1 == bullet_pos)//플레이어가 움직일때 총이랑 서로 지나가므로 범위를 늘림
		|| (bullet_direction == directionToRight && (pos == bullet_pos || pos + 1 == bullet_pos))
		);
}


int main()
{
	int major;
	int minor;

	Screen  screen(85);
	Player	player("(-_-)", 0);
	Enemy	enemy("(`_#)", 80);
	Bullets bullets(80);

	// game loop

	bool isLooping = true;
	while (isLooping) {
		screen.clear();

		player.update("(-_-)", &enemy);
		enemy.update("(`_#)", &player);
		bullets.update(&player, &enemy, &screen);

		player.draw(&screen);
		enemy.draw(&screen);
		bullets.draw(&screen);

		player.screenHP(&screen);
		enemy.screenHP(&screen);
		bullets.replace_magazine_massage_screen(&screen);

		if (player.playerDead())
		{
			isLooping = false;
		}
		//printf("%d\n", enemy.respawn_time);
		screen.render();
		Sleep(100);

		if (!_kbhit()) continue;

		major = _getch();
		switch (major) {
		case 'q':
			isLooping = false;
			break;

		case ' ':
			player.fire(&bullets, &enemy);
			break;
		case 224: // arrow key, function key pressed
			minor = _getch();
			switch (minor) {
			case 75: // left
				player.move(directionToLeft); // 				
				break;
			case 77: // right
				player.move(directionToRight);
				break;
			}
			break;
		}
	}
	printf("\n\nGame Over!!ψ(｀∇´)ψ\n");
	return 0;
}