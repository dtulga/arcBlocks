#include "graphics.h"

#include <algorithm>

namespace arc {

RenderModule render;
GraphicsModule graphics;

//
// Screen
//

void CheckError(const int sdl_return) {
	if (sdl_return < 0) {
		log::Error(string("SDL render error: ") + SDL_GetError());
	}
}

uint32_t SDLFlagsFromScreenProperties(const ScreenProperties& props) {
	uint32_t flags = 0;

	if (props.opengl) {
		flags |= SDL_WINDOW_OPENGL;
	}
	if (props.high_dpi) {
		flags |= SDL_WINDOW_ALLOW_HIGHDPI;
	}
	if (props.hidden) {
		flags |= SDL_WINDOW_HIDDEN;
	}
	if (props.borderless) {
		flags |= SDL_WINDOW_BORDERLESS;
	}
	if (props.resizeable) {
		flags |= SDL_WINDOW_RESIZABLE;
	}
	if (props.minimized) {
		flags |= SDL_WINDOW_MINIMIZED;
	}
	if (props.maximized) {
		flags |= SDL_WINDOW_MAXIMIZED;
	}
	if (props.input_grabbed) {
		flags |= SDL_WINDOW_INPUT_GRABBED;
	}
	return flags;
}

Screen::~Screen() {
	if (renderer_ != nullptr) {
		SDL_DestroyRenderer(renderer_);
	}
	if (window_ != nullptr) {
		SDL_DestroyWindow(window_);
	}
}

SDL_Renderer* Screen::renderer() {
	if (renderer_ != nullptr) {
		return renderer_;
	}
	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); // TODO: Enable disabling vsync!
	if (properties_.type == SCREEN_FULLSCREEN && (properties_.width > 0 || properties_.height > 0)) { // TODO: Mobile screen scaling
		if (properties_.width <= 0 || properties_.height <= 0) {
			if (SDL_GetRendererOutputSize(renderer_, &(properties_.render_width), &(properties_.render_height)) != 0) {
				log::Fatal("Screen", string("SDL get renderer output size error: ") + SDL_GetError());
				return nullptr;
			}
			RecalculateWidthHeightFromRendererAspectRatio();
		}
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_RenderSetLogicalSize(renderer_, properties_.width, properties_.height);
	}
	if (renderer_ == nullptr) {
		log::Fatal("Screen", string("SDL create renderer error: ") + SDL_GetError());
	}
	return renderer_;
}

void Screen::RecalculateWidthHeightFromRendererAspectRatio() {
	if (properties_.width <= 0) {
		uint64_t height = properties_.height;
		uint64_t width = (height * properties_.render_width) / properties_.render_height;
		properties_.width = (int) width;
	} else { // height <= 0
		uint64_t width = properties_.width;
		uint64_t height = (width * properties_.render_height) / properties_.render_width;
		properties_.height = (int) height;
	}
}

void Screen::setTitle(const string& new_title) {
	if (properties_.title != new_title) {
		properties_.title = new_title;
		SDL_SetWindowTitle(window_, properties_.title.c_str());
	}
}

int Screen::width() {
	SDL_GetWindowSize(window_, &(properties_.width), &(properties_.height));
	return properties_.width;
}
int Screen::height() {
	SDL_GetWindowSize(window_, &(properties_.width), &(properties_.height));
	return properties_.height;
}

void Screen::resizeTo(const int width, const int height) {
	properties_.width = width;
	properties_.height = height;
	SDL_SetWindowSize(window_, width, height);
}

int Screen::renderWidth() {
	if (SDL_GetRendererOutputSize(renderer(), &(properties_.render_width), &(properties_.render_height)) != 0) {
		log::Fatal("Screen", string("SDL get renderer output size error: ") + SDL_GetError());
		return 0;
	}	
	return properties_.render_width;
}

int Screen::renderHeight() {
	if (SDL_GetRendererOutputSize(renderer(), &(properties_.render_width), &(properties_.render_height)) != 0) {
		log::Fatal("Screen", string("SDL get renderer output size error: ") + SDL_GetError());
		return 0;
	}	
	return properties_.render_height;
}

void Screen::show() {
	properties_.hidden = false;
	SDL_ShowWindow(window_);
}

void Screen::hide() {
	properties_.hidden = true;
	SDL_HideWindow(window_);
}

void Screen::focus() {
	SDL_RaiseWindow(window_);
}

