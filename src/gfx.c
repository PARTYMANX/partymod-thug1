#include <gfx.h>

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <patch.h>
#include <config.h>
#include <window.h>

uint8_t *antialiasing = 0x007d0e00;
uint8_t *hq_shadows = 0x007d0e01;
uint8_t *distance_clipping = 0x007d0e06;
uint32_t *clipping_distance = 0x007d0e10;	// int from 1-100
uint8_t *fog = 0x007d0e02;
uint8_t *addr_setaspectratio = 0x00485050;
uint8_t *addr_getscreenangfactor = 0x00485090;
float *orig_screenanglefactor = 0x006831ec;
float *screenAspectRatio = 0x00707860;

typedef struct {
	uint32_t antialiasing;
	uint32_t hq_shadows;
	uint32_t distance_clipping;
	uint32_t clipping_distance;	// int from 1-100
	uint32_t fog;
} graphicsSettings;

graphicsSettings graphics_settings;

float requestedAspect = 0.0f;

void writeConfigValues() {
	*antialiasing = graphics_settings.antialiasing;
	*hq_shadows = graphics_settings.hq_shadows;
	*distance_clipping = graphics_settings.distance_clipping;

	uint32_t distance = graphics_settings.clipping_distance;
	if (distance > 100) {
		distance = 100;
	} else if (distance < 1) {
		distance = 1;
	}
	*clipping_distance = distance * 5.0f + 95.0f;

	*fog = graphics_settings.fog;
}

uint8_t getCurrentLevel() {
	uint8_t **skate = (uint8_t *)0x0076a788;

	if (*skate) {
		uint8_t **career = *skate + 0x20;
		return *(*career + 0x5c4);
	} else {
		return 0;
	}
}

float getDesiredAspectRatio() {
	uint32_t resX, resY;
	getWindowDimensions(&resX, &resY);

	uint8_t *letterbox_active = 0x0072df3e;
	//printf("LETTERBOX: %d\n", *letterbox_active);

	if (*letterbox_active) {
		return requestedAspect;
	} else {
		return ((float)resX / (float)resY);
	}
}

float __cdecl getScreenAngleFactor() {
	float aspect = getDesiredAspectRatio();

	float result;

	if (aspect > (4.0f / 3.0f)) {
		result = ((aspect / (4.0f / 3.0f)));
	} else {
		// if aspect ratio is less than 4:3, don't reduce horizontal FOV, instead, increase vertical FOV
		result = 1.0f;
	}

	//printf("angle: 0x%08x, 0x%08x, %f %f, %d, x%f, off %d\n", addr_getscreenangfactor + 2, orig_screenanglefactor, *orig_screenanglefactor, result, *isLetterboxed, *viewportYMult, *viewportYOffset);
	//*isLetterboxed = 1;
	//*viewportYMult = 1.0f;
	//*viewportYOffset = 2;

	*screenAspectRatio = aspect;

	if (*orig_screenanglefactor != 1.0f)
		return *orig_screenanglefactor;
	else 
		return result;
}

void __cdecl setAspectRatio(float aspect) {
	requestedAspect = aspect;	// store the requested ratio for cutscenes with forced formatting
	*screenAspectRatio = getDesiredAspectRatio();
}

