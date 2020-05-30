#include "NRF24L01.h"
#include "ILI9327.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "GUI.h"
#include "semphr.h"

//#define BitIsSet(reg, bit) ((reg & (1<<(bit))) != 0)

uint8_t PayloadSize; //maximum data to be sent in one packet
extern SemaphoreHandle_t xSPI1_Mutex;

uint8_t MyAddress[] = {
    0xE7,
    0xE7,
    0xE7,
    0xE7,
    0xE7
};
/* Receiver address */
uint8_t TxAddress[] = {
    0x7E,
    0x7E,
    0x7E,
    0x7E,
    0x7E
};

uint16_t RxFailCounter = 0;

extern uint16_t PValue;
extern uint16_t DValue;
extern uint16_t IValue;
extern int angleVal;
extern int16_t PWMVal;
extern int16_t speedVal;


//channel you will use, from 0 to 125 eg. working frequency from 2.4 to 2.525 GHz
//maximum data to be sent in one packet
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
uint8_t NRF24L01_Init(uint8_t channel, uint8_t payload_size)
{
	//Initialize CE and CSN pins
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	//Common settings
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;

	//CSN pins
	GPIO_InitStruct.GPIO_Pin = NRF24L01_CSN_PIN;
	GPIO_Init(NRF24L01_CSN_PORT, &GPIO_InitStruct);
	//CE pins
	GPIO_InitStruct.GPIO_Pin = NRF24L01_CE_PIN;
	GPIO_Init(NRF24L01_CE_PORT, &GPIO_InitStruct);

	NRF24L01_CE_LOW;
	NRF24L01_CSN_HIGH;

	//Initialize SPI
	/*GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;


	//Enable clock

	//Pinspack nr. 2           SCK           MISO          MOSI
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);
*/
/*
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);

	SPI_InitTypeDef SPI_InitStruct;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI1, &SPI_InitStruct);

	SPI_Cmd(SPI1, ENABLE);*/
	
	//Max payload is 32bytes
	if (payload_size > 32)
		payload_size = 32;
	
	PayloadSize = payload_size;
	
	//Reset nRF24L01+ to power on registers values
	NRF24L01_SoftwareReset();
	
	//Channel select
	NRF24L01_SetChannel(channel);
	
	//Set pipeline to max possible 32 bytes
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P0, PayloadSize); // Auto-ACK pipe
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P1, PayloadSize); // Data payload pipe
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P2, PayloadSize);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P3, PayloadSize);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P4, PayloadSize);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P5, PayloadSize);
	
	//Set RF settings (2mbps, output power)
	NRF24L01_SetRF(NRF24L01_DataRate_2M, NRF24L01_OutputPower_0dBm);
	
	//Config register
	NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG);
	
	//Enable auto-acknowledgment for all pipes
	NRF24L01_WriteRegister(NRF24L01_REG_EN_AA, 0x3F);
	
	//Enable RX addresses
	NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, 0x3F);

	//Auto retransmit delay: 1000 (4x250) us and Up to 15 retransmit trials
	NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, 0x4F);
	
	//Dynamic length configurations: No dynamic length
	NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, (0 << NRF24L01_DPL_P0) | (0 << NRF24L01_DPL_P1) | (0 << NRF24L01_DPL_P2) | (0 << NRF24L01_DPL_P3) | (0 << NRF24L01_DPL_P4) | (0 << NRF24L01_DPL_P5));
	
	//Clear FIFOs
	NRF24L01_FLUSH_TX;
	NRF24L01_FLUSH_RX;
	
	//Clear interrupts
	NRF24L01_CLEAR_INTERRUPTS;
	
	//Go to RX mode
	NRF24L01_PowerUpRx();
	
	return 1;
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_SetMyAddress(uint8_t *adr)
{
	NRF24L01_CE_LOW;
	NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, adr, 5);
	NRF24L01_CE_HIGH;
}

