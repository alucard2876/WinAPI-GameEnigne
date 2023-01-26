#include <Windows.h>
#include <chrono>
#include <iostream>
#include <gdiplus.h>
#include <queue>
#include <map>
#include <thread>

#pragma comment(lib,"gdiplus.lib")

namespace tde 
{
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam);

	uint32_t ConvertGdiColorToHEX(Gdiplus::Color color);

	struct vector2 {
		int x;
		int y;
	};

	enum GameObjectState {
		NONE = 0,
		Idle = 1,
		Attack = 2,
		Death = 3,
		Walk = 4,
		IdleDepth = 5,
		Damage = 6
	};

	enum WeaponType {
		Cold = 0,
		FireArm = 1,
	};

	class Sprite {
	public:
		Sprite();
		Sprite(const wchar_t* fileName);
		Sprite(int width, int height);

		int SpriteHeight();
		int SpriteWidth();

		int GetSampleColor(float x, float y);
		int GetPixelColor(int x, int y);

		bool GetSampleAlpha(float x, float y);
		bool GetPixelAlpha(int x, int y);

		void SetPixel(int x, int y, uint32_t color);

		const wchar_t* GetFileName();

		~Sprite();
	private:
		void Create(int width, int height);
		void Load(const wchar_t* fileName);

	private:
		int Width;
		int Height;
		const wchar_t* FileName;
		int* Colors = nullptr;
		bool* AplhaChannels = nullptr;
	};

	class Animation {
	protected:
		std::queue<std::shared_ptr<Sprite>> Frames;
		bool isPlaying = false;

	public:
		void DeleyedStart(bool startPlay);

		bool IsPlaing();

		virtual std::shared_ptr<Sprite> Play(int objWidth, int objHeight, float elapsedTime, float animationSpeed = 1.0f, bool force = true);

		void AssignFrame(std::shared_ptr<Sprite> sprite);

		std::shared_ptr<Sprite> GetCurrentSprite();

		bool IsEmpty();

	public:
		bool isPlayedAllCicle = false;

	protected:
		uint8_t counter;
		int idleTime;
		bool isDelayed = false;
		uint8_t frameCount = 0;
		std::shared_ptr<Sprite> previousSprite;
	};


	class AudioSource
	{
	public:
		AudioSource(const wchar_t* fileName);

		void Play();

	private:
		const wchar_t* FileName;
	};

	class GameObject 
	{
	public:
		GameObject(float x, float y, float vx, float vy);

	public:
		float X;
		float Y;
		float Vx;
		float Vy;
		bool remove;

		int id;

		std::map<GameObjectState, AudioSource> Audio;

		std::map<GameObjectState, std::shared_ptr<Animation>> Animations;

	public:
		std::shared_ptr<Animation> GetAnimation(GameObjectState state);

		std::shared_ptr<Animation> GetAnimationByCurrentState();

		void SetCurrentState(GameObjectState state);

		GameObjectState GetCurrentState();

		virtual void Process(float distanceToPlayer, int fov, std::shared_ptr<GameObject> player) = 0;

	protected:
		GameObjectState CurrentState;

	};

	class Engine {
	public:
		Engine();
		int GetHeight();
		int GetWidth();
		
		bool Create(const wchar_t* title, int width, int height);
		void Show();
		
		void SetLine(int startX, int starY, int endX, int endY, int32_t color);
		void SetPoint(int x, int y, int32_t color);

		virtual void OnWindowCreate() = 0;
		virtual void OnWindowUpdate(float deltaTime = 1.0f) = 0;
		
		void DrawTextLine(int x, int y, std::string text, uint32_t color = 0x0000000, uint32_t scale = 3);
	protected:
		HWND WindowHandler;
		HINSTANCE WindowInstance;
		WNDCLASS WindowClass;
		HDC WindowHDC;
		LPVOID BitmapMemory;
		BITMAPINFO BitmapInfo;
		RECT ClientRect;

		int Width;
		int Height;

		int RectWidth;
		int RectHeight;

		bool isRunning = true;
	
	protected:
		void Reload();

	private:
		std::shared_ptr<Sprite> font = nullptr;

		void CreateFontSheet();
		void Run();
	};

	class Weapon
	{
	public:
		WeaponType Type;
		std::map<GameObjectState, std::shared_ptr<Animation>> Animations;

	public:
		Weapon()
		{

		}
	public:

		std::shared_ptr<Animation> GetAnimation(GameObjectState state)
		{
			return Animations[state];
		}

		std::shared_ptr<Animation> GetCurrentAnimation()
		{
			return CurrentAnimation;
		}

		virtual void DrawSelf(Engine* engine, int fov, std::shared_ptr<Sprite> currentSprite, bool* isHandDrawed, std::vector<vector2>* playerPosition) = 0;

		void OnMouseButtonDown(GameObjectState state)
		{
			CurrentAnimation = Animations[state];
			CurrentAnimation->DeleyedStart(true);
		}

	protected:
		std::shared_ptr<Animation> CurrentAnimation;
		//std::shared_ptr<AudioSource> Audio;
	};

}