void setLetterbox(int isLetterboxed) {
	//printf("SETTING LETTERBOX MODE: %d\n", isLetterboxed);

	uint8_t *letterbox_active = 0x0072df3e;
	float *conv_y_multiplier = 0x0072dfc0;
	float *conv_x_multiplier = 0x0072dfbc;
	uint32_t *conv_y_offset = 0x0072dfc8;
	uint32_t *conv_x_offset = 0x0072dfc4;
	uint32_t *backbuffer_height = 0x0072dfac;
	uint32_t *backbuffer_width = 0x0072dfa8;

	//printf("Y OFFSET: %d, BACKBUFFER WIDTH: %d, BACKBUFFER HEIGHT: %d, MULTIPLIER: %f\n", *conv_y_offset, *backbuffer_width, *backbuffer_height, *conv_y_multiplier);

	*letterbox_active = isLetterboxed;

	float backbufferAspect = (float)*backbuffer_width / (float)*backbuffer_height;
	float desiredAspect = getDesiredAspectRatio();

	//printf("BACKBUFFER: %f, DESIRED: %f\n", backbufferAspect, desiredAspect);

	if (backbufferAspect > desiredAspect) {
		*conv_x_multiplier = (desiredAspect / backbufferAspect) * (*backbuffer_width / 640.0f);
		*conv_y_multiplier = *backbuffer_height / 480.0f;

		uint32_t width = round((*conv_x_multiplier) * 640.0f);

		*conv_x_offset = (*backbuffer_width - width) / 2;
		*conv_y_offset = 16 * *conv_y_multiplier;
	} else {
		*conv_x_multiplier = *backbuffer_width / 640.0f;
		*conv_y_multiplier = (backbufferAspect / desiredAspect) * (*backbuffer_height / 480.0f);

		uint32_t height = round((*conv_y_multiplier) * 480.0f);

		*conv_x_offset = 0;
		*conv_y_offset = (16 * *conv_y_multiplier) + ((*backbuffer_height - height) / 2);
	}

	//printf("MULT X: %f, MULT Y: %f\n", *conv_x_multiplier, *conv_y_multiplier);
	//printf("OFFSET X: %d, OFFSET Y: %d\n", *conv_x_offset, *conv_y_offset);

	//*letterbox_active = isLetterboxed;
}

void setDisplayRegion() {
	float *conv_y_multiplier = 0x0072dfc0;
	float *conv_x_multiplier = 0x0072dfbc;
	uint32_t *conv_y_offset = 0x0072dfc8;
	uint32_t *conv_x_offset = 0x0072dfc4;
	uint32_t *backbuffer_height = 0x0072dfac;
	uint32_t *backbuffer_width = 0x0072dfa8;
	uint32_t *display_x = 0x0072df10;
	uint32_t *display_y = 0x0072df14;
	uint32_t *display_width = 0x0072df18;
	uint32_t *display_height = 0x0072df1c;

	// calculate screen size
	uint32_t width = round((*conv_x_multiplier) * 640.0f);
	uint32_t height = round((*conv_y_multiplier) * 480.0f);

	*display_x = (*backbuffer_width - width) / 2;
	*display_width = width;
	*display_y = (*backbuffer_height - height) / 2;
	*display_height = height;
}

void renderWorldWrapper() {
	void (*renderWorld)() = 0x00477b40;
	renderWorld();

	uint8_t *letterbox_active = 0x0072df3e;

	if (getCurrentLevel() == 0) {
		if (*letterbox_active != 1) {
			setLetterbox(1);
		}
	} else {
		// NOTE: the only reason we can't leave this on is because it breaks split screen in a very funny way (both screens render on top of each other and share a depth buffer)
		if (*letterbox_active != 0) {
			setLetterbox(0);
		}
	}
	
}

#include <d3d9.h>

struct blackBarVertex {
	float x, y, z, w;
	uint32_t color;
	float u, v;
};