/**
 * Set address you will communicate with
 *
 * Parameters:
 * 	- uint8_t* adr: pointer to 5 bytes of address
 */
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_SetTxAddress(uint8_t *adr)
{
	NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, adr, 5);
	NRF24L01_WriteRegisterMulti(NRF24L01_REG_TX_ADDR, adr, 5);
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_SetPipe2Address(uint8_t adr)
{
	NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P2, adr);
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_SetPipe3Address(uint8_t adr)
{
	NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P3, adr);
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_SetPipe4Address(uint8_t adr)
{
	NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P4, adr);
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_SetPipe5Address(uint8_t adr)
{
	NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P5, adr);
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_WriteBit(uint8_t reg, uint8_t bit, BitAction value)
{
	uint8_t tmp = NRF24L01_ReadRegister(reg);
	if(value != Bit_RESET)
		tmp |= 1 << bit;
	else
		tmp &= ~(1 << bit);

	NRF24L01_WriteRegister(reg, tmp);
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
uint8_t NRF24L01_ReadBit(uint8_t reg, uint8_t bit)
{
	uint8_t tmp = NRF24L01_ReadRegister(reg);
	if (!BitIsSet(tmp, bit))
		return 0;

	return 1;
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
uint8_t NRF24L01_ReadRegister(uint8_t reg)
{
	NRF24L01_CSN_LOW;
	SPI_Send(NRF24L01_READ_REGISTER_MASK(reg));
	uint8_t value = SPI_Send(NRF24L01_NOP_MASK);
	NRF24L01_CSN_HIGH;
	
	return value;
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_ReadRegisterMulti(uint8_t reg, uint8_t* data, uint8_t count)
{
	NRF24L01_CSN_LOW;
	SPI_Send(NRF24L01_READ_REGISTER_MASK(reg));
	SPI_ReadMass(data, NRF24L01_NOP_MASK, count);
	NRF24L01_CSN_HIGH;
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_WriteRegister(uint8_t reg, uint8_t value)
{
	NRF24L01_CSN_LOW;
	SPI_Send(NRF24L01_WRITE_REGISTER_MASK(reg));
	SPI_Send(value);
	NRF24L01_CSN_HIGH;
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t count)
{
	NRF24L01_CSN_LOW;
	SPI_Send(NRF24L01_WRITE_REGISTER_MASK(reg));
	SPI_WriteMass(data, count);
	NRF24L01_CSN_HIGH;
}

/**
 * Put NRF24L01 to TX mode
 *
 *
 */
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_PowerUpTx()
{
	NRF24L01_CLEAR_INTERRUPTS;
	NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG | (0 << NRF24L01_PRIM_RX) | (1 << NRF24L01_PWR_UP));
}

/**
 * Put NRF24L01 to RX mode
 */
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_PowerUpRx()
{
	NRF24L01_FLUSH_RX;
	NRF24L01_CLEAR_INTERRUPTS;
	NRF24L01_CE_LOW;
	NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG | 1 << NRF24L01_PWR_UP | 1 << NRF24L01_PRIM_RX);
	NRF24L01_CE_HIGH;
	//HERE MUST BE DELAY 135us
}

/**
 *
 *
 *
 */
/*******************************************************************************
* Function Name  :
* Description    : Put NRF24L01 to power down mode
* 				   In this mode, you will not be able to receive or transmit data
*******************************************************************************/
void NRF24L01_PowerDown()
{
	NRF24L01_CE_LOW;
	NRF24L01_WriteBit(NRF24L01_REG_CONFIG, NRF24L01_PWR_UP, Bit_RESET);
}

/**
 *
 *
 * Parameters:
 * 	- uint8_t *data: Pointer to bytes to be sent, max length of payload_size
 */
/*******************************************************************************
* Function Name  :
* Description    : Transmit data with NRF24L01
*******************************************************************************/
void NRF24L01_Transmit(uint8_t *data)
{
	uint8_t count = PayloadSize;

	//Chip enable put to low, disable it
	NRF24L01_CE_LOW;
	
	//Go to power up tx mode
	NRF24L01_PowerUpTx();
	
	//Clear TX FIFO from NRF24L01+
	NRF24L01_FLUSH_TX;
	
	//Send payload to nRF24L01+
	NRF24L01_CSN_LOW;
	//Send write payload command
	SPI_Send(NRF24L01_W_TX_PAYLOAD_MASK);
	//Fill payload with data
	SPI_WriteMass(data, count);
	NRF24L01_CSN_HIGH;
	//HERE MUST BE DELAY 135us
	
	//Delay(1000);
	//Send data!
	NRF24L01_CE_HIGH;
}

/**
 *
 *
 * Parameters:
 * 	- uint8_t *data: pointer where to save data
 */
/*******************************************************************************
* Function Name  :
* Description    : Get data from NRF24L01
*******************************************************************************/
void NRF24L01_GetData(uint8_t* data)
{
	//Pull down chip select
	NRF24L01_CSN_LOW;
	//Send read payload command
	SPI_Send(NRF24L01_R_RX_PAYLOAD_MASK);
	//Read payload
	SPI_WrRdMass(data, data, PayloadSize);
	//Pull up chip select
	NRF24L01_CSN_HIGH;
	
	//Reset status register, clear RX_DR interrupt flag
	NRF24L01_WriteRegister(NRF24L01_REG_STATUS, (1 << NRF24L01_RX_DR));
}

/**
 *
 * Returns 1 if ready, 0 if not
 */
/*******************************************************************************
* Function Name  :
* Description    : Checks if data is ready to be read from NRF24L01
*******************************************************************************/
uint8_t NRF24L01_DataReady()
{
	if(BitIsSet(NRF24L01_GetStatus(), NRF24L01_RX_DR)) return 1;
	return !NRF24L01_RxFifoEmpty();
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
uint8_t NRF24L01_RxFifoEmpty() {
	return BitIsSet(NRF24L01_ReadRegister(NRF24L01_REG_FIFO_STATUS), NRF24L01_RX_EMPTY);
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
uint8_t NRF24L01_GetStatus()
{
	NRF24L01_CSN_LOW;
	//First received byte is always status register
	uint8_t status = SPI_Send(NRF24L01_NOP_MASK);
	NRF24L01_CSN_HIGH;
	
	return status;
}

/**
 *
 *
 * Returns members of NRF24L01_Transmit_Status_t struct:
 * 	- NRF24L01_Transmit_Status_Lost if message is lost
	- NRF24L01_Transmit_Status_Ok if message is OK sent
	- NRF24L01_Transmit_Status_Sending if message is still sending
 */
/*******************************************************************************
* Function Name  :
* Description    : Get transmissions status
*******************************************************************************/
NRF24L01_Transmit_Status_t NRF24L01_GetTransmissionStatus()
{
	uint8_t status = NRF24L01_GetStatus();
	if(BitIsSet(status, NRF24L01_TX_DS))
		return NRF24L01_Transmit_Status_Ok;
	else if(BitIsSet(status, NRF24L01_MAX_RT))
		return NRF24L01_Transmit_Status_Lost;
	
	return NRF24L01_Transmit_Status_Sending;
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NRF24L01_SoftwareReset(void)
{
	uint8_t data[5];
	
	NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, 	NRF24L01_REG_DEFAULT_VAL_CONFIG);
	NRF24L01_WriteRegister(NRF24L01_REG_EN_AA,		NRF24L01_REG_DEFAULT_VAL_EN_AA);
	NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, 	NRF24L01_REG_DEFAULT_VAL_EN_RXADDR);
	NRF24L01_WriteRegister(NRF24L01_REG_SETUP_AW, 	NRF24L01_REG_DEFAULT_VAL_SETUP_AW);
	NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, NRF24L01_REG_DEFAULT_VAL_SETUP_RETR);
	NRF24L01_WriteRegister(NRF24L01_REG_RF_CH, 		NRF24L01_REG_DEFAULT_VAL_RF_CH);
	NRF24L01_WriteRegister(NRF24L01_REG_RF_SETUP, 	NRF24L01_REG_DEFAULT_VAL_RF_SETUP);
	NRF24L01_WriteRegister(NRF24L01_REG_STATUS, 	NRF24L01_REG_DEFAULT_VAL_STATUS);
	NRF24L01_WriteRegister(NRF24L01_REG_OBSERVE_TX, NRF24L01_REG_DEFAULT_VAL_OBSERVE_TX);
	NRF24L01_WriteRegister(NRF24L01_REG_RPD, 		NRF24L01_REG_DEFAULT_VAL_RPD);

	//P0
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_4;
	NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, data, 5);
	
	//P1
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_4;
	NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, data, 5);
	
	NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P2, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P2);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P3, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P3);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P4, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P4);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P5, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P5);
	
	//TX
	data[0] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_4;
	NRF24L01_WriteRegisterMulti(NRF24L01_REG_TX_ADDR, data, 5);

	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P0, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P0);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P1, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P1);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P2, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P2);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P3, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P3);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P4, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P4);
	NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P5, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P5);
	NRF24L01_WriteRegister(NRF24L01_REG_FIFO_STATUS, NRF24L01_REG_DEFAULT_VAL_FIFO_STATUS);
	NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, 		NRF24L01_REG_DEFAULT_VAL_DYNPD);
	NRF24L01_WriteRegister(NRF24L01_REG_FEATURE, 	NRF24L01_REG_DEFAULT_VAL_FEATURE);
}