// TODO: OpenGL context with SDL_GL_CreateContext

int Screen::xPos() {
	SDL_GetWindowPosition(window_, &(properties_.xpos), &(properties_.ypos));
	return properties_.xpos;
}

int Screen::yPos() {
	SDL_GetWindowPosition(window_, &(properties_.xpos), &(properties_.ypos));
	return properties_.ypos;	
}

void Screen::center() {
	SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void Screen::moveTo(const int xpos, const int ypos) {
	properties_.xpos = xpos;
	properties_.ypos = ypos;
	SDL_SetWindowPosition(window_, xpos, ypos);
}

//
// Sprite
//

Sprite::Sprite(SDL_Texture* texture, const int width, const int height, const bool stream, const bool render)
  : texture_(texture), width_(width), height_(height), is_stream_(stream), is_render_(render) {}

// Create a sub-sprite, which does not own the texture, but can still be drawn.
Sprite::Sprite(Sprite& original, const int sub_x, const int sub_y, const int width, const int height, bool& ok)
  : texture_(original.texture_), sub_x_(sub_x), sub_y_(sub_y), width_(width), height_(height), is_sub_(true) {
  	if (sub_x < 0 || sub_y < 0 || width < 0 || height < 0 || sub_x + width > original.width_ || sub_y + height > original.height_) {
  		ok = false;
  	} else {
  		ok = true;
  	}
}

// Create a copy of this sprite with a new color mod.
Sprite::Sprite(Sprite& original, const Color& color_mod)
	: texture_(original.texture_),
	color_mod_(color_mod),
	sub_x_(original.sub_x_), sub_y_(original.sub_y_),
	width_(original.width_), height_(original.height_),
	is_sub_(true) {}

Sprite::~Sprite() {
	if (texture_ != nullptr && !is_sub_) {
		SDL_DestroyTexture(texture_);
	}
}

bool Sprite::sub_region(SDL_Rect& rect) const {
	if (is_sub_) {
		rect.x = sub_x_;
		rect.y = sub_y_;
		rect.w = width_;
		rect.h = height_;
		return true;
	}
	return false;
}

bool Sprite::sub_region(SDL_Rect& rect, const int x, const int y, const int width, const int height, bool& ok, bool allow_crop) const {
	if (!is_sub_ && x == 0 && y == 0 && width == width_ && height == height_) {
		ok = true;
		return false;
	}
	rect.x = sub_x_ + x;
	rect.y = sub_y_ + y;
	rect.w = width;
	rect.h = height;
	if (allow_crop) {
		if (rect.x + width > width_) {
			rect.w = width_ - rect.x;
		}
		if (rect.y + height > height_) {
			rect.h = height_ - rect.y;
		}
	}
	if (rect.x >= width_ || rect.y >= height_ || rect.x + rect.w > width_ || rect.y + rect.h > height_) {
		log::Error("Sprite", "Invalid sprite drawing coordinates.");
		ok = false;
		return false;
	}
	ok = true;
	return true;
}

void Sprite::SetRenderParams() {
	if (has_color_mod()) { // TODO: Doesn't work for empty/none texture mod! (If previously set) //
		SDL_SetTextureColorMod(texture_, color_mod_.r, color_mod_.g, color_mod_.b);
	}
	if (has_alpha_mod()) {
		SDL_SetTextureAlphaMod(texture_, alpha_mod_);
	}
}

//
// RenderModule
//

RenderModule::~RenderModule() {
	screen_ = nullptr;
	renderer_ = nullptr;
	deleteallslotvector(sprites_);
}

void RenderModule::SetScreenContext(Screen& screen) { // Set the screen to render to currently.
	screen_ = &screen;
	renderer_ = screen.renderer();
	ClearSpriteContext();
}

void RenderModule::SetSpriteContext(Sprite& sprite) { // Render to a sprite instead of the screen.
	if (!sprite.is_render()) {
		throw graphics_error("cannot set a non-rendering sprite as a render target");
	}
	SDL_SetRenderTarget(renderer_, sprite.texture());
	ClearDrawOffset();
}

void RenderModule::ClearSpriteContext() { // Render to the screen again.
	SDL_SetRenderTarget(renderer_, NULL);
	ClearDrawOffset();
}

void RenderModule::Clear() {
	if (renderer_ == nullptr) { return; }
	CheckError(SDL_SetRenderDrawColor(renderer_, clear_color_.r, clear_color_.g, clear_color_.b, clear_color_.a));
	CheckError(SDL_RenderClear(renderer_));
}

void RenderModule::Clear(const Color& color) {
	if (renderer_ == nullptr) { return; }
	CheckError(SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a));
	CheckError(SDL_RenderClear(renderer_));
}

