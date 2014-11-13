/*
 * do_verification.h
 *
 *  Created on: 26.5.2014
 *      Author: Vladimir
 */

#ifndef DO_VERIFICATION_H_
#define DO_VERIFICATION_H_

#define MD5_HASH_SIZE 16
#define BYTE_WIDTH 0x0
#define HALFWORD_WIDTH 0x01
#define WORD_WIDTH 0x02
#define HASH_DESTINATION_32K_ADDRESS 0x10006000
#define HASH_DESTINATION_16K_ADDRESS 0x2007FFF0
#define INITIAL_CHUNK_32K_ADDRESS 0x10000010
#define INITIAL_HASH_32K_ADDRESS 0x10000000
#define INITIAL_CHUNK_16K_ADDRESS 0x2007C010
#define INITIAL_HASH_16K_ADDRESS 0x2007C000
#define INITIAL_FLASH_ADDRESS 0x00005000
#define INITIAL_32K_ADDRESS 0x10000000
#define INITIAL_16K_ADDRESS 0x2007C000
#define NUMBER_OF_CHUNKS 3

/*
 * Структура во која се чуваат информации за верификацијата на архивата.
 * -numberOfTransferredChunks е број на префрлени парчиња во даден момент;
 * -numberOfVerifiedChunks е број на верифицирани парчиња во даден момент;
 * -numberOfChunks е вкупен број на парчиња кои што треба да бидат префрлени и верифицирани;
 * -sizeOfChunk е големина на едно парче.
 */

typedef struct {
	uint16_t numberOfTransferredChunks;
	uint16_t numberOfVerifiedChunks;
	uint16_t numberOfChunks;
	uint32_t sizeOfChunk;
} Details;

//

/*
 * Функција за иницијализација на структурата.
 * -*d е структирата која се иницијализира;
 * -numberOfChunks е број на парчиња;
 * -sizeOfChunk е големина на едно парче
 */
void initDetails(Details* d, uint16_t numberOfChunks, uint32_t sizeOfChunk);

/*
 * Функција за иницијализација на тастер
 */
void initButton(void);

/*
 * Функција за вклучување на DMA контролерот
 */
void powerDMA(void);

/*
 * Функција за добивање на бројот на парчиња
 */
uint16_t getNumberOfChunks(void);

/*
 * Функција за добивање на големината на едно парче
 */
uint32_t getSizeOfChunk(void);

/*
 * Функција за добивање на преамбулата
 */
uint16_t getPreamble(void);

/*
 * Функција за проверка на подглавјето. Враќа 0 за неточно и 1 за точно.
 */
int checkEnd(Details *d);

/*
 * Функција за почетен DMA трансфер;
 * -sourceAddress е изворна адреса;
 * -destinationAddress е одредишна адреса;
 * -numberOfChunksToTransfer е број на парчиња што ќе се пренесат;
 * -*d е тековната структура за преносот и верификацијата.
 */
void initialDMATransfer(uint32_t sourceAddress, uint32_t destinationAddress,
		int numberOfChunksToTransfer, Details* d);

/*
 * Функција за иницијализација на DMA контролерот
 */
void initDMA();

/*
 * Функција за верифицирање на парчињата сместени во 32K SRAM.Враќа 0 за неточно и 1 за точно.
 * -numberOfChunksToVerify е број на парчиња кои треба да се верификуваат
 * -*d е тековната структура за преносот и верификацијата.
 */
int doVerification32(int numberOfChunksToVerify, Details *d);

/*
 * Функција за пренесување на парчиња во 16K SRAM.
 * -sourceAddress е изворна адреса;
 * -destinationAddress е одредишна адреса;
 * -numberOfChunksToTransfer е број на парчиња што ќе се пренесат;
 * -*d е тековната структура за преносот и верификацијата.
 */
void startDMATransfer16(uint32_t sourceAddress, uint32_t destinationAddress,
		int numberOfChunksToTransfer, Details *d);

/*
 * Функција за пренесување на парчиња во 32K SRAM.
 * -sourceAddress е изворна адреса;
 * -destinationAddress е одредишна адреса;
 * -numberOfChunksToTransfer е број на парчиња што ќе се пренесат;
 * -*d е тековната структура за преносот и верификацијата.
 */
void startDMATransfer32(uint32_t sourceAddress, uint32_t destinationAddress,
		int numberOfChunksToTransfer, Details *d);