/**
 *
 *
 * Returns number from 0 to 15
 */
/*******************************************************************************
* Function Name  :
* Description    : Get number of transmissions needed in last sent
*******************************************************************************/
uint8_t NRF24L01_GetRetransmissionsCount(void)
{//Low 4 bits
	return NRF24L01_ReadRegister(NRF24L01_REG_OBSERVE_TX) & 0x0F;
}

/**
 *
 *
 * Parameters:
 * 	- uint8_t channel: channel from 0 to 125
 */
/*******************************************************************************
* Function Name  :
* Description    : Set working channel
*******************************************************************************/
void NRF24L01_SetChannel(uint8_t channel)
{
	if (channel <= 125)
		NRF24L01_WriteRegister(NRF24L01_REG_RF_CH, channel);
}

/**
 *
 *
 * Parameters:
 * 	- NRF24L01_DataRate_t DataRate: Data rate
 * 		- NRF24L01_DataRate_2M			2Mbps datarate
 * 		- NRF24L01_DataRate_1M			1Mbps datarate
 * 		- NRF24L01_DataRate_250k			250k datarate
 * 	- NRF24L01_OutputPower_t OutPwr: output power
 * 		- NRF24L01_OutputPower_M18dBm	-18 dBm power
 * 		- NRF24L01_OutputPower_M12dBm	-12 dBm power
 * 		- NRF24L01_OutputPower_M6dBm		-6 dBm power
 * 		- NRF24L01_OutputPower_0dBm		0dBm power
 */