void RenderModule::DrawRect(const int x, const int y, const int width, const int height) {
	if (renderer_ == nullptr) { return; }
	SDL_Rect r = SDL_Rect_From_Coordinates(off_x_ + x, off_y_ + y, width, height);
	CheckError(SDL_SetRenderDrawColor(renderer_, draw_color_.r, draw_color_.g, draw_color_.b, draw_color_.a));
	CheckError(SDL_RenderFillRect(renderer_, &r));
}

void RenderModule::DrawRect(const int x, const int y, const int width, const int height, const Color& color) {
	if (renderer_ == nullptr) { return; }
	SDL_Rect r = SDL_Rect_From_Coordinates(off_x_ + x, off_y_ + y, width, height);
	CheckError(SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a));
	CheckError(SDL_RenderFillRect(renderer_, &r));
}

void RenderModule::DrawRectBorder(const int x, const int y, const int width, const int height, const Color& color) {
	if (renderer_ == nullptr) { return; }
	SDL_Rect r = SDL_Rect_From_Coordinates(off_x_ + x, off_y_ + y, width, height);
	CheckError(SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a));
	CheckError(SDL_RenderDrawRect(renderer_, &r));
}

void RenderModule::DrawLine(const int x_start, const int y_start, const int x_end, const int y_end) {
	if (renderer_ == nullptr) { return; }
	CheckError(SDL_SetRenderDrawColor(renderer_, draw_color_.r, draw_color_.g, draw_color_.b, draw_color_.a));
	CheckError(SDL_RenderDrawLine(renderer_, off_x_ + x_start, off_y_ + y_start, off_x_ + x_end, off_y_ + y_end));
}

void RenderModule::DrawLine(const int x_start, const int y_start, const int x_end, const int y_end, const Color& color) {
	if (renderer_ == nullptr) { return; }
	CheckError(SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a));
	CheckError(SDL_RenderDrawLine(renderer_, off_x_ + x_start, off_y_ + y_start, off_x_ + x_end, off_y_ + y_end));
}

#define A_MASK 0xFF000000
#define R_MASK 0x00FF0000
#define G_MASK 0x0000FF00
#define B_MASK 0x000000FF
#define RGB_MASK 0x00FFFFFF

// Mixes the two colors, based on the alpha of the addition.
// Keeps the original alpha the same.
void internal_32_addition_alpha_blend(uint32_t& original, const uint32_t addition) {
	const uint32_t add_alpha = (addition & A_MASK) >> 24;
	if (add_alpha == 0) {
		return; // No changes.
	}

	const uint32_t a = original & A_MASK;
	if (add_alpha == 0xFF) {
		original = a + (addition & RGB_MASK); // Replace color entirely.
	}

	const uint32_t orig_alpha = 0xFF - add_alpha;
	
	const uint32_t r = (((original & R_MASK) * orig_alpha / 0xFF) + ((addition & R_MASK) * add_alpha / 0xFF)) & R_MASK;
	const uint32_t g = (((original & G_MASK) * orig_alpha / 0xFF) + ((addition & G_MASK) * add_alpha / 0xFF)) & G_MASK;
	const uint32_t b = (((original & B_MASK) * orig_alpha / 0xFF) + ((addition & B_MASK) * add_alpha / 0xFF)) & B_MASK;
	original = a + r + g + b;
}

// Mixes the two colors, and sets the alpha (must be 0x00 - 0xFF)
void internal_32_color_blend(uint32_t& original, const uint32_t addition, const uint32_t alpha) {
	const uint32_t r = ((original & R_MASK) * 7 / 8 + (addition & R_MASK) / 8) & R_MASK;
	const uint32_t g = ((original & G_MASK) * 7 / 8 + (addition & G_MASK) / 8) & G_MASK;
	const uint32_t b = ((original & B_MASK) * 7 / 8 + (addition & B_MASK) / 8) & B_MASK;
	original = ((alpha << 24) & A_MASK) + r + g + b;
}

