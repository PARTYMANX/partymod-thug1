#include <glyph.h>

#include <stdint.h>

#include <config.h>
#include <patch.h>
#include <input.h>

uint8_t forceGlyphs = 0;

char *buttonsPs2 = "ButtonsPs2";
char *buttonsNgc = "ButtonsNgc";
char *meta_button_map_ps2 = "meta_button_map_ps2";
char *meta_button_map_gamecube = "meta_button_map_gamecube";

// in the pc release, most button tags were changed from meta button mapped values to explicit buttons
// this wrapper re-translates them into rough equivalents for a controller using xbox layout
uint32_t dehexifyDigitWrapper(uint8_t *button) {
	uint32_t (*orig_dehexify)(uint8_t *) = 0x00401e40;

	if (getUsingKeyboard() && !forceGlyphs) {
		return orig_dehexify(button);
	} else {
		uint8_t val = *button;
		switch (val) {
		case 0x4D:
		case 0x6D:
			return 3;
		case 0x4E:
		case 0x6E:
			return 2;
		case 0x4F:
		case 0x6F:
			return 1;
		case 0x50:
		case 0x70:
			return 0;
		case 0x51:
		case 0x71:
			return 14;
		case 0x52:
		case 0x72:
			return 15;
		case 0x53:
		case 0x73:
			return 16;
		case 0x54:
		case 0x74:
			return 17;
		default:
			return orig_dehexify(button);
		}
	}
}

uint8_t __cdecl shouldUseGlyph(uint8_t idx) {
	if (getUsingKeyboard() && !forceGlyphs) {
		return idx >= 0x04 && idx <= 0x0D;
	} else {
		return idx <= 17;
	}
}

uint8_t getGlyphConfig() {
	forceGlyphs = getConfigBool(CONFIG_MISC_SECTION, "PreferGlyphs", 0);
	return getConfigInt(CONFIG_MISC_SECTION, "GlyphStyle", 1);
}

void patchButtonGlyphs() {
	uint8_t glyphStyle = getGlyphConfig();

	// load the appropriate glyph font and meta button map for glyph style (this only affects the basic menu control prompts)
	if (glyphStyle == 1) {
		patchDWord(0x00478c0f + 4, buttonsPs2);
		patchDWord(0x00478b88 + 1, meta_button_map_ps2);
	} else if (glyphStyle == 2) {
		patchDWord(0x00478c0f + 4, buttonsNgc);
		patchDWord(0x00478b88 + 1, meta_button_map_gamecube);
	}

	// SText::Draw
	// wrap dehexify to convert to correct glyph
	patchCall(0x004af394, dehexifyDigitWrapper); 
	
	// use custom logic to determine when to use a key string or glyph
	// PUSH EAX
	patchByte(0x004af3c4, 0x67);
	patchByte(0x004af3c4 + 1, 0x50);
	
	patchCall(0x004af3c4 + 2, shouldUseGlyph);

	// CMP AL, 0x1
	patchByte(0x004af3c4 + 7, 0x3c);
	patchByte(0x004af3c4 + 8, 0x01);

	// restore EAX with a pop
	patchByte(0x004af3c4 + 9, 0x67);
	patchByte(0x004af3c4 + 10, 0x58);

	// JZ 004af499
	patchByte(0x004af3c4 + 11, 0x0f);
	patchByte(0x004af3c4 + 12, 0x84);
	patchDWord(0x004af3c4 + 13, 0x000000c4);

	// NOP the rest
	patchNop(0x004af3c4 + 17, 9);

	// SText::QueryString
	// wrap dehexify to convert to correct glyph
	patchCall(0x004ae54b, dehexifyDigitWrapper);

	// use custom logic to determine when to use a key string or glyph
	// PUSH EAX
	patchByte(0x004ae56f, 0x67);
	patchByte(0x004ae56f + 1, 0x50);
	
	patchCall(0x004ae56f + 2, shouldUseGlyph);

	// CMP AL, 0x1
	patchByte(0x0004ae56f + 7, 0x3c);
	patchByte(0x004ae56f + 8, 0x01);

	// restore EAX with a pop
	patchByte(0x004ae56f + 9, 0x67);
	patchByte(0x004ae56f + 10, 0x58);

	// JZ 004ae5cf
	patchByte(0x004ae56f + 11, 0x74);
	patchByte(0x004ae56f + 12, 0x53);

	// NOP the rest
	patchNop(0x004ae56f + 13, 5);
}
