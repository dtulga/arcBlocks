#pragma once

#include <exception>
#include <vector>

#ifdef __APPLE__
	#include "TargetConditionals.h"
	#ifdef TARGET_OS_IPHONE
		// iOS and Simulator
		#include "SDL.h"
		#define ARC_MOBILE 1
		#define ARC_IOS 1
	#else
		#include <SDL2/SDL.h>
		#define ARC_MAC 1
	#endif
#elif __unix__
	#include <SDL2/SDL.h>
	#define ARC_UNIX 1
#elif __ANDROID__
	#include <SDL2/SDL.h>
	#define ARC_MOBILE 1
	#define ARC_ANDROID 1
#else
	#include <SDL.h>
	#define ARC_WIN 1
#endif

//ifdef IMAGE_SUPPORT_ON
#ifdef __APPLE__
	#ifdef ARC_IOS
		#include "SDL_image.h"
	#else
		#include <SDL2_image/SDL_image.h>
	#endif
#elif __unix__
	#include <SDL2/SDL_image.h>
#else
	#include <SDL_image.h>
#endif
//endif

#include "log.h"
#include "slot_vector.h"

#define SCREEN_TYPE uint8_t

#define SCREEN_FULLMOBILE_WINDOWDESKTOP 0 /* Default */
#define SCREEN_FULLSCREEN 1
#define SCREEN_WINDOW 2 /* Doesn't work on mobile! */


namespace arc {

// Used for creating custom screens
struct ScreenProperties {
	string title;
	SCREEN_TYPE type = SCREEN_FULLMOBILE_WINDOWDESKTOP;
	int width = 0;
	int height = 0;
	int xpos = SDL_WINDOWPOS_UNDEFINED;
	int ypos = SDL_WINDOWPOS_UNDEFINED;
	// These two are set by the Screen object when requested:
	int render_width = 0;
	int render_height = 0;
	// Flags:
	// Provides an OpenGL drawing interface, the internal renderer may use OpenGL even if this is false.
	bool opengl = false;
	bool high_dpi = false; // For a HighDPI OpenGL canvas on Mac/iOS
	bool borderless = false; // Shows/hides the status bar on mobile
	// These only apply to desktop windows:
	bool hidden = false;
	bool resizeable = false;
	bool minimized = false;
	bool maximized = false;
	bool input_grabbed = false;
};

inline SDL_Rect SDL_Rect_From_Coordinates(const int x, const int y, const int w, const int h) {
	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return r;
}

// Should always be used as a Screen&
class Screen {
public:
	Screen(const ScreenProperties& properties, SDL_Window* window) : properties_(properties), window_(window) {}
	~Screen();

	const string& title() const { return properties_.title; }
	void setTitle(const string& new_title);

	// TODO: These change the renderer logical size in fullscreen mode! //
	int width();
	int height();
	void resizeTo(const int width, const int height);

	int renderWidth();
	int renderHeight();

	bool visible() const { return !properties_.hidden; }
	void show();
	void hide();
	void focus();
	void showFocus() { show(); focus(); }

	// TODO: OpenGL context
	
	SDL_Renderer* renderer();

	int xPos();
	int yPos();
	void center();
	void moveTo(const int xpos, const int ypos);

protected:
	void RecalculateWidthHeightFromRendererAspectRatio();

	ScreenProperties properties_;
	// ALL Owned
	SDL_Window* window_ = nullptr;
	SDL_Renderer* renderer_ = nullptr;
	SDL_Renderer* sprite_renderer_ = nullptr;

	DELETE_COPY_AND_ASSIGN(Screen);
};

struct Color {
	uint8_t a;
	uint8_t r;
	uint8_t g;
	uint8_t b;

	Color() : a(0), r(0), g(0), b(0) {} // Transparent
	explicit Color(uint32_t hex_color) : // argb
		a((hex_color >> 24) & 0xFF),
		r((hex_color >> 16) & 0xFF),
		g((hex_color >> 8) & 0xFF),
		b(hex_color & 0xFF) {
		if (a == 0 && (r != 0 || g != 0 || b != 0)) { a = 255; } // So colors like 0xFFFFFF work for solid colors.
	}

	Color(const uint8_t red, const uint8_t green, const uint8_t blue) : a(255), r(red), g(green), b(blue) {}
	Color(const uint8_t alpha, const uint8_t red, const uint8_t green, const uint8_t blue) : a(alpha), r(red), g(green), b(blue) {}