void internal_32_alpha_blend(uint32_t& original, const uint32_t addition) {
	uint32_t alpha = (original & A_MASK) >> 24;
	if (alpha == 0x00) {
		original = (0x40 << 24) + (addition & RGB_MASK); // Set to 1/4 alpha.
		return;
	} else if (alpha < 0xFF) {
		if (alpha == 0xC0) {
			alpha = 0xFF;
		} else {
			alpha += 0x40;
		}
	}
	internal_32_color_blend(original, addition, alpha);
}

// TODO: Selectable Blending Value //
void internal_draw_point(uint32_t* pixel_data,
	const int x, const int y, const int w, const int h,
	const int thickness, const uint32_t pixel_value, const bool blending) {
	if (thickness > 1) {
		for (int yt = y; yt < y + thickness && yt < h; yt++) {
			for (int xt = x; xt < x + thickness && xt < w; xt++) {
				const int pos = (yt * w) + xt;
				if (blending) {
					internal_32_alpha_blend(pixel_data[pos], pixel_value);
				} else {
					pixel_data[pos] = pixel_value;
				}
			}
		}
	} else {
		pixel_data[(y * w) + x] = pixel_value;
	}
}

void RenderModule::DrawLineIntoBuffer32(DataBuffer& buffer,
	const int x_start, const int y_start,
	const int x_end, const int y_end,
	const int thickness, const uint32_t pixel_value, const bool blending) {

	const int totalx = x_end - x_start;
	const int totaly = y_end - y_start;

	const int sx = totalx < 0 ? -1 : 1;
	const int sy = totaly < 0 ? -1 : 1;

	const int w = buffer.width;
	const int h = buffer.height;

	uint32_t* pixel_data = (uint32_t*)buffer.data;

	if (totalx == 0 && totaly == 0) {
		internal_draw_point(pixel_data, x_start, y_start, w, h, thickness, pixel_value, blending);
	} else if (totalx == 0) {
		// Vertical
		for (int y = y_start; y != y_end + sy && y < h && y >= 0; y += sy) {
			internal_draw_point(pixel_data, x_start, y, w, h, thickness, pixel_value, blending);
		}
	} else if (totaly == 0) {
		// Horizontal
		for (int x = x_start; x != x_end + sx && x < w && x >= 0; x += sx) {
			internal_draw_point(pixel_data, x, y_start, w, h, thickness, pixel_value, blending);
		}
	} else {
		// Diagonal
		const int atx = abs(totalx);
		const int aty = abs(totaly);
		int error = (atx > aty ? atx : -aty) / 2;
		int error2;

		int x = x_start;
		int y = y_start;

		while (true) {
			internal_draw_point(pixel_data, x, y, w, h, thickness, pixel_value, blending);
			if (x == x_end && y == y_end) break;
			error2 = error;
			if (error2 > -atx) {
				error -= aty;
				x += sx;
			}
			if (error2 < aty) {
				error += atx;
				y += sy;
			}
		}
	}
}

void RenderModule::DrawSprite(Sprite& sprite, const int x, const int y) {
	if (renderer_ == nullptr) { return; }

	SDL_Rect src;
	const bool src_region = sprite.sub_region(src);

	SDL_Rect dst = SDL_Rect_From_Coordinates(off_x_ + x, off_y_ + y, sprite.width(), sprite.height());
	const bool dst_region = (dst.x != 0 || dst.y != 0 || screen_->renderWidth() != sprite.width() || screen_->renderHeight() != sprite.height());

	sprite.SetRenderParams();
	CheckError(SDL_RenderCopy(renderer_, sprite.texture(), src_region ? &src : NULL, dst_region ? &dst : NULL));
}

void RenderModule::DrawSpriteSubRegion(Sprite& sprite, const int x, const int y, const int sub_x, const int sub_y,
	const int sub_width, const int sub_height, const bool allow_crop) {
	if (renderer_ == nullptr) { return; }

	SDL_Rect src; // Set only if src_region == true
	SDL_Rect dst;

	bool ok = false;
	const bool src_region = sprite.sub_region(src, sub_x, sub_y, sub_width, sub_height, ok, allow_crop);
	if (!ok) { return; }

	if (src_region) {
		dst = SDL_Rect_From_Coordinates(off_x_ + x, off_y_ + y, src.w, src.h); // In case of cropping.
	} else {
		dst = SDL_Rect_From_Coordinates(off_x_ + x, off_y_ + y, sub_width, sub_height);
	}

	// TODO: Optimize this check away?
	const bool dst_region = (dst.x != 0 || dst.y != 0 || screen_->renderWidth() != src.w || screen_->renderHeight() != src.h);

	sprite.SetRenderParams();
	CheckError(SDL_RenderCopy(renderer_, sprite.texture(), src_region ? &src : NULL, dst_region ? &dst : NULL));
}

