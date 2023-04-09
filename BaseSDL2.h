//GNU AFFERO 3.0 Useop GIm April 10. 2023

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <array>
#include <vector>
#include <thread>
#include <mutex> 
#include <queue>
#include <thread>

static std::mutex sdl2_mutex;

namespace SDL
{
	/*
	Sprite Object
	*/
	class BaseSprite
	{
	public:
		std::vector<SDL_Rect>	mSpriteClips = {};
		SDL_Texture* mTexture = nullptr;
		SDL_Rect						mClip = { 0, 0, 0, 0 };
		SDL_Rect						mRect = { 0, 0, 0, 0 };
		float								mRott = 0.0f;
		SDL_RendererFlip			mFlip = SDL_FLIP_NONE;
		int									mTexWidth = 0;
		int									mTexHeight = 0;
		int									mChunkW = 0;
		int									mChunkH = 0;
		size_t							mTexInd = 0;

		BaseSprite() = default;

		BaseSprite(
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

		~BaseSprite()
		{
			SDL_DestroyTexture(mTexture);
		}

		/*
		Dviding the texture by chunks
		rect[0] : # cols
		rect[1] : # rows
		rect[2] : # of cols
		rect[3] : # of rows
		*/
		void Chunk(const std::array<int, 4> rect)
		{
			this->mChunkW = this->mTexWidth / rect.at(2);
			this->mChunkH = this->mTexHeight / rect.at(3);
			for (int y = rect.at(0); y < this->mChunkH; ++y)
			{
				for (int x =  rect.at(1); x < this->mChunkW; ++x)
				{
					auto px = this->mChunkW * x + rect.at(0);
					auto py = this->mChunkH * y + rect.at(1);
					this->mSpriteClips.emplace_back(SDL_Rect({px, py, this->mChunkW, this->mChunkH}));
				}
			}
		}
		
		void AddRect(const std::array<int, 4> rect)
		{
			std::lock_guard<std::mutex> lock(sdl2_mutex);
			this->mRect.x += rect.at(0);
			this->mRect.y += rect.at(1);
			this->mRect.w += rect.at(2);
			this->mRect.h += rect.at(3);
		}

		void SetRect(const std::array<int, 4> rect)
		{
			std::lock_guard<std::mutex> lock(sdl2_mutex);
			this->mRect.x = rect.at(0);
			this->mRect.y = rect.at(1);
			this->mRect.w = rect.at(2);
			this->mRect.h = rect.at(3);
		} 
		
		void SetRotr(float rotation)
		{
			std::lock_guard<std::mutex> lock(sdl2_mutex);
			this->mRott = rotation;
		}

		void Draw(SDL_Renderer* renderer)
		{
			SDL_RenderCopyEx(
				renderer, 
				this->mTexture, 
				&mSpriteClips.at(mTexInd),
				&this->mRect,
				this->mRott,
				nullptr,
				this->mFlip);
		}

	};

	/*
	This is the basic window for SDL
	*/
	class BaseWindow
	{
	protected:
		bool __quit = false;

		SDL_Window* __window = nullptr;	//basic window
		SDL_Renderer* __renderer = nullptr; //basic renderer

		Uint32 __TickCount = 0; //TickCount
		const float __frameTime_limit = 10.0f; //frame limitation
		const float __deltaTime_maximum = 1.0f; //time per frame maximum
		const float __deltaTime_unit = 100.0f; //unit for delta time

		//********************
		// ** OBJECT CONTROL **
		//********************
		std::vector<BaseSprite*> __m_sprites_list;

		std::vector<std::thread> __thread_pool;

		void _DestroyAllSprites()
		{
			for (auto& temp : __m_sprites_list)
				if(temp != nullptr)
					delete temp;
			__m_sprites_list.clear();
		}

		void _RenderInit(std::array<Uint8, 4> color)
		{
			SDL_RenderClear(this->__renderer);
			SDL_SetRenderDrawColor(this->__renderer, color.at(0), color.at(1), color.at(2), color.at(3));
		}

		void _RenderDraw()
		{
			SDL_SetRenderDrawColor(this->__renderer, 0, 0, 0, 255);
			SDL_RenderClear(this->__renderer);
			for (auto& sprite : __m_sprites_list)
			{
				sprite->Draw(this->__renderer);
				SDL_SetRenderDrawColor(this->__renderer, 255, 255, 255, 100);
				SDL_RenderDrawRect(this->__renderer, &sprite->mRect);
			}
			SDL_RenderPresent(this->__renderer);
		}

		void _UpdateTimer()
		{
			 //wait until __frameTime_limit
			while (!SDL_TICKS_PASSED(SDL_GetTicks(), this->__TickCount + this->__frameTime_limit));
			//diff from last tick and current tick
			this->__deltaTime = (SDL_GetTicks() - this->__TickCount) / this->__deltaTime_unit;
			//limit delta
			if (this->__deltaTime > this->__deltaTime_maximum) this->__deltaTime = this->__deltaTime_maximum;
			this->__TickCount = SDL_GetTicks();
		}

		void _UpdateKey()
		{
			SDL_Event event;
			this->__quit = false;
			mkeys = NULL;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT){
					this->__quit = true;
					break;
				}
				else if (event.type == SDL_KEYDOWN) {
					std::lock_guard<std::mutex> lock(sdl2_mutex);
					mkeys = event.key.keysym.sym;
				}
			}
			const Uint8* sdlKey = SDL_GetKeyboardState(NULL);

			if (sdlKey[SDL_SCANCODE_ESCAPE]){
				this->__quit = true;
			}
		}

	public:

		float __deltaTime = 0; //delta time

		SDL_Keycode mkeys;

		BaseWindow() = default;

		BaseWindow(const char* window_title, std::array<int, 4> window_rect)
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

		~BaseWindow()
		{
			SDL_DestroyWindow(__window);
			SDL_DestroyRenderer(__renderer);
			SDL_Quit();
			IMG_Quit();
		}

		SDL_Renderer* getRenderer()
		{
			return __renderer;
		}

		void RemoveSprite(BaseSprite* src)
		{
			std::lock_guard<std::mutex> lock(sdl2_mutex);
			for (auto& sprite : __m_sprites_list)
			{
				if (sprite == src)
				{
					auto index = std::distance(sprite, src);
					__m_sprites_list.erase(__m_sprites_list.begin(), __m_sprites_list.begin() + index);
				}	
			}
		}

		void AddSprite(BaseSprite* sprite)
		{
			std::lock_guard<std::mutex> lock(sdl2_mutex);
			__m_sprites_list.push_back(sprite);
		}

		virtual void BaseRenderLoop()
		{
			fprintf(stdout, "Render Loop start\n");
			this->__quit = false;
			while (!this->__quit)
			{
				_UpdateKey();
				_UpdateTimer();
				_RenderInit({ 0, 0, 0, 255 });
				_RenderDraw();
				SDL_Delay(10);
			}
			fprintf(stdout, "Render Loop end\n");
		}
	};
}