	Color(const Color& other) : a(other.a), r(other.r), g(other.g), b(other.b) {}
	Color& operator=(const Color& other) {
		if (this != &other) {
			a = other.a;
			r = other.r;
			g = other.g;
			b = other.b;
		}
		return *this;
	}

	// TODO: to SDL color

	uint32_t toARGB8888() const { return (a << 24) + (r << 16) + (g << 8) + b; }

	bool is_solid() const { return a == 255; }
	bool is_transparent() const { return a == 0; }
};

// Basic colors
const Color transparent = Color();
const Color white(0xFFFFFF);
const Color silver(0xC0C0C0);
const Color gray(0x808080);
const Color black(0xFF000000);

const Color red(0xFF0000);
const Color maroon(0x800000);

const Color yellow(0xFFFF00);
const Color olive(0x808000);

const Color lime(0x00FF00);
const Color green(0x008000);

const Color cyan(0x00FFFF); const Color aqua(0x00FFFF);
const Color teal(0x008080);

const Color blue(0x0000FF);
const Color navy(0x000080);

const Color fuchsia(0xFF00FF);
const Color purple(0x800080);
// TODO: support the rest of the web/x11 colors!

// Should always be used as a Sprite&
class Sprite {
public:
	Sprite(SDL_Texture* texture, const int width, const int height, const bool stream = false, const bool render = false);
	// Create a sub-sprite, which does not own the texture, but can still be drawn.
	Sprite(Sprite& original, const int sub_x, const int sub_y, const int width, const int height, bool& ok);
	// Create a copy of this sprite with a new color mod.
	Sprite(Sprite& original, const Color& color_mod);
	~Sprite();

	SDL_Texture* texture() { return texture_; }
	uint32_t format() { return format_; }
	bool is_sub() const { return is_sub_; }
	bool is_stream() const { return is_stream_; }
	bool is_render() const { return is_render_; }

	int x() const { return sub_x_; }
	int y() const { return sub_y_; }
	int width() const { return width_; }
	int height() const { return height_; }

	bool has_color_mod() const { return !color_mod_.is_transparent(); }
	Color color_mod() const { return color_mod_; }
	void set_color_mod(const Color& color) { color_mod_ = color; }

	bool has_alpha_mod() const { return alpha_mod_ != 0; }
	uint8_t alpha_mod() const { return alpha_mod_; }
	void set_alpha_mod(const uint8_t alpha) { alpha_mod_ = alpha; }

	bool sub_region(SDL_Rect& rect) const;
	bool sub_region(SDL_Rect& rect, const int x, const int y, const int width, const int height, bool& ok, bool allow_crop = false) const;

	bool reload() { return false; } // Reloads from the data cache in case the texture is lost. // TODO! //

	// This only needs to be called for non-image (PNG) textures.
	void enableAlphaBlending() {
		SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
	}

	void SetRenderParams(); // Sets any neccessary color mod, or other transforms.

protected:
	SDL_Texture* texture_ = nullptr; // Owned, if sub_x/y are 0, otherwise NOT Owned.
	Color color_mod_; // TODO: Multiple sprites sharing the same texture with a mix of color mod and none doesn't work yet! //
	// TODO: Format/DataBuffer or Surface for caching/reloading of textures + to-sprite rendering.
	uint32_t format_ = SDL_PIXELFORMAT_UNKNOWN;
	int sub_x_ = 0;
	int sub_y_ = 0;
	int width_ = 0;
	int height_ = 0;
	uint8_t alpha_mod_ = 0;
	bool is_sub_ = false;
	bool is_stream_ = false;
	bool is_render_ = false;

	DELETE_COPY_AND_ASSIGN(Sprite);
};

// Used for storing sprite data. (usually streaming)
struct DataBuffer {
	DataBuffer() {}

	void* data = nullptr;
	size_t len = 0; // In bytes.
	uint32_t format = SDL_PIXELFORMAT_ARGB8888;
	int bytes_per_row = 0;
	int width = 0; // In pixels.
	int height = 0; // In pixels.

	// Be sure to set width first!
	void set_bytes_per_row(const int pixel_size_bytes = 4, const int padding_alignment_bytes = 4) {
		bytes_per_row = pixel_size_bytes * width;
		int rem = bytes_per_row % padding_alignment_bytes;
		if (rem) {
			bytes_per_row += padding_alignment_bytes - rem;
		}
	}
};

struct PixelFontLetter{
	PixelFontLetter() {}
	PixelFontLetter(const int x_start, const int y_start, const int width, const int height)
		: x(x_start), y(y_start), w(width), h(height) {}

	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	// TODO: Offset from baseline? //
};

struct PixelFontData {
	Sprite* sprite = nullptr;
	// All letters, STARTING FROM 32 (space)
	std::vector<PixelFontLetter> letter_data;
	int spacing = 0; // Pixels between letters.
	int line_height = 0; // Height of each line
	int line_spacing = 0; // Pixels between lines.
};

// TODO: Multiple window support simulataneously with threads.
// TODO: Render to sprites (may currently fail!)
class RenderModule {
public:
	RenderModule() {};
	~RenderModule();

