/*
 * do_verification.c
 *
 *  Created on: 26.5.2014
 *      Author: Vladimir
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

#include "do_verification.h"
#include "md5.h"
#include "payload_generator.h"

void initDetails(Details* d, uint16_t numberOfChunks, uint32_t sizeOfChunk) {
	d->numberOfTransferredChunks = 0;
	d->numberOfVerifiedChunks = 0;
	d->numberOfChunks = numberOfChunks;
	d->sizeOfChunk = sizeOfChunk;
}

void initButton(void) {
	//pin dirrection setting
	LPC_GPIO2->FIODIR |= 0xFF;
	LPC_GPIO2->FIOCLR |= 0xFF;
	LPC_GPIO2->FIOSET |= 0xFF;
	LPC_PINCON->PINSEL4 |= 0x00400000;
	// to enable falling edge detected interrupt
	LPC_GPIOINT->IO2IntEnF |= 0x800;
	//to set up a pin internal pull up
	LPC_SC->EXTMODE |= 0x00000002;
	LPC_SC->EXTPOLAR |= 0;
	NVIC_EnableIRQ(EINT1_IRQn);
}

void powerDMA(void) {
	LPC_SC->PCONP |= 1 << 29;
}

uint16_t getNumberOfChunks(void) {
	uint8_t *ptr1 = (uint8_t*) (0x00004002);
	uint8_t *ptr2 = (uint8_t*) (0x00004003);

	return *ptr2 * 256 + *ptr1;
}

uint32_t getSizeOfChunk(void) {
	uint8_t *ptr1 = (uint8_t*) (0x00004007);
	uint8_t *ptr2 = (uint8_t*) (0x00004006);
	uint8_t *ptr3 = (uint8_t*) (0x00004005);
	uint8_t *ptr4 = (uint8_t*) (0x00004004);

	return *ptr1 * 256 * 256 * 256 + *ptr2 * 256 * 256 + *ptr3 * 256 + *ptr4;
}

uint16_t getPreamble() {
	uint8_t *ptr1 = (uint8_t*) (0x00004000);
	uint8_t *ptr2 = (uint8_t*) (0x00004001);

	return *ptr2 * 256 + *ptr1;
}

int checkEnd(Details *d) {
	uint8_t *ptr = (uint8_t *) (INITIAL_FLASH_ADDRESS
			+ d->numberOfChunks * (d->sizeOfChunk + MD5_HASH_SIZE));

	int i;
	for (i = 0; i < 8; i++) {
		if (ptr[i] != 0x0AB) {
			return 0;
		}
	}

	return 1;
}
void initDMA() {
	//вклучи го DMA контролерот
	powerDMA();

	//овозможи //провери ги парчињата
	LPC_GPDMA->DMACConfig = 0x01;

	LPC_GPDMA->DMACIntTCClear |= 0x0FF;

	//овозможи прекин
	NVIC_EnableIRQ(DMA_IRQn);
}

void setDMARegisters(uint32_t sourceAddress, uint32_t destinationAddress,
		uint16_t transferSize, uint8_t sWidth, uint8_t dWidth) {

	//поставување на изворна и одредишна адреса
	LPC_GPDMACH0->DMACCSrcAddr = sourceAddress;
	LPC_GPDMACH0->DMACCDestAddr = destinationAddress;

	LPC_GPDMACH0->DMACCControl = ((transferSize & 0x0FFF) | (0x0 << 12)
			| (0x0 << 15) | (sWidth << 18) | (dWidth << 21) | (1 << 26)
			| (1 << 27) | (1 << 31));

	//пренеси ги податоците
	LPC_GPDMACH0->DMACCConfig = 0x0C001;

}

void TransferChunks(uint32_t sourceAddress, uint32_t destinationAddress,
		int numberOfChunksToTransfer, Details* d) {

	d->numberOfTransferredChunks += numberOfChunksToTransfer;

	uint32_t numberOfBytes = numberOfChunksToTransfer
			* (d->sizeOfChunk + MD5_HASH_SIZE);

	//ќе се појави проблем ако numberOfBytes < 4

	if (numberOfBytes >= 4) {
		uint16_t transferSize = numberOfBytes / 4;
		setDMARegisters(sourceAddress, destinationAddress, transferSize,
		WORD_WIDTH,
		WORD_WIDTH);

		// тука ќе треба да се внимава дали претходниот трансфер завршил
		if (numberOfBytes % 4 != 0) {

			setDMARegisters(sourceAddress + numberOfBytes,
					destinationAddress + numberOfBytes, numberOfBytes % 4,
					BYTE_WIDTH, BYTE_WIDTH);

		}
	} // ако треба да се пренесат помалку од 4 бајти
	else {
		setDMARegisters(sourceAddress, destinationAddress, numberOfBytes % 4,
		BYTE_WIDTH, BYTE_WIDTH);
	}

}

void calculateHash(uint8_t* hashDestination, uint8_t *chunk, uint32_t dataSize) {
	int size;
	int index;
	MD5_CTX ctx;

	/* Initialize the MD5 context */
	MD5_Init(&ctx);

	/* Special case if data size is bellow MD5_BUFFER_SIZE_BYTES */
	if (dataSize < MD5_BUFFER_SIZE_BYTES) {
		MD5_Update(&ctx, chunk, dataSize);
	} else {
		/* Calculate MD5 hash with iterative calculation due to */
		/* 64B buffer space limitation of the MD5 API */
		for (size = 0; dataSize >= MD5_BUFFER_SIZE_BYTES; dataSize -=
		MD5_BUFFER_SIZE_BYTES) {
			index = size;
			size += MD5_BUFFER_SIZE_BYTES;
			MD5_Update(&ctx, &chunk[index], MD5_BUFFER_SIZE_BYTES);
		}

		if (dataSize > 0) {
			MD5_Update(&ctx, &chunk[size], dataSize);
		}
	}

	MD5_Final(hashDestination, &ctx);
}