/*******************************************************************************
* Function Name  :
* Description    : Set RF parameters for NRF24L01
*******************************************************************************/
void NRF24L01_SetRF(NRF24L01_DataRate_t DataRate, NRF24L01_OutputPower_t OutPwr)
{
	uint8_t tmp = 0;
	
	if (DataRate == NRF24L01_DataRate_2M) {
		tmp |= 1 << NRF24L01_RF_DR_HIGH;
	} else if (DataRate == NRF24L01_DataRate_250k) {
		tmp |= 1 << NRF24L01_RF_DR_LOW;
	}
	//If 1Mbps, all bits set to 0
	
	if (OutPwr == NRF24L01_OutputPower_0dBm) {
		tmp |= 3 << NRF24L01_RF_PWR;
	} else if (OutPwr == NRF24L01_OutputPower_M6dBm) {
		tmp |= 2 << NRF24L01_RF_PWR;
	} else if (OutPwr == NRF24L01_OutputPower_M12dBm) {
		tmp |= 1 << NRF24L01_RF_PWR;
	}
	
	NRF24L01_WriteRegister(NRF24L01_REG_RF_SETUP, tmp);
}









/*******************************************************************************
* Function Name  : SPI_Send
* Description    :
*******************************************************************************/
uint8_t SPI_Send(uint8_t data)
{


	/* Fill output buffer with data */
	SPI1->DR = data;
	/* Wait for transmission to complete */
	while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
	/* Wait for received data to complete */
	while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
	/* Wait for SPI to be ready */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));
	/* Return data from buffer */
	uint8_t ret = SPI1->DR;

	return ret;
}


/*******************************************************************************
* Function Name  : SPI_WrRdMass
* Description    :
*******************************************************************************/
void SPI_WrRdMass(uint8_t* dataOut, uint8_t* dataIn, uint16_t count) {
	for(uint16_t i=0; i<count; i++)
		dataIn[i] = SPI_Send(dataOut[i]);
}

