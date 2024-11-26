#include <gfx.h>

#include <stdint.h>

#include <patch.h>
#include <config.h>
#include <window.h>

uint8_t *antialiasing = 0x007d6434;
uint8_t *hq_shadows = 0x007d6435;
uint32_t *distance_clipping = 0x007d643a;
uint32_t *clipping_distance = 0x007d6444;	// int from 1-100
uint8_t *fog = 0x007d6436;
uint8_t *addr_setaspectratio = 0x004a0780;
uint8_t *addr_getscreenangfactor = 0x004a07c0;
float *orig_screenanglefactor = 0x0067e544;
float *screenAspectRatio = 0x00701340;

typedef struct {
	uint32_t antialiasing;
	uint32_t hq_shadows;
	uint32_t distance_clipping;
	uint32_t clipping_distance;	// int from 1-100
	uint32_t fog;
} graphicsSettings;

graphicsSettings graphics_settings;

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
	uint8_t **skate = (uint8_t *)0x007ce478;

	if (*skate) {
		uint8_t **career = *skate + 0x20;
		return *(*career + 0x630);
	} else {
		return 0;
	}
}

float getDesiredAspectRatio() {
	// hack to detect main menu for aspect ratio shenanigans
	uint32_t resX, resY;
	getWindowDimensions(&resX, &resY);

	if (getCurrentLevel() == 0) {
		return 4.0f / 3.0f;
	} else {
		return ((float)resX / (float)resY);
	}
}

float __cdecl getScreenAngleFactor() {
	float aspect = getDesiredAspectRatio();

	float result = ((aspect / (4.0f / 3.0f)));

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
	*screenAspectRatio = getDesiredAspectRatio();
}

void setLetterbox(int isLetterboxed) {
	//printf("SETTING LETTERBOX MODE: %d\n", isLetterboxed);

	uint8_t *letterbox_active = 0x00786cbe;
	float *conv_y_multiplier = 0x00786d84;
	float *conv_x_multiplier = 0x00786d80;
	uint32_t *conv_y_offset = 0x00786d8c;
	uint32_t *conv_x_offset = 0x00786d88;
	uint32_t *backbuffer_height = 0x00786d70;
	uint32_t *backbuffer_width = 0x00786d6c;

	//printf("Y OFFSET: %d, BACKBUFFER WIDTH: %d, BACKBUFFER HEIGHT: %d, MULTIPLIER: %f\n", *conv_y_offset, *backbuffer_width, *backbuffer_height, *conv_y_multiplier);

	float backbufferAspect = (float)*backbuffer_width / (float)*backbuffer_height;
	float desiredAspect = getDesiredAspectRatio();

	//printf("BACKBUFFER: %f, DESIRED: %f\n", backbufferAspect, desiredAspect);

	if (backbufferAspect > desiredAspect) {
		*conv_x_multiplier = (desiredAspect / backbufferAspect) * (*backbuffer_width / 640.0f);
		*conv_y_multiplier = *backbuffer_height / 480.0f;

		uint32_t width = (*conv_x_multiplier) * 640;

		*conv_x_offset = (*backbuffer_width - width) / 2;
		*conv_y_offset = 16 * *conv_y_multiplier;
	} else {
		*conv_x_multiplier = *backbuffer_width / 640.0f;
		*conv_y_multiplier = (backbufferAspect / desiredAspect) * (*backbuffer_height / 480.0f);

		uint32_t height = (*conv_y_multiplier) * 480;

		*conv_x_offset = 0;
		*conv_y_offset = (16 * *conv_y_multiplier) + ((*backbuffer_height - height) / 2);
	}

	//printf("MULT X: %f, MULT Y: %f\n", *conv_x_multiplier, *conv_y_multiplier);
	//printf("OFFSET X: %d, OFFSET Y: %d\n", *conv_x_offset, *conv_y_offset);

	*letterbox_active = isLetterboxed;
}