void startDMATransfer(uint32_t sourceAddress, uint32_t destinationAddress,
		int numberOfChunksToTransfer, Details *d) {

	//ако треба да се пренесат сите парчиња наеднаш
	if (d->numberOfChunks - d->numberOfTransferredChunks
			>= numberOfChunksToTransfer) {
		TransferChunks(sourceAddress, destinationAddress,
				numberOfChunksToTransfer, d);
	}// ако треба да се пренесат парчињата едно по едно
	else {
		int i;
		int temp = d->numberOfChunks - d->numberOfTransferredChunks;
		for (i = 0; i < temp; i++) {
			TransferChunks(sourceAddress, destinationAddress, 1, d);
			sourceAddress += (d->sizeOfChunk + MD5_HASH_SIZE);
			destinationAddress += (d->sizeOfChunk + MD5_HASH_SIZE);
		}
	}
}

void initialDMATransfer(uint32_t sourceAddress, uint32_t destinationAddress,
		int numberOfChunksToTransfer, Details* d) {

	if (d->numberOfChunks - d->numberOfTransferredChunks
			>= numberOfChunksToTransfer) {
		TransferChunks(sourceAddress, destinationAddress,
				numberOfChunksToTransfer, d);
	} else {
		int i;
		int temp = d->numberOfChunks - d->numberOfTransferredChunks;
		for (i = 0; i < temp; i++) {
			TransferChunks(sourceAddress, destinationAddress, 1, d);
		}
	}

}

void startDMATransfer16(uint32_t sourceAddress, uint32_t destinationAddress,
		int numberOfChunksToTransfer, Details *d) {

	startDMATransfer(sourceAddress, destinationAddress,
			numberOfChunksToTransfer, d);
}

void startDMATransfer32(uint32_t sourceAddress, uint32_t destinationAddress,
		int numberOfChunksToTransfer, Details *d) {

	startDMATransfer(sourceAddress, destinationAddress,
			numberOfChunksToTransfer, d);
}

int doVerification(int numberOfChunksToVerify, Details *d, uint32_t chunkAdd,
		uint32_t hashAdd) {
	//почетна адреса на парчињата и на оригиналниот хеш
	uint32_t chunkAddress = chunkAdd;
	uint32_t hashAddress = hashAdd;

	uint8_t hashDestination[16];
	uint8_t* hash;
	uint8_t* chunk;
	int i;
	for (i = 0; i < numberOfChunksToVerify; i++) {
		chunk = (uint8_t*) (chunkAddress);
		hash = (uint8_t*) (hashAddress);
		//пресметај хеш на тековното парче
		calculateHash(&hashDestination[0], chunk, d->sizeOfChunk);

		//споредба со оригиналниот хеш
		if (compareHashes(hashDestination, hash) == 0) {
			return 0;
		}

		//пресметај адреса на следно парче и хеш
		chunkAddress += d->sizeOfChunk + MD5_HASH_SIZE;
		hashAddress += d->sizeOfChunk + MD5_HASH_SIZE;
		d->numberOfVerifiedChunks++;
	}

	return 1;
}