/*******************************************************************************
* Function Name  : SPI_WriteMass
* Description    :
*******************************************************************************/
void SPI_WriteMass(uint8_t* dataOut, uint16_t count)
{
	for(uint16_t i=0; i<count; i++)
		SPI_Send(dataOut[i]);
}

/*******************************************************************************
* Function Name  : SPI_ReadMass
* Description    :
*******************************************************************************/
void SPI_ReadMass(uint8_t* dataIn, uint8_t dummy, uint16_t count)
{
	for(uint16_t i=0; i<count; i++)
		dataIn[i] = SPI_Send(dummy);
}

extern const unsigned char SmallFont[];
/*******************************************************************************
* Function Name  : SPI_ReadMass
* Description    :
*******************************************************************************/
void Radio_Task(void * pvParameters)
{



	uint8_t dataOut[32], dataIn[32];

	for(uint8_t i=0; i<32; i++)
		dataOut[i]=i;

	//GUI_DispString("Hello world! THIS IS VERY FUCKING SHIT IF YOU KNOW WHAT I MEAN");

	//uint16_t i = 0;
	while(1)
	{

		for(uint8_t i=0; i<32; i++)
			dataOut[i]++;

		dataOut[0] = PValue & 0xFF;
		dataOut[1] = (PValue>>8) & 0xFF;
		dataOut[2] = DValue & 0xFF;
		dataOut[3] = (DValue>>8) & 0xFF;
		dataOut[4] = IValue & 0xFF;
		dataOut[5] = (IValue>>8) & 0xFF;

	while(xSemaphoreTake(xSPI1_Mutex, ( TickType_t ) 10000 ) == pdFALSE);
		/* Transmit data, goes automatically to TX mode */
		NRF24L01_Transmit(dataOut);

		/* Wait for data to be sent */
		while(NRF24L01_GetTransmissionStatus() == NRF24L01_Transmit_Status_Sending);

		vTaskDelay(1);

		/* Go back to RX mode */
		NRF24L01_PowerUpRx();

		/* Wait received data, wait max 100ms, if time is larger, then data were probably lost */
		RxFailCounter = 9/*ms*/ * 10;
		while(!NRF24L01_DataReady() && RxFailCounter);
		uint8_t RxStatus = (RxFailCounter != 0)?1:0;

		/* Get data from NRF2L01+ */
		NRF24L01_GetData(dataIn);
	xSemaphoreGive(xSPI1_Mutex);
		if(RxStatus)
		{
			angleVal = (dataIn[1]<<8) | dataIn[0];
			PWMVal = (dataIn[3]<<8) | dataIn[2];
			speedVal = (dataIn[5]<<8) | dataIn[4];
		}



		//static uint8_t i=0;

		//ILI9327_setFont(SmallFont);

		//ILI9327_setColor((RxStatus)?VGA_GREEN:VGA_RED);

		for(uint8_t i=0; i<6; i++)
		{
			char I[20] = "Rx";
			char temp[5]; temp[0] = i+'0';temp[1] = '\0';
			strcat_(I, temp);
			//ILI9327_NumWDesc(210, 12+i*12, I, dataIn[i]);

			char O[20] = "Tx";
			strcat_(O, temp);
			//ILI9327_NumWDesc(310, 12+i*12, O, dataOut[i]);
		}

		/*ILI9327_NumWDesc(200, 12+6*12, "Status:", NRF24L01_ReadRegister(NRF24L01_REG_STATUS));
		ILI9327_NumWDesc(300, 12+6*12, "FIFO:", NRF24L01_ReadRegister(NRF24L01_REG_FIFO_STATUS));

		ILI9327_NumWDesc(200, 12+7*12, "Ch:", NRF24L01_ReadRegister(NRF24L01_REG_RF_CH));
		ILI9327_NumWDesc(300, 12+7*12, "RFSt:", NRF24L01_ReadRegister(NRF24L01_REG_RF_SETUP));*/

/*int xPos = LCD_GetXSize() / 2;
int yPos = LCD_GetYSize() / 3;
GUI_SetFont(GUI_FONT_COMIC24B_ASCII);
GUI_DispStringHCenterAt("Hello world!", xPos, yPos);*/

		vTaskDelay(20);
	}

}


