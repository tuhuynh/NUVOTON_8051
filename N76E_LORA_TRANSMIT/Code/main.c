 //***********************************************************************************************************
//  File Function: LORA Ra01 transmit using N76E003 SPI in Master mode 
//	HTTU - 2018
// ref: N76E003_BSP_Keil_C51_V1.0.5
//***********************************************************************************************************
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"
#include "main.h"
//***********************************************************************************************************
#include "main.h"

#define LED1 P12 //status led
#define LED2 P13 //error led.
#define LORA_SS P15

typedef uint32_t uint64_t[2];
int _implicitHeaderMode = 0;
/////prototype of function

void SPI_Initial(void);
uint8_t readRegister(uint8_t address);
void writeRegister(uint8_t address, uint8_t value);
void setFrequency(long frequency);
void setTxPower(int level);
void implicitHeaderMode();
void explicitHeaderMode();
void ERROR();
void loraBegin(long frequency);
int beginPacket(int implicitHeader);
int endPacket();
uint8_t loraWrite(const uint8_t *buffer, uint8_t size);
void loraSleep();
void loraIdle();

///////end of prototype ////////////////////
void main()
{	uint8_t value=0;
 	Set_All_GPIO_Quasi_Mode;
	P12_Quasi_Mode;	  //led 1
	P13_Quasi_Mode;	//led 2	
	SPI_Initial();
	LORA_SS=1;
	loraBegin(433);
	
	while(1)
	{
	 beginPacket(0);
	 loraWrite("HELLO",5); //test with HELLO	 
	 endPacket();								 
	 Timer0_Delay1ms(3000);	//flash led with duration 3 seconds.
	 LED1=LED1^0x1;
	}

}

///*********************spi init****************************************************************
void SPI_Initial(void)
{      
	P15_Quasi_Mode;														
	P10_Quasi_Mode;														// P10(SPCLK) Quasi mode
	P00_Quasi_Mode;														// P00 (MOSI) Quasi mode
    P01_Quasi_Mode;														// P22 (MISO) Quasi mode
	  
    
    set_DISMODF;                                // SS General purpose I/O ( No Mode Fault ) 
    clr_SSOE;
   
    clr_LSBFE;                                  // MSB first         
    clr_CPOL;                                   // The SPI clock is low in idle mode
    clr_CPHA;                                   // The data is sample on the second edge of SPI clock 
    set_MSTR;                                   // SPI in Master mode 
    SPICLK_DIV2;                 				// Select SPI clock 
    set_SPIEN;                                  // Enable SPI function 
    clr_SPIF;
}
/******************************************************************************************
* error function
*
******************************************************************************************
*/
void ERROR()
{	 //LED2 flash when system error.
while(1){
	LED2=0;
	Timer0_Delay1ms(500);
	LED2=1;
	Timer0_Delay1ms(500);
	}
}


/******************************************************************************************
* transmit array
*
******************************************************************************************
*/
uint8_t loraWrite(const uint8_t *buffer, uint8_t size)
{
	uint8_t i;
  int currentLength = readRegister(REG_PAYLOAD_LENGTH);

  // check size
  if ((currentLength + size) > MAX_PKT_LENGTH) {
    size = MAX_PKT_LENGTH - currentLength;
  }

  // write data
  for (i = 0; i < size; i++) {
    writeRegister(REG_FIFO, buffer[i]);
  }

  // update length
  writeRegister(REG_PAYLOAD_LENGTH, currentLength + size);

  return size;
}

