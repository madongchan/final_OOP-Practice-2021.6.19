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

class Screen;
class Player;
class Enemy;
class Bullet;
class PenetrableBullet;
class Timer;
class UI;

//class Timer {
//private:
//	unsigned int	count;
//	bool			active;
//	bool			eventReady;
//	bool			infinite; // if true, endless counting. if count is initially set to UINT_MAX, it implies there is no timeout.
//
//public:
//	Timer() : count(0), active(false), infinite(false), eventReady(false) {}
//
//	void setCount(unsigned int nRemaining) {
//		count = nRemaining;
//		active = true;
//		eventReady = false;
//		if (nRemaining == UINT_MAX) infinite = true;
//	}
//	int getCount() { return count; }
//	void reset() { count = 0;  active = false; eventReady = false; infinite = false; }
//	void update() // if the timer is set and expired, it would return true. otherwise, return false
//	{
//		if (active == false) return;
//		// active == true
//		if (count == 0) return;
//		// count > 0
//		--count;
//		if (count == 0) {
//			if (infinite == true) count = UINT_MAX; //endless timer
//			else eventReady = true;
//		}
//	}
//	bool isDone()
//	{
//		return eventReady;
//	}
//	bool isActive() { return active; }
//};

class Screen {
private:
	int		size;
	char* canvas;

public:

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
	virtual ~Screen()
	{
		delete[] canvas;
		canvas = nullptr;
		size = 0;
	}
};
class GameObject
{
private:
	char	face[20];
	int		pos;
	int		direction;
	Screen* screen;
	GameObject** gameObjects;

public:

	GameObject(GameObject** gameObjects, Screen* screen, const char* face, int pos, int direction)
		: pos(pos), direction(direction), screen(screen), gameObjects(gameObjects)
	{
		setFace(face);
	}
	virtual ~GameObject() {}

	void move(int direction)
	{
		direction == directionToRight ? pos++ : pos--;
	}
	void move()
	{
		(direction == directionToLeft) ? --pos : ++pos;
	}
	virtual void start() {}
	virtual void draw()
	{
		screen->draw(pos, face);
	}
	virtual void update() {}

	int getPos() { return pos; } // getter function
	void setPos(int pos) { this->pos = pos; } // setter function

	int getDirection() { return direction; }
	void setDirection(int direction) { this->direction = direction; }

	const char* getFace() { return face; }
	void setFace(const char* face) { strcpy(this->face, face); }

	Screen* getScreen() { return screen; }
	GameObject** getGameObjects() { return gameObjects; }
};
class Player : public GameObject {
private:
	int			nRemaining;
	char*  originalFace;

	Bullet* find_unused_bullet();
	Enemy* find_closest_enemy();

public:

	Player(GameObject** gameObjects, Screen* screen, const char* face, int pos);
	void fire();
	void draw() override;
	void update() override;
	void onEnemyHit()
	{
		setFace("\\(^_^)/");
		nRemaining = 30;
	}
	~Player() override {
		delete[] originalFace;
	}
};
class Enemy : public GameObject {
private:
	int		nRemaining;
	int     nMovementInterval;
	float   fPos;
	char	originalFace[20];
	UI*		ui;

public:

	Enemy(GameObject** gameObjects, Screen* screen, const char* face, int pos)
		: GameObject(gameObjects, screen, face, pos, directionToLeft),
		nRemaining(0), nMovementInterval(1), fPos(pos), ui(nullptr)
	{
		strcpy(originalFace, face);
	}

	void start() override;

	void update() override
	{
		int movement = rand() % 3 - 1;
		fPos += movement * 0.3f;
		setPos((int)fPos);

		if (nRemaining == 0) return;
		--nRemaining;
		if (nRemaining == 0) setFace(originalFace);
	}
	bool isHit(Bullet* bullet);
	void onHit();
};
class Bullet : public GameObject {
private:
	bool	isReady;

protected:
	void makeReady()
	{
		isReady = true;
	}

public:

