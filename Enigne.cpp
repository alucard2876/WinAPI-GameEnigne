#include "Enigne.h"

namespace tde
{
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
	{
		if (uMsg == WM_DESTROY)
		{
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hwnd, uMsg, wparam, lparam);
	}

	uint32_t ConvertGdiColorToHEX(Gdiplus::Color color)
	{
		return ((color.GetRed() & 0xff) << 16) + ((color.GetGreen() & 0xff) << 8) + (color.GetBlue() & 0xff);
	}

	Sprite::Sprite()
	{
		Create(8, 8);
	}

	Sprite::Sprite(const wchar_t* fileName)
	{
		FileName = fileName;
		Load(fileName);
	}

	Sprite::Sprite(int width, int height)
	{
		Width = width;
		Height = height;
		Create(width, height);
	}


	int Sprite::SpriteHeight() { return Height; }

	int Sprite::SpriteWidth() { return Width; }

	Sprite::~Sprite()
	{
		delete Colors;
		delete AplhaChannels;
	}

	int Sprite::GetSampleColor(float x, float y)
	{
		int sampleX = (int)(x * (float)Width);
		int sampleY = (int)(y * ((float)Height - 1));
		if (sampleX < 0 || sampleX >= Width || sampleY < 0 || sampleY >= Height)
			return 0x000;

		return Colors[sampleY * Width + sampleX];
	}

	int Sprite::GetPixelColor(int x, int y)
	{
		if (x < 0 || x >= Width || y < 0 || y >= Height)
			return 0x000;

		return Colors[y * Width + x];
	}

	bool Sprite::GetSampleAlpha(float x, float y)
	{
		int sampleX = (int)(x * (float)Width);
		int sampleY = (int)(y * ((float)Height - 1));
		if (sampleX < 0 || sampleX >= Width || sampleY < 0 || sampleY >= Height || AplhaChannels == nullptr)
			return false;

		return AplhaChannels[sampleY * Width + sampleX];
	}

	bool Sprite::GetPixelAlpha(int x, int y)
	{
		if (x < 0 || x >= Width || y < 0 || y >= Height || AplhaChannels == nullptr)
			return false;

		return AplhaChannels[y * Width + x];
	}

	void Sprite::SetPixel(int x, int y, uint32_t color)
	{
		if (x < 0 || x >= Width || y < 0 || y >= Height)
			return;

		AplhaChannels[y * Width + x] = true;
		Colors[y * Width + x] = color;
	}

	void Sprite::Create(int width, int height)
	{
		Colors = new int[width * height];
		AplhaChannels = new bool[width * height];
		for (size_t i = 0; i < Width * Height; i++)
		{
			Colors[i] = (int)0x0000000;
			AplhaChannels[i] = false;
		}

	}
	const wchar_t* Sprite::GetFileName() {
		return FileName;
	}

	void Sprite::Load(const wchar_t* fileName) {
		delete[] Colors;
		Width = 0; Height = 0;
		int width = 0;
		FILE* file = nullptr;

		_wfopen_s(&file, fileName, L"rb");
		if (file == nullptr)
			return;

		fread(&width, sizeof(int), 1, file);

		Width = Height = width;

		Create(Width, Height);

		fread(Colors, sizeof(int), Width * Height, file);
		fread(AplhaChannels, sizeof(bool), Width * Height, file);
		fclose(file);
	}

	bool Animation::IsPlaing() {
		return isPlaying;
	}

	bool Animation::IsEmpty() {
		return Frames.empty();
	}

	std::shared_ptr<Sprite> Animation::GetCurrentSprite()
	{
		return previousSprite;
	}

	std::shared_ptr<Sprite> Animation::Play(int objWidth, int objHeight, float elapsedTime, float animationSpeed, bool force)
	{
		return previousSprite;
	}

	void Animation::DeleyedStart(bool delayed)
	{
		isDelayed = delayed;
	}

	void Animation::AssignFrame(std::shared_ptr<Sprite> sprite)
	{
		Frames.push(sprite);
		previousSprite = sprite;
	}


	Engine::Engine() : WindowInstance(GetModuleHandle(nullptr))
	{

	}