/*
 * Функција со која што се пренесуваат парчиња во некоја од мемориите
 * -sourceAddress е изворна адреса;
 * -destinationAddress е одредишна адреса;
 * -numberOfChunksToTransfer е број на парчиња што ќе се пренесат;
 * -*d е тековната структура за преносот и верификацијата.
 */
void startDMATransfer(uint32_t sourceAddress, uint32_t destinationAddress,
		int numberOfChunksToTransfer, Details *d);

/*
 * Функција за пресметка на MD5 hash.
 * -*hashDestination е поле каде што се сместува пресметаниот хеш
 * -*chunk е парчето за кое се пресметува хеш
 * -dataSize е големина на парчето
 */
void calculateHash(uint8_t* hashDestination, uint8_t *chunk, uint32_t dataSize);

/*
 * Функција за пренос на парчиња со DMA контролер
 * -sourceAddress е изворна адреса;
 * -destinationAddress е одредишна адреса;
 * -numberOfChunksToTransfer е број на парчиња што ќе се пренесат;
 * -*d е тековната структура за преносот и верификацијата.
 */
void TransferChunks(uint32_t sourceAddress, uint32_t destinationAddress,
		int numberOfChunksToTransfer, Details* d);

// за поставување на регистрите во DMA контролерот

/*
 * Функција за поставување на регистрите во DMA контролерот.
 * -sourceAddress е изворна адреса;
 * -destinationAddress е одредишна адреса;
 * -transferSize е број на трансфери што ќе направи DMA контролерот;
 * -sWidth е бројот на бајти од изворот што ќе се пренесуваат при еден пренос;
 * -dWidth е бројот на бајти од одредиштето што ќе се запишуваат при еден пренос;
 */
void setDMARegisters(uint32_t sourceAddress, uint32_t destinationAddress,
		uint16_t transferSize, uint8_t sWidth, uint8_t dWidth);

/*
 * Функција за верифицирање на парчињата сместени во 16K SRAM.Враќа 0 за неточно и 1 за точно.
 * -numberOfChunksToVerify е број на парчиња кои треба да се верификуваат
 * -*d е тековната структура за преносот и верификацијата.
 */
int doVerification16(int numberOfChunksToVerify, Details *d);

//за проверка на тековно пресметаниот hash и претходниот

/*
 * Функција за споредба на пресметаниот и оригиналниот хеш.Враќа 0 за неточно и 1 за точно.
 * -calculatedHash е тековно пресметаниот хеш
 * -originalHash е оригиналниот хеш
 */
int compareHashes(uint8_t* calculatedHash, uint8_t* originalHash);

/*
 * Главна функција во која што зе извршува целокупната преоверка. Враќа 0 за неточно и 1 за точно.
 * -*d е тековната структура за преносот и верификацијата.
 * -whereToVerify означува во која меморија се проверува, 32К/16K SRAM
 */
int mainFunction(Details* d, int whereToVerify);

/*
 * Функција за верифицирање на парчињата сместени во некоја од мемориите.
 * Враќа 0 за неточно и 1 за точно.
 * -numberOfChunksToVerify е број на парчиња кои треба да се верификуваат
 * -*d е тековната структура за преносот и верификацијата.
 * -chunkAddress е адреса на првото парче
 * -hashAddress е адреса на оригиналниот хеш за соодветното парче
 */
int doVerification(int numberOfChunksToVerify, Details *d,
		uint32_t chunkAddress, uint32_t hashAddress);

/*
 * Функција за верифицирање на парчињата едно по едно. Враќа 0 за неточно и 1 за точно.
 * -numberOfChunksToVerify е број на парчиња кои треба да се верификуваат
 * -*whereToVerify е во кој дел од меморијата треба да се верифицира, 32К/16К
 * -*d е тековната структура за преносот и верификацијата.
 */
int singleVerification(int numberOfChunksToVerify, int *whereToVerify,
		Details *d);

/*
 * Функција за трепкање на диода
 * -time е фреквенција на трепкање
 */
void blinkLed(int time);

/*
 * Функција за пренос и верифицирање во соодветна меморија. Враќа 0 за неточно и 1 за точно.
 * -*d е тековната структура за преносот и верификацијата
 * -*whereToVerify е во кој дел од меморијата треба да се верифицира, 32К/16К
 */
int transferAndVerify(Details *d, int* whereToVerify);

#endif /* DO_VERIFICATION_H_ */