void RenderModule::DrawSpriteScaling(Sprite& sprite, const int x, const int y, const double scale_factor) {
	if (renderer_ == nullptr) { return; }

	const int width = (int) (((double) sprite.width()) * scale_factor);
	const int height = (int) (((double) sprite.height()) * scale_factor);

	DrawSpriteStretch(sprite, x, y, width, height);
}

void RenderModule::DrawSpriteStretch(Sprite& sprite, const int x, const int y, const int width, const int height) {
	if (renderer_ == nullptr) { return; }

	SDL_Rect src;
	const bool src_region = sprite.sub_region(src);

	SDL_Rect dst = SDL_Rect_From_Coordinates(off_x_ + x, off_y_ + y, width, height);
	const bool dst_region = (dst.x != 0 || dst.y != 0 || screen_->renderWidth() != width || screen_->renderHeight() != height);

	sprite.SetRenderParams();
	CheckError(SDL_RenderCopy(renderer_, sprite.texture(), src_region ? &src : NULL, dst_region ? &dst : NULL));
}

void RenderModule::DrawSpriteTiled(Sprite& sprite, const int x, const int y, const int width, const int height) {
	if (renderer_ == nullptr) { return; }

	int cur_x = x;
	int cur_y = y;
	int cur_w = width;
	int cur_h = height;
	int draw_w = 0;
	int draw_h = 0;
	const int sw = sprite.width();
	const int sh = sprite.height();

	while (cur_h > 0) {
		cur_x = x;
		cur_w = width;
		while (cur_w > 0) {
			draw_w = min(sw, cur_w);
			draw_h = min(sh, cur_h);
			DrawSpriteSubRegion(sprite, cur_x, cur_y, 0, 0, draw_w, draw_h);
			cur_w -= sw;
			cur_x += sw;
		}
		cur_h -= sh;
		cur_y += sh;
	}
}

// TODO: Text Alignment - this only generates left-aligned text.
// (Use the Style struct for that.)
void RenderModule::DrawPixelText(const string& text, const size_t font_id,
								 const int x, const int y, const Color& color, const Color& back_color) {
	const size_t len = text.len();
	if (font_id >= pixel_fonts_.size()) {
		log::Error("RenderModule", "Tried to draw text from an invalid pixel font id!");
		return;
	} else if (len == 0) {
		return;
	}

	const PixelFontData& font = pixel_fonts_.at(font_id);
	const size_t fs = font.letter_data.size();
	const int line_height = font.line_height;
	const int line_spacing = font.line_spacing;

	int cx = x;
	int cy = y;
	unsigned int char_count = 0;

	for (size_t i = 0; i < len; i++) {
		const uint8_t rc = text.at(i);
		if (rc == '\n') {
			// Detect newline
			cy += line_height + line_spacing;
			cx = x; // TODO: Since left-aligned, currently. //
			char_count++;
			continue;
		} else if (rc < 32) {
			// Do not print any special characters.
			continue;
		}
		// TODO: This ignores newlines before any text 
		const uint8_t c = rc - 32;
		if (c >= fs) {
			continue;
			// Not in the font.
		}
		
		if (cx != x) {
			cx += font.spacing;
		}

		const PixelFontLetter& ld = font.letter_data[c];
		// As w and h are 0 for letters not in the font as well.
		if (ld.w != 0 && ld.h != 0) {
            if (c != 0) { // As space is always blank.
                render.DrawSpriteSubRegion(*font.sprite, cx, cy, ld.x, ld.y, ld.w, ld.h);
                // TODO: Color + Background Color + Maybe Get Text Size? //
            }

			cx += ld.w;
			char_count++;
		}
	}

	if (char_count != len) {
		// TODO: Prevent this from possible spamming? //
		log::Warn("RenderModule", "Character(s) not in pixel font when drawing text!");
	}
}

