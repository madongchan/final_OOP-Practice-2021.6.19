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
struct Timer;

struct Timer {
	unsigned int	count;
	bool			active;
	bool			eventReady;
	bool			infinite; // if true, endless counting. if count is initially set to UINT_MAX, it implies there is no timeout.

	Timer() : count(0), active(false), infinite(false), eventReady(false) {}

	void setCount(unsigned int nRemaining) {
		count = nRemaining;
		active = true;
		eventReady = false;
		if (nRemaining == UINT_MAX) infinite = true;
	}
	int getCount() { return count; }
	void reset() { count = 0;  active = false; eventReady = false; infinite = false; }
	void update() // if the timer is set and expired, it would return true. otherwise, return false
	{
		if (active == false) return;
		// active == true
		if (count == 0) return;
		// count > 0
		--count;
		if (count == 0) {
			if (infinite == true) count = UINT_MAX; //endless timer
			else eventReady = true;
		}
	}
	bool isDone() 
	{
		return eventReady;
	}
	bool isActive() { return active; }
};

struct Screen {
	int		size;
	char*	canvas;

	// constructor (생성자 함수) 메모리공간상에 적재되는 순간 호출되는
	Screen(unsigned int size)
	{
		if (size == 0) size = 80;
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
	float   hp;
	bool	alive;

	Timer	timerSmiling;
	Timer   timerHPchanged;
	Timer	timerBlinking;

	Player(const char* face, int pos) : pos(pos), hp(50), alive(true)
	{
		strcpy(this->face, face);
		this->pos = pos;
	}

	bool isAlive() { return alive; }
	void fire(Bullets* bullets, Enemy* enemy);
	void move(int direction)
	{
		(direction == directionToLeft) ? --pos : ++pos;
	}
	void draw(Screen* screen)
	{
		if (alive == false) return;
		// alive == true

		// make sure whether it requires blinking.
		if (timerBlinking.isActive() == true) {
			if (timerBlinking.isDone() == false) {
				int remainingHp = (int)hp;
				if (timerBlinking.getCount() % remainingHp != 0)
					screen->draw(pos, face);
			}
		}
		else {
			screen->draw(pos, face);
		}

		// make sure whether my HP has been changed.
		if (timerHPchanged.isActive() == true) {
			if (timerHPchanged.isDone() == false) {
				static char buf[10];
				sprintf(buf, "%d", (int)hp);
				screen->draw(pos + 1, buf);
			}
		}
	}
	void update(const char* face)
	{
		timerSmiling.update();
		timerHPchanged.update();
		timerBlinking.update();

		if (timerSmiling.isDone() == true) { // timer expired
			strcpy(this->face, face); 
			timerSmiling.reset();
		}
		if (timerHPchanged.isDone() == true)
			timerHPchanged.reset();

		if (timerBlinking.isDone() == true) 
			timerBlinking.reset();
	}
	float getHP() { return hp; }
	void getDamaged(float damage)
	{
		if (alive == false) return;
		if (hp <= 0.0f) return;

		hp -= damage;

		if (hp <= 10.0f) timerBlinking.setCount(INT_MAX); // start blinking infinitely

		timerHPchanged.setCount(5); // show changed HP for five seconds.

		if (hp <= 0.0f) {
			hp = 0.0f;
			alive = false;
		}
	}
	void onEnemyHit()
	{
		strcpy(face, "\\(^_^)/");
		timerSmiling.setCount(30);
	}
	int  getPos()
	{
		return pos;
	}
	const char* getFace()
	{
		return face;
	}
};
struct Enemy {
	char	face[20];
	int		pos;
	float   fPos;
	float   hp;
	bool	alive;

	Timer   timerCrying;
	Timer	timerRevival;
	Timer   timerHPchanged;

	Enemy() {
		start();
	}