	Bullet(GameObject** gameObjects, Screen* screen)
		: GameObject(gameObjects, screen, "-", 0, directionToLeft),
		isReady{ true }
	{}

	~Bullet() override {}
	void setFire(Player* player, Enemy* enemy)
	{
		isReady = false; // inUse

		// direction 설정
		int enemy_pos = enemy->getPos();
		int player_pos = player->getPos();
		const char* player_face = player->getFace();

		setDirection(directionToLeft);
		if (player_pos < enemy_pos) setDirection(directionToRight);

		// bullet position 설정
		setPos(player_pos);
		if (getDirection() == directionToRight)
			setPos(getPos() + (strlen(player_face) - 1));
	}

	void draw() override
	{
		if (isReady == true) return;
		GameObject::draw();
	}
	void update() override
	{
		if (isReady == true) return;

		move();
		GameObject** gameObjects = getGameObjects();

		Player* player = nullptr;
		for (int i = 0; i < 83; i++)
		{
			GameObject* obj = gameObjects[i];
			player = dynamic_cast<Player*>(obj);
			if (player != nullptr)
				break;
		}

		for (int i = 0; i < 83; i++)
		{
			GameObject* obj = gameObjects[i];
			Enemy* enemy = dynamic_cast<Enemy*>(obj);
			if (enemy == nullptr) continue;

			// enemy != nullptr
			if (enemy->isHit(this) == false) continue;

			enemy->onHit();
			if (player != nullptr)
				player->onEnemyHit();
			makeReady();
			break;
		}

		Screen* screen = getScreen();
		if (!screen->isInRange(this)) makeReady();
	}

	bool isAvailable() { return isReady; }
};

class PenetrableBullet : public Bullet {

public:

	PenetrableBullet(GameObject** gameObjects, Screen* screen)
		: Bullet(gameObjects, screen)
	{}

	~PenetrableBullet() override {}
	
	void update() override
	{
		if (isAvailable() == true) return;

		move();
		GameObject** gameObjects = getGameObjects();

		Player* player = nullptr;
		for (int i = 0; i < 83; i++)
		{
			GameObject* obj = gameObjects[i];
			player = dynamic_cast<Player*>(obj);
			if (player != nullptr)
				break;
		}

		for (int i = 0; i < 83; i++)
		{
			GameObject* obj = gameObjects[i];
			Enemy* enemy = dynamic_cast<Enemy*>(obj);
			if (enemy == nullptr) continue;

			// enemy != nullptr
			if (enemy->isHit(this) == false) continue;

			enemy->onHit();
			if (player != nullptr)
				player->onEnemyHit();
			//makeReady();
			//break;
		}

		Screen* screen = getScreen();
		if (!screen->isInRange(this)) makeReady();
	}
};

class UI : public GameObject {
	int score;
	
public:
	UI(GameObject** gameObjects, Screen* screen) 
		: GameObject(gameObjects, screen, "", 2, directionToLeft),
		score(0) 
	{}

	void update() override {
		static char buf[20];
		sprintf(buf, "%d", score);
		setFace(buf);
	}

	void setScore() {
		score++;
	}
};

