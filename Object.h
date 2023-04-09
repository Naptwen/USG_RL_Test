//GNU AFFERO 3.0 Useop GIm April 10. 2023

#include "BaseSDL2.h"
#include <iostream>
#include <random>
#include <time.h>
#include <set>

class SpaceShip : public SDL::BaseSprite
{
public:

	SpaceShip(
		const char* path,
		SDL_Renderer* renderer,
		size_t index = 0)
	{
		mTexInd = index;
		SDL_Surface* surface = IMG_Load(path);
		if (!surface) SDL_Log("Failed to load texture %s!", path);
		this->mTexture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		if (!this->mTexture) SDL_Log("Failed to convert surface to texture %s!", path);
		SDL_QueryTexture(this->mTexture, nullptr, nullptr, &this->mTexWidth, &this->mTexHeight);
	}
};

class Rock : public SDL::BaseSprite
{
public:
	Rock(
		const char* path,
		SDL_Renderer* renderer,
		size_t index = 0)
	{
		mTexInd = index;
		SDL_Surface* surface = IMG_Load(path);
		if (!surface) SDL_Log("Failed to load texture %s!", path);
		this->mTexture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		if (!this->mTexture) SDL_Log("Failed to convert surface to texture %s!", path);
		SDL_QueryTexture(this->mTexture, nullptr, nullptr, &this->mTexWidth, &this->mTexHeight);
	}
};

class World : public SDL::BaseWindow
{
	std::random_device rd;
	std::mt19937 mt;
	SpaceShip* _ship = nullptr;
	std::vector<Rock*> _rock_list;

	void _KeyAction()
	{
		switch (mkeys) {
		case SDLK_w:
			_ship->mRect.y -= static_cast<int>(80.0f * __deltaTime);
			break;
		case SDLK_s:
			_ship->mRect.y += static_cast<int>(80.0f * __deltaTime);
			break;
		default:
			break;
		}
		if (_ship->mRect.y < 0)
			_ship->mRect.y = 0;
		else if (_ship->mRect.y > 350)
			_ship->mRect.y = 350;
	}

	void _UpdateObject()
	{
		std::set<int> random_ints;
		std::uniform_int_distribution<int> dist(0, 7);
		for (auto& rock : _rock_list)
		{
			rock->mRect.x -= static_cast<int>(40 * __deltaTime);
			if (rock->mRect.x <= 0)
			{
				int random_int = dist(mt);
				do {
					random_int = dist(mt);
				} while (random_ints.count(random_int));
				random_ints.insert(random_int);
				rock->SetRect({ 650, random_int * 50, 50, 50 });
			}
		}
	}

	void _UpdateCollision()
	{
		for (auto& rock : _rock_list)
		{
			bool collision =
				rock->mRect.x + rock->mRect.w < _ship->mRect.x ||
				rock->mRect.y + rock->mRect.h < _ship->mRect.y ||
				_ship->mRect.x + _ship->mRect.w < rock->mRect.x ||
				_ship->mRect.y + _ship->mRect.h < rock->mRect.y;
			if (!collision)
			{
				printf("collision!\n");
				init();
				break;
			}
		}
	}

	void spaceShip()
	{
		printf("Set ship\n");
		_ship = new SpaceShip("img/SpeedRunner.png", this->__renderer, 0);
		_ship->Chunk({ 1,1,2,4 });
		_ship->SetRect({ 0, 200, 100, 25 });
		this->AddSprite(_ship);
	}

	void setrock()
	{
		printf("Set rock\n");
		std::random_device rd;
		std::mt19937 mt;
		std::set<int> random_ints;
		std::uniform_int_distribution<int> dist(0, 400);

		for (size_t i = 0; i < 5; ++i)
		{
			Rock* rock = new Rock("img/Asteroid Brown.png", this->__renderer, 0);
			rock->Chunk({ 0,0,1,1 });
			int random_int = dist(mt);
			do {
				random_int = dist(mt);
			} while (random_ints.count(random_int));
			random_ints.insert(random_int);
			rock->SetRect({ 650, random_int, 50, 50 });
			this->AddSprite(rock);
			_rock_list.push_back(rock);
		}
	}

	void init()
	{
		printf("Init game\n");
		if (_ship != nullptr)
			delete _ship;
		for (auto& rock : _rock_list)
			delete rock;
		_rock_list.clear();
		__m_sprites_list.clear();
		spaceShip();
		setrock();
	}

public:

	void BaseRenderLoop() final
	{
		fprintf(stdout, "Render Loop start\n");
		this->__quit = false;
		init();
		while (!this->__quit)
		{
			_RenderInit({ 0, 0, 0, 255 });
			_UpdateKey();
			_KeyAction();
			_UpdateCollision();
			_UpdateObject();
			_UpdateTimer();
			_RenderDraw();
		}
		fprintf(stdout, "Render Loop end\n");
	}

	World(const char* window_title, std::array<int, 4> window_rect)
	{
		if (SDL_Init(SDL_INIT_EVERYTHING))
		{
			SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
			return;
		}
		this->__window = SDL_CreateWindow(
			window_title,
			window_rect.at(0),
			window_rect.at(1),
			window_rect.at(2),
			window_rect.at(3),
			SDL_WINDOW_OPENGL);
		if (!this->__window)
		{
			SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
			return;
		}
		this->__renderer = SDL_CreateRenderer(this->__window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (!this->__renderer)
		{
			SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
			return;
		}
		if (!IMG_Init(IMG_INIT_PNG))
		{
			SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
			return;
		}
		SDL_SetRenderDrawBlendMode(this->__renderer, SDL_BLENDMODE_BLEND);
		return;
	}
};