	bool Engine::Create(const wchar_t* title, int width, int height)
	{
		try
		{
			WNDCLASS window = {};
			window.lpszClassName = title;
			window.hInstance = WindowInstance;
			window.hIcon = LoadIcon(NULL, IDI_WINLOGO);
			window.hCursor = NULL;
			window.lpfnWndProc = WindowProc;

			RECT rect;
			rect.left = 250;
			rect.top = 250;
			rect.right = rect.left + width;
			rect.bottom = rect.top + height;

			Height = height;
			Width = width;

			RegisterClass(&window);

			WindowHandler = CreateWindowEx(0, title, title, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
				250, 250,
				rect.right - rect.left, rect.bottom - rect.top,
				NULL, NULL,
				WindowInstance, NULL);
			if (WindowHandler == NULL)
				return false;

			GetClientRect(WindowHandler, &ClientRect);


			RectWidth = width;//clientRect.right - clientRect.left;
			RectHeight = height;// clientRect.bottom - clientRect.top;

			BitmapMemory = VirtualAlloc(0,
				RectWidth * RectHeight * 4,
				MEM_RESERVE | MEM_COMMIT,
				PAGE_READWRITE
			);

			BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
			BitmapInfo.bmiHeader.biWidth = RectWidth;
			// Negative height makes top left as the coordinate system origin for the DrawPixel function, otherwise its bottom left
			BitmapInfo.bmiHeader.biHeight = -RectHeight;
			BitmapInfo.bmiHeader.biPlanes = 1;
			BitmapInfo.bmiHeader.biBitCount = 32;
			BitmapInfo.bmiHeader.biCompression = BI_RGB;

			WindowHDC = GetDC(WindowHandler);

			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			ULONG_PTR gdiplusToken;
			// Initialize GDI+.
			Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

			CreateFontSheet();

			OnWindowCreate();
			return true;
		}
		catch (...) {}
		return false;
	}

	void Engine::Show()
	{
		ShowWindow(WindowHandler, SW_SHOW);
		std::thread t = std::thread::thread(&Engine::Run, this);
		t.join();
	}

	void Engine::Run()
	{
		auto tp1 = std::chrono::system_clock::now();
		auto tp2 = std::chrono::system_clock::now();
		while (isRunning)
		{
			tp2 = std::chrono::system_clock::now();
			std::chrono::duration<float> elapsedTime = tp2 - tp1;
			tp1 = tp2;

			/*	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}*/

			StretchDIBits(WindowHDC,
				0, 0,
				RectWidth, RectHeight,
				0, 0,
				RectWidth, RectHeight,
				BitmapMemory, &BitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);

			OnWindowUpdate(elapsedTime.count());
			Sleep(1000);
		}
	}

	void Engine::SetLine(int startX, int startY, int endX, int endY, int32_t color)
	{
		{

			bool axisSwapped = false;
			if (abs(static_cast<int>(endX) - static_cast<int>(startX)) < abs(static_cast<int>(endY) - static_cast<int>(startY)))
			{
				axisSwapped = true;
				std::swap(startX, startY);
				std::swap(endX, endY);
			}

			int deltaX = abs(static_cast<int>(endX) - static_cast<int>(startX));
			int deltaY = abs(static_cast<int>(endY) - static_cast<int>(startY));

			if (startX > endX) {
				std::swap(startX, endX);
				std::swap(startY, endY);
			}

			int error = 0;
			int deltaErr = (deltaY + 1);

			int y = startY;
			int dirY = endY - startY;

			if (dirY > 0) dirY = 1;
			if (dirY < 0) dirY = -1;

			for (int x = startX; x <= endX; x++)
			{
				if (!axisSwapped) SetPoint(x, y, color);
				else SetPoint(y, x, color);

				error += deltaErr;
				if (error >= (deltaX + 1)) {
					y += dirY;
					error -= (deltaX + 1);
				}
			}
		}
	}

	void Engine::SetPoint(int x, int y, int32_t color)
	{
		if (x < 0 || x >= GetWidth() || y < 0 || y >= GetHeight())
			return;

		int32_t* Pixel = (int32_t*)BitmapMemory;
		Pixel += y * RectWidth + x;
		*Pixel = color;
	}

	void Engine::DrawTextLine(int x, int y, std::string text, uint32_t color, uint32_t scale)
	{
		int32_t sx = 0;
		int32_t sy = 0;

		for (auto c : text)
		{
			if (c == '\n')
			{
				sx = 0; sy += 8 * scale;
			}
			else
			{
				int32_t ox = (c - 32) % 16;
				int32_t oy = (c - 32) / 16;

				if (scale > 1)
				{
					for (uint32_t i = 0; i < 8; i++)
						for (uint32_t j = 0; j < 8; j++)
							if (font->GetPixelColor(i + ox * 8, j + oy * 8) > 0)
								for (uint32_t is = 0; is < scale; is++)
									for (uint32_t js = 0; js < scale; js++)
										SetPoint(x + sx + (i * scale) + is, y + sy + (j * scale) + js, color);
				}
				else
				{
					for (uint32_t i = 0; i < 8; i++)
						for (uint32_t j = 0; j < 8; j++)
							if (font->GetPixelColor(i + ox * 8, j + oy * 8) > 0)
								SetPoint(x + sx + i, y + sy + j, color);
				}
				sx += 8 * scale;
			}
		}
	}