void RenderModule::GetPixelTextSize(const string& text, const size_t font_id, int& width, int& height) {
	width = 0;
	height = 0;

	if (font_id >= pixel_fonts_.size()) {
		return;
	}

	const size_t len = text.len();
	if (len == 0) {
		return;
	}

	const PixelFontData& font = pixel_fonts_.at(font_id);
	const size_t fs = font.letter_data.size();
	const int line_height = font.line_height;
	const int line_spacing = font.line_spacing;

	int w = 0;
	int tw = 0;
	int th = 0;

	// Calculate final text size.
	for (size_t i = 0; i < len; i++) {
		const uint8_t rc = text.at(i);
		if (rc == '\n') {
			th += line_height + line_spacing;
			tw = max(tw, w);
			w = 0;
			continue;
		} else if (rc < 32) {
			// Do not print any special characters.
			continue;
		}

		const uint8_t c = rc - 32;
		if (c >= fs) {
			continue;
			// Not in the font.
		}

		if (w > 0) {
			w += font.spacing;
		}

		const PixelFontLetter& ld = font.letter_data[c];
		// As w and h are 0 for letters not in the font as well.
		if (ld.w != 0 && ld.h != 0) {
			w += ld.w;
		}
	}

	// Add in final line
	th += line_height;
	tw = max(tw, w);

	width = tw;
	height = th;
}

int RenderModule::GetPixelTextWidth(const string& text, const size_t font_id) {
	int w, h;
	GetPixelTextSize(text, font_id, w, h);
	return w;
}

int RenderModule::GetPixelTextHeight(const string& text, const size_t font_id) {
	int w, h;
	GetPixelTextSize(text, font_id, w, h);
	return h;
}

// TODO: Format? //
// This is only to be used for static text, not dynamic.
Sprite& RenderModule::SpriteFromPixelText(const string& text, const size_t font_id, const Color& color, const Color& back_color) {
	if (font_id >= pixel_fonts_.size()) {
		throw graphics_error("Tried to render static text from an invalid pixel font id!");
	}

	const size_t len = text.len();
	if (len == 0) {
		throw graphics_error("Tried to render static text from an empty string!");
	}

	int tw = 0;
	int th = 0;

	GetPixelTextSize(text, font_id, tw, th);

	if (tw == 0 || th == 0) {
		throw graphics_error(
			"Tried to render static pixel text that generated an empty sprite! (Most likely due to characters not in pixel font.)");
	}

	// This also sets it to the render context.
	Sprite& r_sprite = CreateBlankSpriteForRendering(tw, th, SDL_PIXELFORMAT_ARGB8888 /*font.sprite_->format*/);
	if (!back_color.is_transparent()) {
		DrawRect(tw, th, back_color);
	}
	DrawPixelText(text, font_id, color);
	// Copy into a static sprite.
	Sprite& s_sprite = render.SpriteFromRenderSprite(r_sprite);
	render.DeleteSprite(r_sprite); // Not needed anymore.
	
	return s_sprite;
}

void RenderModule::RenderFrameDone() {
	if (renderer_ == nullptr) { return; }
	SDL_RenderPresent(renderer_);
}

size_t RenderModule::LoadPixelFont(const PixelFontData& font_data) {
	if (font_data.sprite == nullptr) {
		return -1;
	} else {
		pixel_fonts_.push_back(font_data);
		return pixel_fonts_.size() - 1;
	}
}

// Also sets the sprite to the current context!
Sprite& RenderModule::CreateBlankSpriteForRendering(const int width, const int height, const uint32_t format) {
	SDL_Texture* texture = SDL_CreateTexture(renderer_, format, SDL_TEXTUREACCESS_TARGET, width, height);
	Sprite* s = new Sprite(texture, width, height, false, true);
	SetSpriteContext(*s);
	return addtoslotvector(sprites_, s);
}

Sprite& RenderModule::SpriteFromBitmap(const string& file_path, const bool stream) {
	string bmp_fn = file_path;
	SDL_Surface* surface = SDL_LoadBMP(bmp_fn.c_str());

	if (surface == nullptr) {
		throw graphics_error("Unable to load bitmap: " + file_path + SDL_GetError());
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);

	if (texture == nullptr) {
		throw graphics_error("Unable to create texture from loaded bitmap: " + file_path + SDL_GetError());
	}

	Sprite* s = new Sprite(texture, surface->w, surface->h, stream); // TODO: Format + Caching for reload?

	SDL_FreeSurface(surface);

	return addtoslotvector(sprites_, s);
}