void drawBlackBars() {
	void (*setTexture)(int, int, int) = 0x004b92f0;
	void (*setVertexShader)(int) = 0x004b9270;
	float *conv_y_multiplier = 0x0072dfc0;
	float *conv_x_multiplier = 0x0072dfbc;
	uint32_t *conv_y_offset = 0x0072dfc8;
	uint32_t *conv_x_offset = 0x0072dfc4;
	uint32_t *backbuffer_height = 0x0072dfac;
	uint32_t *backbuffer_width = 0x0072dfa8;

	float backbufferAspect = (float)*backbuffer_width / (float)*backbuffer_height;

	if (backbufferAspect > getDesiredAspectRatio()) {
		uint32_t width = round((*conv_x_multiplier) * 640.0f);
		uint32_t barwidth = (*backbuffer_width - width) / 2;

		setTexture(0, 0, 0);	// unbind texture
		setVertexShader(0x44);

		struct blackBarVertex blackBar[4] = {
			{-0.5f, -0.5f, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
			{-0.5f, *backbuffer_height + 0.5f, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
			{barwidth + 0.5f, *backbuffer_height + 0.5f, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
			{barwidth + 0.5f, -0.5f, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
		};

		IDirect3DDevice9_DrawPrimitiveUP(*(IDirect3DDevice9 **)0x007cec6c, D3DPT_TRIANGLEFAN, 2, blackBar, sizeof(struct blackBarVertex));

		blackBar[0].x += width + barwidth;
		blackBar[1].x += width + barwidth;
		blackBar[2].x += width + barwidth;
		blackBar[3].x += width + barwidth;

		IDirect3DDevice9_DrawPrimitiveUP(*(IDirect3DDevice9 **)0x007cec6c, D3DPT_TRIANGLEFAN, 2, blackBar, sizeof(struct blackBarVertex));
	} else if (backbufferAspect < getDesiredAspectRatio()) {
		uint32_t height = round((*conv_y_multiplier) * 480.0f);
		uint32_t barheight = (*backbuffer_height - height) / 2;

		setTexture(0, 0, 0);	// unbind texture
		setVertexShader(0x44);

		struct blackBarVertex blackBar[4] = {
			{-0.5f, -0.5f, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
			{-0.5f, barheight + 0.5f, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
			{*backbuffer_width + 0.5f, barheight + 0.5f, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
			{*backbuffer_width + 0.5f, -0.5f, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
		};

		IDirect3DDevice9_DrawPrimitiveUP(*(IDirect3DDevice9 **)0x007cec6c, D3DPT_TRIANGLEFAN, 2, blackBar, sizeof(struct blackBarVertex));

		blackBar[0].y += height + barheight;
		blackBar[1].y += height + barheight;
		blackBar[2].y += height + barheight;
		blackBar[3].y += height + barheight;

		IDirect3DDevice9_DrawPrimitiveUP(*(IDirect3DDevice9 **)0x007cec6c, D3DPT_TRIANGLEFAN, 2, blackBar, sizeof(struct blackBarVertex));
	}
}

void patchLetterbox() {
	patchJmp(0x00496c30, setLetterbox);

	// set viewport for rendering
	// 3d
	patchNop(0x00497b49, 42);
	patchCall(0x00497b49, setDisplayRegion);

	// 2d
	patchNop(0x00497f99, 35);
	patchCall(0x00497f99, setDisplayRegion);

	// hacky: wrap render world in main loop with something to set letterbox if needed
	patchCall(0x00421bc8, renderWorldWrapper);

	patchCall(0x004980c9, drawBlackBars);
}

struct flashVertex {
	float x, y, z, w;
	uint32_t color;
	float u, v;
};

// vertices are passed into the render function in the wrong order when drawing screen flashes; reorder them before passing to draw
void __fastcall reorder_flash_vertices(uint32_t *d3dDevice, uint32_t *maybedeviceagain, uint32_t *alsodevice, uint32_t prim, uint32_t count, struct flashVertex *vertices, uint32_t stride){
	void(__fastcall *drawPrimitiveUP)(void *, void *, void *, uint32_t, uint32_t, struct flashVertex *, uint32_t) = maybedeviceagain[83];

	struct flashVertex tmp;

	tmp = vertices[0];
	vertices[0] = vertices[1];
	vertices[1] = vertices[2];
	vertices[2] = tmp;

	drawPrimitiveUP(d3dDevice, maybedeviceagain, alsodevice, prim, count, vertices, stride);
}

void patchScreenFlash() {
	patchNop(0x004b91e7, 6);
	patchCall(0x004b91e7, reorder_flash_vertices);
}

void setBlurWrapper(uint32_t blur) {
	uint32_t *screen_blur = 0x0072e1cc;

	*screen_blur = 0;
}

void patchBlur() {
	patchJmp(0x00496b10, setBlurWrapper);
}

void patchDisableGamma() {
	patchByte(0x004af860, 0xc3);	// return immediately when changing gamma
}

uint32_t movie_width;
uint32_t movie_height;
uint32_t movie_is_anamorphic;	// hack for credits movies

struct movieVertex {
	float x, y, z, w;
	uint32_t color;
	float u, v;
};

void movie_setTexture(int index, IDirect3DTexture9 *tex, int unk) {
	void (*setTexture)(int, int, int) = 0x004b92f0;

	D3DSURFACE_DESC desc;

	IDirect3DTexture9_GetLevelDesc(tex, 0, &desc);

	movie_width = desc.Width;
	movie_height = desc.Height;

	setTexture(index, tex, unk);
}

void __stdcall movie_drawPrim(IDirect3DDevice9 *dev, D3DPRIMITIVETYPE type, uint32_t numPrims, struct movieVertex *vertices, uint32_t stride) {
	uint32_t *backbuffer_height = 0x0072dfac;
	uint32_t *backbuffer_width = 0x0072dfa8;

	//float targetAspect = (float)movie_width / (float)movie_height;
	float targetAspect = 4.0f / 3.0f;	// while my programmer brain says "respect the original size!" the devs said "what if the credits were anamorphic 1:1"
	float backbufferAspect = (float)*backbuffer_width / (float)*backbuffer_height;

	float target_width = *backbuffer_width;
	float target_height = *backbuffer_height;
	float target_offset_x = 0.0f;
	float target_offset_y = 0.0f;

	if (backbufferAspect > targetAspect) {
		target_width = (targetAspect / backbufferAspect) * *backbuffer_width;
		target_offset_x = (*backbuffer_width - target_width) / 2;
	} else if (backbufferAspect < targetAspect) {
		target_height = (backbufferAspect / targetAspect) * *backbuffer_height;
		target_offset_y = (*backbuffer_height - target_height) / 2;
	}

	vertices[0].x = target_offset_x;
	vertices[0].y = target_offset_y;

	vertices[1].x = target_offset_x + target_width;
	vertices[1].y = target_offset_y;

	vertices[2].x = target_offset_x;
	vertices[2].y = target_offset_y + target_height;

	vertices[3].x = target_offset_x + target_width;
	vertices[3].y = target_offset_y + target_height;

	IDirect3DDevice9_DrawPrimitiveUP(dev, type, numPrims, vertices, stride);
}

void patchMovieBlackBars() {
	patchCall(0x004223b9, movie_setTexture);
	patchCall(0x004224be, movie_drawPrim);
	patchNop(0x004224be + 5, 3);
}

void loadGfxSettings() {
	graphics_settings.antialiasing = getConfigBool(CONFIG_GRAPHICS_SECTION, "AntiAliasing", 0);
	graphics_settings.hq_shadows = getConfigBool(CONFIG_GRAPHICS_SECTION, "HQShadows", 0);
	graphics_settings.distance_clipping = getConfigBool(CONFIG_GRAPHICS_SECTION, "DistanceClipping", 0);
	graphics_settings.clipping_distance = getConfigInt(CONFIG_GRAPHICS_SECTION, "ClippingDistance", 100);
	graphics_settings.fog = getConfigBool(CONFIG_GRAPHICS_SECTION, "Fog", 0);

	if (getConfigBool(CONFIG_GRAPHICS_SECTION, "DisableBlur", 1)) {
		patchBlur();
	}

	if (getConfigBool(CONFIG_GRAPHICS_SECTION, "DisableFullscreenGamma", 1)) {
		patchDisableGamma();
	}
}

float clippingDistance = 96000.0f;

void patchGfx() {
	patchCall(0x005c569f, writeConfigValues);	// don't load config, use our own

	patchJmp(addr_setaspectratio, setAspectRatio);
	patchJmp(addr_getscreenangfactor, getScreenAngleFactor);

	patchLetterbox();
	//patchScreenFlash();

	patchDWord(0x004b9d8b + 2, &clippingDistance);	// expand default clipping distance to avoid issues in large level backgrounds (I.E. hawaii)

	patchMovieBlackBars();
}