void setDisplayRegion() {
	float *conv_y_multiplier = 0x00786d84;
	float *conv_x_multiplier = 0x00786d80;
	uint32_t *conv_y_offset = 0x00786d8c;
	uint32_t *conv_x_offset = 0x00786d88;
	uint32_t *backbuffer_height = 0x00786d70;
	uint32_t *backbuffer_width = 0x00786d6c;
	uint32_t *display_x = 0x00786c90;
	uint32_t *display_y = 0x00786c94;
	uint32_t *display_width = 0x00786c98;
	uint32_t *display_height = 0x00786c9c;

	// calculate screen size
	uint32_t width = (*conv_x_multiplier) * 640;
	uint32_t height = (*conv_y_multiplier) * 480;

	*display_x = (*backbuffer_width - width) / 2;
	*display_width = width;
	*display_y = (*backbuffer_height - height) / 2;
	*display_height = height;
}

void renderWorldWrapper() {
	void (*renderWorld)() = 0x0048c330;
	renderWorld();

	uint8_t *letterbox_active = 0x00786cbe;

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

void draw2DWrapper() {
	void (*draw2d)() = 0x004dfb90;
	void (*setTexture)(int, int, int) = 0x004da730;
	float *conv_x_multiplier = 0x00786d80;
	uint32_t *conv_x_offset = 0x00786d88;
	uint32_t *backbuffer_height = 0x00786d70;
	uint32_t *backbuffer_width = 0x00786d6c;

	draw2d();

	float backbufferAspect = (float)*backbuffer_width / (float)*backbuffer_height;

	if (backbufferAspect > getDesiredAspectRatio()) {
		uint32_t width = (*conv_x_multiplier) * 640;
		uint32_t barwidth = (*backbuffer_width - width) / 2;

		setTexture(0, 0, 0);	// unbind texture

		struct blackBarVertex blackBar[4] = {
			{0.0f, 0.0f, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
			{0.0f, *backbuffer_height, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
			{barwidth, *backbuffer_height, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
			{barwidth, 0.0f, 0.0f, 1.0f, 0xff000000, 0.0f, 0.0f},
		};

		IDirect3DDevice9_DrawPrimitiveUP(*(IDirect3DDevice9 **)0x007d69e0, D3DPT_TRIANGLEFAN, 4, blackBar, sizeof(struct blackBarVertex));

		blackBar[0].x += width + barwidth;
		blackBar[1].x += width + barwidth;
		blackBar[2].x += width + barwidth;
		blackBar[3].x += width + barwidth;

		IDirect3DDevice9_DrawPrimitiveUP(*(IDirect3DDevice9 **)0x007d69e0, D3DPT_TRIANGLEFAN, 4, blackBar, sizeof(struct blackBarVertex));
	}
	
}

void patchLetterbox() {
	patchJmp(0x004b2440, setLetterbox);
	//patchByte(0x004b2e63, 0xeb);
	//patchByte(0x004b33c8, 0xeb);
	patchByte(0x004df63a, 0xeb);	// fix letterboxed display

	// set viewport for rendering
	// 3d
	patchNop(0x004b2e65, 58);
	patchCall(0x004b2e65, setDisplayRegion);

	// 2d
	patchNop(0x004b33ca, 35);
	patchCall(0x004b33ca, setDisplayRegion);

	// hacky: wrap render world in main loop with something to set letterbox if needed
	patchCall(0x0044f1c0, renderWorldWrapper);

	// hacky: wrap draw 2d func to add black bars to cover overdraw
	patchCall(0x004b341d, draw2DWrapper);
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
	patchJmp(0x004b2330, setBlurWrapper);
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
}

void patchGfx() {
	patchCall(0x005f4591, writeConfigValues);	// don't load config, use our own

	patchJmp(addr_setaspectratio, setAspectRatio);
	patchJmp(addr_getscreenangfactor, getScreenAngleFactor);

	patchLetterbox();
	patchScreenFlash();

	patchFloat(0x004db357 + 6, 64000.0f);	// expand default clipping distance to avoid issues in large level backgrounds (I.E. skatopia)
}