Sprite& RenderModule::SpriteFromImage(const string& file_path, const bool stream) { // Including PNG, etc.
	string img_fn = file_path;
	SDL_Surface* surface = IMG_Load(img_fn.c_str());

	if (surface == nullptr) {
		throw graphics_error("Unable to load image: " + file_path + IMG_GetError());
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);

	if (texture == nullptr) {
		throw graphics_error("Unable to create texture from loaded image: " + file_path + SDL_GetError());
	}

	Sprite* s = new Sprite(texture, surface->w, surface->h, stream); // TODO: Format + Caching for reload?

	SDL_FreeSurface(surface);

	return addtoslotvector(sprites_, s);
}

Sprite& RenderModule::SpriteFromDataBuffer(const DataBuffer& data, const bool stream) {
	SDL_Texture* texture = SDL_CreateTexture(
		renderer_, data.format, stream ? SDL_TEXTUREACCESS_STREAMING : SDL_TEXTUREACCESS_STATIC, data.width, data.height);
	Sprite* s = new Sprite(texture, data.width, data.height, stream);

	UpdateSpriteFromDataBuffer(*s, data);
	return addtoslotvector(sprites_, s);
}

Sprite& RenderModule::SpriteFromSubRegion(Sprite& sprite, const int sub_x, const int sub_y, const int sub_width, const int sub_height) {
	bool ok = false;
	Sprite* s = new Sprite(sprite, sub_x, sub_y, sub_width, sub_height, ok);
	if (!ok) {
		log::Error("RenderModule", "Sub region sprite has invalid coordinates");
		delete s;
		return sprite;
	}
	return addtoslotvector(sprites_, s);
}

Sprite& RenderModule::SpriteFromColorMod(Sprite& sprite, const Color& color_mod) {
	Sprite* s = new Sprite(sprite, color_mod);
	return addtoslotvector(sprites_, s);
}

// Makes a static sprite from a rendered sprite using the render pixels from SDL_RenderReadPixels...
// (so it doesn't have to be drawn again)
Sprite& RenderModule::SpriteFromRenderSprite(Sprite& sprite) {
	const uint32_t format = SDL_PIXELFORMAT_ARGB8888; // TODO: sprite.format
	const int w = sprite.width();
	const int h = sprite.height();

	DataBuffer buffer;
	const size_t bl = size_t{ 4 } * size_t(w) * size_t(h);
	buffer.len = bl;
	buffer.data = malloc(bl);
	buffer.format = format;
	buffer.width = w;
	buffer.height = h;
	buffer.set_bytes_per_row();
	
	// ReadPixels... make a new sprite from the DataBuffer (non-streaming).
	CheckError(SDL_RenderReadPixels(renderer_, nullptr, format, buffer.data, buffer.bytes_per_row));

	Sprite& s_sprite = SpriteFromDataBuffer(buffer);

	free(buffer.data); // Not needed anymore.

	return s_sprite;
}

void RenderModule::UpdateSpriteFromDataBuffer(Sprite& sprite, const DataBuffer& data) {
	if (sprite.is_stream()) {
		void* t_data;
		int pitch;
		CheckError(SDL_LockTexture(sprite.texture(), NULL, &t_data, &pitch));
		if (pitch != data.bytes_per_row) {
			throw graphics_error("pixel pitch (bytes_per_row) mismatch for streaming sprite texture update");
		}

		memcpy(t_data, data.data, pitch * data.height); // Guaranteed no overlap.

		SDL_UnlockTexture(sprite.texture()); // noerror
	} else {
		SDL_UpdateTexture(sprite.texture(), NULL, data.data, data.bytes_per_row);
	}
}

void RenderModule::DeleteSprite(Sprite& sprite) { // DANGER: Never use a deleted sprite! (or any sub-sprites!)
	deletefromslotvector(sprites_, &sprite);
}

Sprite& RenderModule::AddSpriteFromSDLTexture(SDL_Texture* texture, const int width, const int height) {
	Sprite* s = new Sprite(texture, width, height);
	return addtoslotvector(sprites_, s);
}

void RenderModule::ReloadAllTextures() {
	for (size_t i = 0; i < sprites_.size(); i++) {
		Sprite* s = sprites_[i];
		if (s != nullptr) {
			if (!s->reload()) { // TODO: Fix any failures here! //
				log::Error("RenderModule", "unable to reload sprite texture");
			}
		}
	}
}


//
// GraphicsModule
//