	void SetScreenContext(Screen& screen); // Set the screen to render to currently.
	//SetScreenContextThisThread(Screen& screen); // TODO
	void SetSpriteContext(Sprite& sprite); // Render to a sprite instead of the screen.
	void ClearSpriteContext(); // Render to the screen again.

	//bool SetDefaultFont(const string& font); // TODO: Returns if font was successfully found and loaded.
	void SetDefaultFontSize(const uint32_t font_size_px) { font_size_px_ = font_size_px; }

	void SetDrawOffset(const int x, const int y) { off_x_ = x; off_y_ = y; }
	void AddDrawOffset(const int x, const int y) { off_x_ += x; off_y_ += y; }
	void GetDrawOffset(int& x, int& y) { x = off_x_; y = off_y_; }
	void ClearDrawOffset() { off_x_ = 0; off_y_ = 0; }

	void SetClearColor(const Color& color) { clear_color_ = color; }
	void Clear();
	void Clear(const Color& color);

	void SetDrawColor(const Color& color) { draw_color_ = color; }
	void DrawRect(const int width, const int height) { DrawRect(0, 0, width, height, draw_color_); } // For offset drawing.
	void DrawRect(const int width, const int height, const Color& color) { DrawRect(0, 0, width, height, color); } // For offset drawing.
	void DrawRect(const int x, const int y, const int width, const int height);
	void DrawRect(const int x, const int y, const int width, const int height, const Color& color);
	
	void DrawRectBorder(const int width, const int height) { DrawRectBorder(0, 0, width, height, draw_color_); } // For offset drawing.
	void DrawRectBorder(const int width, const int height, const Color& color) { DrawRectBorder(0, 0, width, height, color); } // For offset drawing.
	void DrawRectBorder(const int x, const int y, const int width, const int height) { DrawRectBorder(x, y, width, height, draw_color_); }
	void DrawRectBorder(const int x, const int y, const int width, const int height, const Color& color);

	void DrawLine(const int x_start, const int y_start, const int x_end, const int y_end);
	void DrawLine(const int x_start, const int y_start, const int x_end, const int y_end, const Color& color);

	void DrawLineIntoBuffer32(DataBuffer& buffer,
		const int x_start, const int y_start, const int x_end, const int y_end,
		const int thickness, const uint32_t pixel_value, const bool blending = false);

	void DrawSprite(Sprite& sprite, const int x = 0, const int y = 0); // Default = 0 allows for offset drawing.
	void DrawSpriteSubRegion(Sprite& sprite, const int x, const int y, const int sub_x, const int sub_y,
		const int sub_width, const int sub_height, const bool allow_crop = false);
	void DrawSpriteScaling(Sprite& sprite, const int x, const int y, const double scale_factor);

	void DrawSpriteStretch(Sprite& sprite, const int width, const int height) { DrawSpriteStretch(sprite, 0, 0, width, height); }
	void DrawSpriteStretch(Sprite& sprite, const int x, const int y, const int width, const int height);

	void DrawSpriteTiled(Sprite& sprite, const int width, const int height) { DrawSpriteTiled(sprite, 0, 0, width, height); }
	void DrawSpriteTiled(Sprite& sprite, const int x, const int y, const int width, const int height);

	void DrawPixelText(const string& text, const size_t font_id, int x, int y, const Color& color, const Color& back_color = transparent);
	void DrawPixelText(const string& text, const size_t font_id, const int x, const int y) { DrawPixelText(text, font_id, x, y, draw_color_, transparent); }
	void DrawPixelText(const string& text, const size_t font_id, const Color& color, const Color& back_color = transparent) {
		DrawPixelText(text, font_id, 0, 0, color, back_color); }
	void DrawPixelText(const string& text, const size_t font_id) { DrawPixelText(text, font_id, 0, 0, draw_color_, transparent); }
	void DrawPixelText(const string& text) { DrawPixelText(text, 0, 0, 0, draw_color_, transparent); }

