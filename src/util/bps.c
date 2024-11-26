#include <stdint.h>
#include <log.h>

// implementation of Near's beat-protocol (bps) (R.I.P.)

uint8_t readByte(uint8_t *buf, size_t *offset) {
	uint8_t result = buf[*offset];
	(*offset)++;
	return result;
}

uint32_t getPatchChecksum(uint8_t *buf, size_t len) {
	size_t offset = len - 4;
	uint32_t patchcrc = 0;
	patchcrc |= readByte(buf, &offset);
	patchcrc |= readByte(buf, &offset) << 8;
	patchcrc |= readByte(buf, &offset) << 16;
	patchcrc |= readByte(buf, &offset) << 24;
	
	return patchcrc;
}

uint64_t decodeNumber(uint8_t *buf, size_t *offset) {
	uint64_t result = 0;
	uint64_t bitOffset = 0;

	while(1) {
		uint8_t byte = readByte(buf, offset);
		result += (byte & 0x7f) << bitOffset;
		if (byte & 0x80) {
			break;
		}
		bitOffset += 7;
		result += 1 << bitOffset;
	}

	return result;
}

int applyPatch(uint8_t *patch, size_t patchLen, uint8_t *input, size_t inputLen, uint8_t **output, size_t *outputLen) {
	size_t patchOffset = 0;
	size_t outputOffset = 0;

	// header and version ('BPS1') (4 bytes)
	char magicNum[5];
	magicNum[0] = readByte(patch, &patchOffset);
	magicNum[1] = readByte(patch, &patchOffset);
	magicNum[2] = readByte(patch, &patchOffset);
	magicNum[3] = readByte(patch, &patchOffset);
	magicNum[4] = 0;

	if (strcmp(magicNum, "BPS1") != 0) {
		log_printf(LL_ERROR, "BPS magic number doesn't match expected: %s\n", magicNum);
		return 1;
	}

	// input size (4 bytes, unsigned)
	uint32_t inputExpectedSize = decodeNumber(patch, &patchOffset);
	if (inputExpectedSize != inputLen) {
		log_printf(LL_ERROR, "Patch input of unexpected size!  expected: %d actual: %d\n", inputExpectedSize, inputLen);
		return 1;
	}

	*outputLen = decodeNumber(patch, &patchOffset);
	*output = malloc(*outputLen);

	uint32_t metadataLen = decodeNumber(patch, &patchOffset);
	patchOffset += metadataLen;	// skip metadata

	int32_t inpOffsetAcc = 0;
	int32_t outOffsetAcc = 0;
	while (patchOffset < patchLen - 12) {
		uint32_t segmentHead = decodeNumber(patch, &patchOffset);
		uint8_t segmentType = segmentHead & 0x03;
		uint32_t segmentLen = (segmentHead >> 2) + 1;
		switch(segmentType) {
			case 0x00: {
				memcpy((*output) + outputOffset, input + outputOffset, segmentLen);
				outputOffset += segmentLen;
				break;
			}
			case 0x01: {
				memcpy((*output) + outputOffset, patch + patchOffset, segmentLen);
				patchOffset += segmentLen;
				outputOffset += segmentLen;
				break;
			}
			case 0x02: {
				int32_t offset = decodeNumber(patch, &patchOffset);
				offset = (offset & 0x01) ? -(offset >> 1) : (offset >> 1);
				inpOffsetAcc += offset;
				memcpy((*output) + outputOffset, input + inpOffsetAcc, segmentLen);
				inpOffsetAcc += segmentLen;
				outputOffset += segmentLen;
				break;
			}
			case 0x03: {
				int32_t offset = decodeNumber(patch, &patchOffset);
				offset = (offset & 0x01) ? -(offset >> 1) : (offset >> 1);
				outOffsetAcc += offset;
				memcpy((*output) + outputOffset, (*output) + outOffsetAcc, segmentLen);
				outOffsetAcc += segmentLen;
				outputOffset += segmentLen;
				break;
			}
		}
	}

	uint32_t inputcrc = 0;
	inputcrc |= readByte(patch, &patchOffset);
	inputcrc |= readByte(patch, &patchOffset) << 8;
	inputcrc |= readByte(patch, &patchOffset) << 16;
	inputcrc |= readByte(patch, &patchOffset) << 24;
	if (inputcrc != crc32(input, inputLen)) {
		log_printf(LL_ERROR, "INPUT CRC DID NOT MATCH: %x %x\n", inputcrc, crc32(input, inputLen));
		goto failure;
	}

	uint32_t outputcrc = 0;
	outputcrc |= readByte(patch, &patchOffset);
	outputcrc |= readByte(patch, &patchOffset) << 8;
	outputcrc |= readByte(patch, &patchOffset) << 16;
	outputcrc |= readByte(patch, &patchOffset) << 24;
	if (outputcrc != crc32(*output, *outputLen)) {
		log_printf(LL_ERROR, "OUTPUT CRC DID NOT MATCH: %x %x\n", outputcrc, crc32(*output, *outputLen));
		goto failure;
	}

	uint32_t patchcrc = 0;
	patchcrc |= readByte(patch, &patchOffset);
	patchcrc |= readByte(patch, &patchOffset) << 8;
	patchcrc |= readByte(patch, &patchOffset) << 16;
	patchcrc |= readByte(patch, &patchOffset) << 24;
	if (patchcrc != crc32(patch, patchLen - 4)) {
		log_printf(LL_ERROR, "PATCH CRC DID NOT MATCH: %x %x\n", patchcrc, crc32(patch, patchLen));
		goto failure;
	}

	return 0;

failure:
	free(*output);
	*output = NULL;
	return 1;
}