GraphicsModule::~GraphicsModule() {
	if (initialized_) {
		initialized_ = false;

		deleteallslotvector(screens_);

		SDL_Quit();
	}
}

bool GraphicsModule::Init() {
	if (initialized_) {
		return true;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		log::Fatal("GraphicsModule", string("SDL initalization error: ") + SDL_GetError());
	} else {
		initialized_ = true;
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) { // Linear scaling.
			log::Error("Linear texture scaling not supported!");
		}
	}

	// Initialize SDL_Image with PNG support (TODO: Turn this on/off + define formats needed!) //
	int image_formats = IMG_INIT_PNG;
	if (!(IMG_Init(image_formats) & image_formats)) {
		log::Fatal(string("SDL_image initialization error: ") + IMG_GetError());
	}

	return initialized_;
}

// TODO: -1 for auto on the width/height, etc.
// SCREEN_FULLSCREEN
Screen& GraphicsModule::CreateFullScreen(const string& title, const int width, const int height) {
	if (!initialized_) {
		throw graphics_error("SDL graphics subsystem not initalized");
	}
	
	ScreenProperties props;
	props.title = title;
	props.type = SCREEN_FULLSCREEN;
	props.width = width;
	props.height = height;

	// TODO: Auto-aspect ratio for width or height == 0 (but not both)
	SDL_Window* window = SDL_CreateWindow(props.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_FULLSCREEN_DESKTOP);

	return AddScreenFromPropertiesAndSDLWindow(props, window);
}

// SCREEN_FULLMOBILE_WINDOWDESKTOP
Screen& GraphicsModule::CreateExactScreen(const string& title, const int width, const int height, const int xpos, const int ypos) {
	if (!initialized_) {
		throw graphics_error("SDL graphics subsystem not initalized");
	}
	
	ScreenProperties props;
	props.title = title;
	props.type = SCREEN_FULLMOBILE_WINDOWDESKTOP;
	props.width = width;
	props.height = height;
	props.xpos = xpos;
	props.ypos = ypos;

	SDL_Window* window = SDL_CreateWindow(props.title.c_str(), xpos, ypos, width, height, 0);

	return AddScreenFromPropertiesAndSDLWindow(props, window);
}

// SCREEN_WINDOW
Screen& GraphicsModule::CreateWindowScreen(const string& title, const int width, const int height, const int xpos, const int ypos) {
	if (!initialized_) {
		throw graphics_error("SDL graphics subsystem not initalized");
	}
	
	ScreenProperties props;
	props.title = title;
	props.type = SCREEN_WINDOW; // TODO: Warning about this not working on mobile!
	props.width = width;
	props.height = height;
	props.xpos = xpos;
	props.ypos = ypos;

	SDL_Window* window = SDL_CreateWindow(props.title.c_str(), xpos, ypos, width, height, 0);

	return AddScreenFromPropertiesAndSDLWindow(props, window);
}

Screen& GraphicsModule::CreateCustomScreen(const ScreenProperties& properties) {
	if (!initialized_) {
		throw graphics_error("SDL graphics subsystem not initalized");
	}

	ScreenProperties props = properties;
	
#ifdef ARC_MOBILE
	if (props.type == SCREEN_FULLMOBILE_WINDOWDESKTOP || props.type == SCREEN_FULLSCREEN) {
		SDL_DisplayMode displayDim;
		if (SDL_GetCurrentDisplayMode(0, &displayDim) == 0) {
			props.width = displayDim.w;
			props.height = displayDim.h;
		} else {
			log::Error("Graphics", "Unable to get the current screen resolution!");
		}
	}
#endif
	SDL_Window* window = SDL_CreateWindow(props.title.c_str(),
										  props.xpos,
										  props.ypos,
										  props.width,
										  props.height,
										  SDLFlagsFromScreenProperties(props));

	return AddScreenFromPropertiesAndSDLWindow(props, window);
}

void GraphicsModule::DeleteScreen(Screen& screen) { // DANGER: Never use a deleted screen!
	deletefromslotvector(screens_, &screen);
}

Screen& GraphicsModule::AddScreenFromPropertiesAndSDLWindow(const ScreenProperties& props, SDL_Window* window) {
	if (window == nullptr) {
		log::Fatal("GraphicsModule", string("SDL window screen creation error: ") + SDL_GetError());
	}

	Screen* s = new Screen(props, window);
	return addtoslotvector(screens_, s);
}

} // namespace arc