	int Engine::GetHeight() {
		return RectHeight;
	}

	int Engine::GetWidth() {
		return RectWidth;
	}


	void Engine::Reload()
	{
		isRunning = false;
		OnWindowCreate();
	}

	void Engine::CreateFontSheet()
	{
		std::string data;
		data += "?Q`0001oOch0o01o@F40o0<AGD4090LAGD<090@A7ch0?00O7Q`0600>00000000";
		data += "O000000nOT0063Qo4d8>?7a14Gno94AA4gno94AaOT0>o3`oO400o7QN00000400";
		data += "Of80001oOg<7O7moBGT7O7lABET024@aBEd714AiOdl717a_=TH013Q>00000000";
		data += "720D000V?V5oB3Q_HdUoE7a9@DdDE4A9@DmoE4A;Hg]oM4Aj8S4D84@`00000000";
		data += "OaPT1000Oa`^13P1@AI[?g`1@A=[OdAoHgljA4Ao?WlBA7l1710007l100000000";
		data += "ObM6000oOfMV?3QoBDD`O7a0BDDH@5A0BDD<@5A0BGeVO5ao@CQR?5Po00000000";
		data += "Oc``000?Ogij70PO2D]??0Ph2DUM@7i`2DTg@7lh2GUj?0TO0C1870T?00000000";
		data += "70<4001o?P<7?1QoHg43O;`h@GT0@:@LB@d0>:@hN@L0@?aoN@<0O7ao0000?000";
		data += "OcH0001SOglLA7mg24TnK7ln24US>0PL24U140PnOgl0>7QgOcH0K71S0000A000";
		data += "00H00000@Dm1S007@DUSg00?OdTnH7YhOfTL<7Yh@Cl0700?@Ah0300700000000";
		data += "<008001QL00ZA41a@6HnI<1i@FHLM81M@@0LG81?O`0nC?Y7?`0ZA7Y300080000";
		data += "O`082000Oh0827mo6>Hn?Wmo?6HnMb11MP08@C11H`08@FP0@@0004@000000000";
		data += "00P00001Oab00003OcKP0006@6=PMgl<@440MglH@000000`@000001P00000000";
		data += "Ob@8@@00Ob@8@Ga13R@8Mga172@8?PAo3R@827QoOb@820@0O`0007`0000007P0";
		data += "O`000P08Od400g`<3V=P0G`673IP0`@3>1`00P@6O`P00g`<O`000GP800000000";
		data += "?P9PL020O`<`N3R0@E4HC7b0@ET<ATB0@@l6C4B0O`H3N7b0?P01L3R000000020";

		font = std::make_shared<Sprite>(128, 48);

		int px = 0, py = 0;
		for (int b = 0; b < 1024; b += 4)
		{
			uint32_t sym1 = (uint32_t)data[b + 0] - 48;
			uint32_t sym2 = (uint32_t)data[b + 1] - 48;
			uint32_t sym3 = (uint32_t)data[b + 2] - 48;
			uint32_t sym4 = (uint32_t)data[b + 3] - 48;
			uint32_t r = sym1 << 18 | sym2 << 12 | sym3 << 6 | sym4;

			for (int i = 0; i < 24; i++)
			{
				int k = r & (1 << i) ? 255 : 0;
				font->SetPixel(px, py, k + k + k);
				if (++py == 48) { px++; py = 0; }
			}
		}
	}

	GameObject::GameObject(float x, float y, float vx, float vy)
	{
		X = x;
		Y = y;
		Vx = vx;
		Vy = vy;
		remove = false;
	}

	std::shared_ptr<Animation> GameObject::GetAnimation(GameObjectState state)
	{
		if (state == NONE)
			return std::make_shared<Animation>();

		return Animations[state];
	}

	std::shared_ptr<Animation> GameObject::GetAnimationByCurrentState()
	{
		if (CurrentState == NONE)
			return std::make_shared<Animation>();

		return Animations[CurrentState];
	}

	void GameObject::SetCurrentState(GameObjectState state)
	{
		if (state == NONE)
			return;

		CurrentState = state;
	}

	GameObjectState GameObject::GetCurrentState()
	{
		return CurrentState;
	}
	
	AudioSource::AudioSource(const wchar_t* fileName)
	{
		FileName = fileName;
	}

	void AudioSource::Play()
	{
		//PlaySound(FileName, NULL, SND_ASYNC);
	}

}