// forward declaration 전방위 선언
// function prototype declaration 함수 원형 선언
bool Screen::isInRange(Bullet* bullet)
{
	int bullet_pos = bullet->getPos();
	return bullet_pos >= 0 && bullet_pos < size;
}
Player::Player(GameObject** gameObjects, Screen* screen, const char* face, int pos)
	: GameObject(gameObjects, screen, face, pos, directionToRight), nRemaining(0)
{
	originalFace = new char[20];
	strcpy(originalFace, face);
}
void Player::fire()
{
	Bullet* bullet = find_unused_bullet();
	if (bullet == nullptr) return;
	Enemy* enemy = find_closest_enemy();
	if (enemy == nullptr) return;
	bullet->setFire(this, enemy);
}
Bullet* Player::find_unused_bullet()
{
	GameObject** gameObjects = getGameObjects();
	for (int i = 0; i < 83; i++)
	{
		GameObject* obj = gameObjects[i];
		Bullet* bullet = dynamic_cast<Bullet*>(obj);
		if (bullet == nullptr) continue;
		if (bullet->isAvailable() == true)
			return bullet;
	}
	return nullptr;
}
Enemy* Player::find_closest_enemy()
{
	GameObject** gameObjects = getGameObjects();
	Enemy* closest = nullptr;
	for (int i = 0; i < 83; i++) {
		GameObject* obj = gameObjects[i];
		Enemy* enemy = dynamic_cast<Enemy*>(obj);
		if (enemy == nullptr) continue;
		if (closest == nullptr) {
			closest = enemy;
			continue;
		}
		// closest != nullptr
		int player_pos = getPos();
		int enemy_pos = enemy->getPos();
		int closest_pos = closest->getPos();
		if (abs(player_pos - enemy_pos) < abs(player_pos - closest_pos)) {
			closest = enemy;
		}
	}
	return closest;
}
void Player::draw()
{
	GameObject::draw();
}
void Player::update()
{
	if (nRemaining == 0) return;
	--nRemaining;
	if (nRemaining == 0) setFace(originalFace);
}
void Enemy::start()
{
	auto gameObjects = getGameObjects();
	for (int i = 0; i < 83; i++)
	{
		GameObject* obj = gameObjects[i];
		UI* ui = dynamic_cast<UI *>(obj);
		if (ui == nullptr) continue;

		this->ui = ui;
		break;
	}
}
void Enemy::onHit()
{
	setFace("(T_T)");
	nRemaining = 10;
	if (ui != nullptr) ui->setScore();
}
bool Enemy::isHit(Bullet* bullet)
{
	int bullet_direction = bullet->getDirection();
	int bullet_pos = bullet->getPos();
	return bullet_pos >= getPos() && bullet_pos < getPos() + strlen(getFace()) - 1;
}

int main()
{
	int major;
	int minor;

	Screen  screen(80);
	GameObject* gameObjects[80 + 1 + 2]; // game object pool

	gameObjects[0] = new Player(gameObjects, &screen, "(-_-)", 50);
	gameObjects[1] = new Enemy(gameObjects, &screen, "(`_#)", 10);
	gameObjects[2] = new Enemy(gameObjects, &screen, "(*_*)", 30);
	for (int i = 0; i < 79; i++)
		gameObjects[i + 3] = new PenetrableBullet(gameObjects, &screen);
	gameObjects[82] = new UI(gameObjects, &screen);

	//모든 객체가 생성되어 배열에 있을 때 호출

	for (int i = 0; i < 83; i++) gameObjects[i]->start();

	// game loop
	bool isLooping = true;
	while (isLooping) {
		screen.clear();

		for (int i = 0; i < 83; i++) gameObjects[i]->update();

		for (int i = 0; i < 83; i++) gameObjects[i]->draw();

		screen.render();
		Sleep(100);

		if (!_kbhit()) continue;

		major = _getch();
		switch (major) {
		case 'q':
			isLooping = false;
			break;

		case ' ':
			(static_cast<Player*>(gameObjects[0]))->fire();
			break;
		case 224: // arrow key, function key pressed
			minor = _getch();
			switch (minor) {
			case 75: // left
				gameObjects[0]->move(directionToLeft); // 				
				break;
			case 77: // right
				gameObjects[0]->move(directionToRight);
				break;
			case 72: // up
				gameObjects[1]->move(directionToLeft);
				break;
			case 80: // down
				gameObjects[1]->move(directionToRight);
				break;
			}
			break;
		}
	}
	printf("\nGame Over\n");
	for (int i = 0; i < 83; i++) delete gameObjects[i];

	return 0;
}