	void GetPixelTextSize(const string& text, const size_t font_id, int& width, int& height);
	void GetPixelTextSize(const string& text, int& width, int& height) { GetPixelTextSize(text, 0, width, height); }
	int GetPixelTextWidth(const string& text, const size_t font_id = 0);
	int GetPixelTextHeight(const string& text, const size_t font_id = 0);

	void RenderFrameDone(); // Presents the frame to the screen.

	//bool LoadFont(const string& font); // TODO: Returns if succesful.
	size_t LoadPixelFont(const PixelFontData& font_data); // Returns the font ID.

	// Also sets the sprite to the current context!
	Sprite& CreateBlankSpriteForRendering(const int width, const int height, const uint32_t format = SDL_PIXELFORMAT_ARGB8888); // 32-bit true color ?

	// Note that non-streaming sprites can be changed, but that operation might be unusually slow.
	// Also note that streaming sprites are expected to be redrawn potentially as often as every frame.
	Sprite& SpriteFromBitmap(const string& file_path, const bool stream = false);
	Sprite& SpriteFromImage(const string& file_path, const bool stream = false); // Including PNG, etc.
	Sprite& SpriteFromDataBuffer(const DataBuffer& data, const bool stream = false);

	// WARNING: Leaves out any letters not in the pixel font!
	// Also note that transparent == default sprite color.
	Sprite& SpriteFromPixelText(const string& text, const size_t font_id,
		const Color& color = transparent, const Color& back_color = transparent);

	Sprite& SpriteFromSubRegion(Sprite& sprite, const int sub_x, const int sub_y, const int sub_width, const int sub_height);

	Sprite& SpriteFromColorMod(Sprite& sprite, const Color& color_mod);

	// Makes a static sprite from a rendered sprite using the render pixels from SDL_RenderReadPixels... (so it doesn't have to be drawn again)
	Sprite& SpriteFromRenderSprite(Sprite& sprite);

	void UpdateSpriteFromDataBuffer(Sprite& sprite, const DataBuffer& data);

	void DeleteSprite(Sprite& sprite); // DANGER: Never use a deleted sprite! (Or any sub-sprites!)

	// Only necessary to enable this when using background blending, not just texture blending.
	void EnableAlphaBlending() { SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND); }
	void DisableAlphaBlending() { SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE); }

	// Generally only called by the event processing loop, when the draw device/video memory is lost. // TODO //
	void ReloadAllTextures();

private:
	Sprite& AddSpriteFromSDLTexture(SDL_Texture* texture, const int width, const int height);

	// Owned, for automatic memory management.
	std::vector<Sprite*> sprites_;

	// Loaded pixel fonts:
	std::vector<PixelFontData> pixel_fonts_;

	Screen* screen_ = nullptr;
	SDL_Renderer* renderer_ = nullptr; // Whatever renderer is being used currently.
	// Defaults:
	Color clear_color_;
	Color draw_color_;
	int off_x_ = 0;
	int off_y_ = 0;
	string font_ = "";
	uint32_t font_size_px_ = 0;
};

extern RenderModule render;

// Global Graphics Rendering/UI Functions
// See Blocks for the actual UI elements.
class GraphicsModule {
public:
	GraphicsModule() {}
	~GraphicsModule();

	bool Init();

	Screen& CreateFullScreen(const string& title, const int width = 0, const int height = 0); // SCREEN_FULLSCREEN
	Screen& CreateExactScreen(const string& title, const int width, const int height,
							  const int xpos = SDL_WINDOWPOS_UNDEFINED, const int ypos = SDL_WINDOWPOS_UNDEFINED); // SCREEN_FULLMOBILE_WINDOWDESKTOP
	Screen& CreateWindowScreen(const string& title, const int width, const int height,
							   const int xpos = SDL_WINDOWPOS_UNDEFINED, const int ypos = SDL_WINDOWPOS_UNDEFINED); // SCREEN_WINDOW
	Screen& CreateCustomScreen(const ScreenProperties& properties);

	void DeleteScreen(Screen& screen); // DANGER: Never use a deleted screen!

private:
	Screen& AddScreenFromPropertiesAndSDLWindow(const ScreenProperties& props, SDL_Window* window);

	std::vector<Screen*> screens_; // For automatic memory management.
	bool initialized_ = false;
};

extern GraphicsModule graphics;

class graphics_error : public std::runtime_error {
public:
	explicit graphics_error(const std::string& what_arg) : std::runtime_error(what_arg) {}
	explicit graphics_error(const char* what_arg) : std::runtime_error(what_arg) {}
	explicit graphics_error(const arc::string& what_arg) : std::runtime_error( (string(what_arg)).c_str() ) {}
};

} // namespace arc