/******************************************************************************************
* initial lora with specific frequency
*
******************************************************************************************
*/
void loraBegin(long frequency)
{
	  uint8_t value=0;
	  value=readRegister(REG_VERSION)  ;

	  if(value!=0x12) ERROR(); //0x12 is ID of RA01
	  loraSleep();
	  setFrequency(433E6);
		writeRegister(REG_FIFO_TX_BASE_ADDR, 0); //
  		writeRegister(REG_FIFO_RX_BASE_ADDR, 0);
		// set LNA boost
  writeRegister(REG_LNA, readRegister(REG_LNA) | 0x03);

  // set auto AGC
  writeRegister(REG_MODEM_CONFIG_3, 0x04);
   // set output power to 17 dBm
  setTxPower(14);
   // put in standby mode
  loraIdle();

}
/******************************************************************************************
* read lora register.
* to read sx1278 register:
* 	- first writing byte is the register address want to read.
*	- second writing byte is default value 0;
*	- read SPDR value return from MISO.
******************************************************************************************
*/
uint8_t readRegister(uint8_t address)
{
		int i=0;
		uint8_t readvalue=0;
 		LORA_SS=0;
		clr_SPIF;
		SPDR=address&0x7f;
		while(!(SPSR&0x80)){}
		clr_SPIF;

		SPDR=0x00;
		for(i=0;i<10;i++){}
		while(!(SPSR&0x80)){}
		clr_SPIF;
	  	LORA_SS=1;
		readvalue=SPDR;
		return readvalue;
}

/******************************************************************************************
* write lora register.
* to wire sx1278 register vith value:
* 	- first writing byte is the register address want to write.
*	- second writing byte is the value want to write;
*	
******************************************************************************************
*/
void writeRegister(uint8_t address, uint8_t value)
{
		
		LORA_SS=0;
		clr_SPIF;
		SPDR=(address|0x80);
		while(!(SPSR&0x80)){}
		clr_SPIF;
  		SPDR=value;
		while(!(SPSR&0x80)){}
		clr_SPIF;
	  	LORA_SS=1;
}

/******************************************************************************************
* write a string to FIFO SX1278
* 	
******************************************************************************************
*/
uint8_t write(const uint8_t *buffer, uint8_t size)
{
	uint8_t i;
  	int currentLength = readRegister(REG_PAYLOAD_LENGTH);
 // check size
  if ((currentLength + size) > MAX_PKT_LENGTH) {
    size = MAX_PKT_LENGTH - currentLength;
  }

  // write data
  for (i = 0; i < size; i++) {
    writeRegister(REG_FIFO, buffer[i]);
  }

  // update length
  writeRegister(REG_PAYLOAD_LENGTH, currentLength + size);

  return size;
}

/******************************************************************************************
* set operating frequency of LORA
* 	
******************************************************************************************
*/
void setFrequency(long frequency)
{
  
  //uint64_t frf = ((uint64_t)frequency << 19) / 32000000;

  //writeRegister(REG_FRF_MSB, (uint8_t)(frf >> 16));
  //writeRegister(REG_FRF_MID, (uint8_t)(frf >> 8));
  //writeRegister(REG_FRF_LSB, (uint8_t)(frf >> 0));
  writeRegister(REG_FRF_LSB,0x0);
  writeRegister(REG_FRF_MID,0x40);
  writeRegister(REG_FRF_MSB,0x6C);


}
/******************************************************************************************
* set transmit power
* 	
******************************************************************************************
*/
void setTxPower(int level)
{
if (level < 2) {
      level = 2;
    } else if (level > 17) {
      level = 17;
    }

    writeRegister(REG_PA_CONFIG, PA_BOOST | (level - 2));
}

/******************************************************************************************
* begin to send packet
* 	
******************************************************************************************
*/
int beginPacket(int implicitHeader)
{
    // put in standby mode
  loraIdle();

  if (implicitHeader) {
    implicitHeaderMode();
  } else {
    explicitHeaderMode();
  }
  
  // reset FIFO address and paload length
  writeRegister(REG_FIFO_ADDR_PTR, 0);
  writeRegister(REG_PAYLOAD_LENGTH, 0);

  return 1;
}
/******************************************************************************************
* end to send process
* 	
******************************************************************************************
*/
int endPacket()
{
  // put in TX mode
  writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
   // wait for TX done
  while ((readRegister(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) == 0) {}
   // clear IRQ's
  writeRegister(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
  return 1;
}

void explicitHeaderMode()
{
   writeRegister(REG_MODEM_CONFIG_1, readRegister(REG_MODEM_CONFIG_1) & 0xfe);
}

void implicitHeaderMode()
{
   writeRegister(REG_MODEM_CONFIG_1, readRegister(REG_MODEM_CONFIG_1) | 0x01);
}

void loraIdle()
{
  writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
}

void loraSleep()
{
  writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
}