int doVerification32(int numberOfChunksToVerify, Details *d) {

	//провери ги парчињата
	if (doVerification(numberOfChunksToVerify, d,
	INITIAL_CHUNK_32K_ADDRESS, INITIAL_HASH_32K_ADDRESS) == 0) {
		return 0;
	}

	return 1;

}

int doVerification16(int numberOfChunksToVerify, Details *d) {

	//провери ги парчињата
	if (doVerification(numberOfChunksToVerify, d,
	INITIAL_CHUNK_16K_ADDRESS, INITIAL_HASH_16K_ADDRESS) == 0) {
		return 0;
	}

	return 1;

}

int transferAndVerify(Details *d, int* whereToVerify) {
	//пренесувај податоци во 16K SRAM и верифицирај во 32K SRAM
	if (*whereToVerify == 32) {
		startDMATransfer16(
				INITIAL_FLASH_ADDRESS
						+ d->numberOfTransferredChunks
								* (d->sizeOfChunk + MD5_HASH_SIZE),
				INITIAL_16K_ADDRESS, NUMBER_OF_CHUNKS, d);
		if (doVerification32(NUMBER_OF_CHUNKS, d) == 0) {
			return 0;
		}

		*whereToVerify = 16;
	} //пренесувај податоци во 32K SRAM и верифицирај во 16K SRAM
	else if (*whereToVerify == 16) {
		startDMATransfer32(
				INITIAL_FLASH_ADDRESS
						+ d->numberOfTransferredChunks
								* (d->sizeOfChunk + MD5_HASH_SIZE),
				INITIAL_32K_ADDRESS, NUMBER_OF_CHUNKS, d);
		if (doVerification16(NUMBER_OF_CHUNKS, d) == 0) {
			return 0;
		}
		*whereToVerify = 32;
	}

	return 1;
}

int singleVerification(int numberOfChunksToVerify, int *whereToVerify,
		Details *d) {
	int i;

	// за проверка во 32K SRAM
	if (*whereToVerify == 32) {
		for (i = 0; i < numberOfChunksToVerify; i++) {
			if (doVerification32(1, d) == 0) {
				return 0;
			}
		}
	} //за проверка во 1632K SRAM
	else if (*whereToVerify == 16) {
		for (i = 0; i < numberOfChunksToVerify; i++) {
			if (doVerification16(1, d) == 0) {
				return 0;
			}
		}
	}

	return 1;
}

int compareHashes(uint8_t* calculatedHash, uint8_t* originalHash) {
	int i;
	for (i = 0; i < 16; i++) {
		if (*calculatedHash != *originalHash) {
			return 0;
		}
		calculatedHash++;
		originalHash++;
	}
	return 1;
}

void blinkLed(int time) {
	// Set P0_22 to 00 - GPIO
	LPC_PINCON->PINSEL1 &= (~(3 << 12));
	// Set GPIO - P0_22 - to be output
	LPC_GPIO0->FIODIR |= (1 << 22);

	volatile static uint32_t i;
	while (1) {
		LPC_GPIO0->FIOSET = (1 << 22); // Turn LED2 on
		for (i = 0; i < time; i++)
			;
		LPC_GPIO0->FIOCLR = (1 << 22); // Turn LED2 off
		for (i = 0; i < time; i++)
			;
	}
}

int mainFunction(Details* d, int whereToVerify) {

	if (getPreamble() != 0xABBA) {
		return 0;
	}

	initialDMATransfer(INITIAL_FLASH_ADDRESS, INITIAL_32K_ADDRESS,
	NUMBER_OF_CHUNKS, d);

	//мало задоцнување на процесорот
	int counter = 0;

	while (counter < 2300) {
		counter++;
	}

	//пренесувај и верифицирај
	while (d->numberOfChunks - d->numberOfVerifiedChunks > NUMBER_OF_CHUNKS) {
		if (transferAndVerify(d, &whereToVerify) == 0) {
			return 0;
		}
	}

	//за останатиот дел од парчињата
	if (d->numberOfChunks - d->numberOfVerifiedChunks != 0) {
		if (singleVerification(d->numberOfChunks - d->numberOfVerifiedChunks,
				&whereToVerify, d) == 0) {
			return 0;
		}
	}

	//проверка на крајот
	if (checkEnd(d) == 0) {
		return 0;
	}

	return 1;
}