	void start()
	{
		strcpy(face, "(`_*)");
		pos = 75;
		fPos = pos;
		alive = true;
		hp = 50.0f;
		timerRevival.reset();
		timerCrying.reset();
		timerHPchanged.reset();
	}
	void move(int direction)
	{
		fPos = direction == directionToLeft ? fPos - 0.1f : fPos + 0.1f;
		pos = fPos;
	}
	void draw(Screen* screen)
	{
		if (alive == false) return;

		screen->draw(pos, face);

		if (timerHPchanged.isActive() == true) {
			static char buf[10];
			sprintf(buf, "%d", (int)hp);
			screen->draw(pos - 3, buf);
		}
	}
	void getDamaged(float damage)
	{
		hp -= damage;

		timerHPchanged.setCount(5);

		if (hp <= 0.0f) {
			hp = 0.0f;
			alive = false;
			timerRevival.setCount(10 * 10); // timer set to revive myself after 10 seconds
		}
	}
	void update(Player* player, const char* face)
	{
		timerHPchanged.update();
		timerRevival.update();
		timerCrying.update();

		if (alive == false) {
			if (timerRevival.isDone() == false) return;

			start(); // timerRevival.reset inside
		}

		if (timerHPchanged.isDone() == true) 
			timerHPchanged.reset();
		if (timerCrying.isDone() == true) {
			strcpy(this->face, face);
			timerCrying.reset();
		}

		int player_pos = player->getPos();
		const char* player_face = player->getFace();
		if (player_pos + strlen(player_face) < pos) {
			move(directionToLeft);
		}
		else if (player_pos > pos) {
			move(directionToRight);
		}
		else {
			if (player->getHP() < 10.0f) {
				player->getDamaged(0.1f);
			}
			else {
				player->getDamaged(1.0f);
			}
		}
	}
	bool isHit(Bullet* bullet);
	void onHit()
	{
		getDamaged(5.0f);
		strcpy(face, "(T_T)");
		timerCrying.setCount(10);
	}
	int  getPos()
	{
		return pos;
	}

	bool isAlive() { return alive; }
};
struct Bullet {
	bool	isReady;
	int		pos;
	int		direction;

	void init()
	{
		isReady = true;
		pos = 0;
		direction = directionToLeft;
	}

	Bullet() :isReady(true), pos(0), direction(directionToLeft)
	{}

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
		screen->draw(pos, '-');
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
			if (direction == directionToRight) pos = 80;
			else pos = -1;
		}
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
	int		nBullets;
	Bullet*	bullets;
	bool	magazineReady;
	Timer   timerMagazine;

	Bullets(unsigned int nBullets) : nBullets{ nBullets == 0 ? 5 : (int)nBullets }, bullets{ new Bullet[nBullets] }, magazineReady{ false }
	{}
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
		timerMagazine.update();

		if (timerMagazine.isDone() == true) {
			timerMagazine.reset();
			magazineReady = true;
		}
		bool available = false;
		for (int i = 0; i < nBullets; i++)
		{
			Bullet* bullet = &bullets[i];
			if (bullet->isAvailable()) available = true;
			bullet->update(player, enemy, screen);
		}
		if (available == false && magazineReady == true) {
			Bullet* bullet = nullptr;
			for (int i = 0; i < nBullets; i++)
			{
				bullet = &bullets[i];
				if (screen->isInRange(bullet)) return;
			}
			for (int i = 0; i < nBullets; i++)
			{
				bullet = &bullets[i];
				bullet->init();
			}
			magazineReady = false;
		}

	}
	Bullet* find_unused_bullet()
	{
		Bullet* bullet = nullptr;
		for (int i = 0; i < nBullets; i++)
		{
			bullet = &bullets[i];
			if (bullet->isAvailable() == true) {
				int nUsed = 0;
				for (int j = 0; j < nBullets; j++)
				{
					Bullet* another = &bullets[j];
					if (j == i) continue;
					if (another->isAvailable() == false)
						nUsed++;
				}
				if (nUsed == 0) {
					timerMagazine.setCount(10 * 10);
				}
				return bullet;
			}
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
	Bullet* bullet = bullets->find_unused_bullet();
	if (bullet == nullptr) return;
	bullet->setFire(this, enemy);
}
bool Enemy::isHit(Bullet* bullet)
{
	if (alive == false) return false;
	int bullet_direction = bullet->getDirection();
	int bullet_pos = bullet->getPos();
	return (bullet_pos >= pos && bullet_pos <= pos + strlen(face) - 1);
}

int main()
{
	int major;
	int minor;

	Screen  screen(80);
	Player	player("(-_-)", 0);
	Enemy	enemy;
	Bullets bullets(5);

	// game loop

	bool isLooping = true;
	while (isLooping && player.isAlive() == true) {
		screen.clear();

		player.update("(-_-)");
		enemy.update(&player, "(`_#)");
		bullets.update(&player, &enemy, &screen);

		player.draw(&screen);
		enemy.draw(&screen);
		bullets.draw(&screen);

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
			case 72: // up
				enemy.move(directionToLeft);
				break;
			case 80: // down
				enemy.move(directionToRight);
				break;
			}
			break;
		}
	}
	printf("\nGame Over\n");

	return 0;
}