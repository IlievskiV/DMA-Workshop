/*
 ===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif
#include <cr_section_macros.h>
#include "payload_generator.h"
#include "iap_driver.h"
#include "do_verification.h"
//#include "mbed.h"


static int buttonIterrupt = 1;


// handler за кога ќе биде притиснат тастерот
void EINT1_IRQHandler(void) {
	buttonIterrupt = 0;
	LPC_SC->EXTINT |= 0x2;
	LPC_GPIO2->FIOCLR |= 0xFF;

}

// кога ќе заврши DMA трансферот
void DMA_IRQHandler(void) {
	LPC_GPDMA->DMACIntTCClear |= 0x0FF;
}

int main(void) {
	e_iap_status iap_status;

	/* Fill the flash with payload and hash data */
	iap_status = (e_iap_status) generator_init();
	if (iap_status != CMD_SUCCESS)
		while (1)
			;   // Error !!!

	Details d;

	initButton();
    initDMA();
	initDetails(&d, getNumberOfChunks(), getSizeOfChunk());

	while (buttonIterrupt) {
	}

	int res = mainFunction(&d, 32);

	if(res==0) {
		blinkLed(1000000);
	}
	else {
		blinkLed(100);
	}


	return 0;
}
