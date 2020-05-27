#ifndef PCD8544_PCD8544
#define PCD8544_PCD8544

// если следующее определение закомментировано то работает програмный SPI
// если раскомментировано - то аппаратный SPI2
// по скорости у меня получилось что аппаратный SPI в 5 раза быстрее заполнял экран (SPI2 = 18 мс, против SOFT-SPI=97 мс)
#define PCD8544_SPI_HARD

// ИСПОЛЬЗОВАНИЕ DMA ПРИ ОБМЕНЕ С ДИСПЛЕЕМ
//#define PCD8544_SPI_DMA_HARD


// для простоты драйвера при программном SPI все линии управления и данных дисплея должны быть подключены к одному порту ввода\вывода !
// если же задан режим аппаратного SPI2 то, на один порт должны быть подключены линии RST, A0, CSE
// а остальные линии (SDA, SCK) должны быть подключены к GPIOB (пины 15 и 13 соответственно)
#define 	PCD8544_GPIO			GPIOB
#define     PCD8544_AHB1_GPIO		RCC_AHB1Periph_GPIOB
// пины дисплея (управление)
#define 	PCD8544_RST_PIN 		14
#define     PCD8544_A0_PIN			12
#define		PCD8544_CSE_PIN			10
//
// в случае если используется SPI2 подключение то вывод SDA дисплея должен быть подключен к GPIOB_Pin_15, и SCK к GPIOB_Pin_13
// остальные линии при SPI2 подключении могут быть подключены к любому порту (не только к GPIOB) см. выше
// пины дисплея (данные)

#define		PCD8544_SDA_PIN			15
#define		PCD8544_SCK_PIN			13




// определения для битбанда
#define IO_BB_ADDR(io_reg_addr,bit_number) ((uint32_t*)(PERIPH_BB_BASE + (((uint32_t)io_reg_addr - PERIPH_BASE) << 5) + (bit_number << 2)))

#define PCD8544_RST_BB_ADDR  IO_BB_ADDR(&PCD8544_GPIO->ODR, PCD8544_RST_PIN)
#define PCD8544_A0_BB_ADDR   IO_BB_ADDR(&PCD8544_GPIO->ODR, PCD8544_A0_PIN)
#define PCD8544_CSE_BB_ADDR  IO_BB_ADDR(&PCD8544_GPIO->ODR, PCD8544_CSE_PIN)
#define PCD8544_SDA_BB_ADDR  IO_BB_ADDR(&PCD8544_GPIO->ODR, PCD8544_SDA_PIN)
#define PCD8544_SCK_BB_ADDR  IO_BB_ADDR(&PCD8544_GPIO->ODR, PCD8544_SCK_PIN)

// Управление линией PCD8544_RST
#define PCD8544_RST1  *PCD8544_RST_BB_ADDR=0x00000001
#define PCD8544_RST0  *PCD8544_RST_BB_ADDR=0x00000000
// Управление линией PCD8544_DC
#define PCD8544_DC1   *PCD8544_A0_BB_ADDR=0x00000001
#define PCD8544_DC0   *PCD8544_A0_BB_ADDR=0x00000000
// Управление линией PCD8544_CS
#define PCD8544_CS1   *PCD8544_CSE_BB_ADDR=0x00000001
#define PCD8544_CS0   *PCD8544_CSE_BB_ADDR=0x00000000

#ifndef PCD8544_SPI_HARD
// Управление линией PCD8544_SCK
#define PCD8544_SCK1  *PCD8544_SCK_BB_ADDR=0x00000001
#define PCD8544_SCK0  *PCD8544_SCK_BB_ADDR=0x00000000
// Управление линией PCD8544_MOSI
#define PCD8544_MOSI1 *PCD8544_SDA_BB_ADDR=0x00000001
#define PCD8544_MOSI0 *PCD8544_SDA_BB_ADDR=0x00000000
#endif



void PCD8544_Init();
void PCD8544_data(uint8_t);
void PCD8544_Refresh();
void PCD8544_Clear();

void PCD8544_Pixel(uint8_t, uint8_t, uint8_t);
void PCD8544_Line(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void PCD8544_Rectangle(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

void PCD8544_Char(uint8_t, uint8_t, uint8_t, uint8_t);
void PCD8544_String(uint8_t, uint8_t y, char*, uint8_t);
void PCD8544_Int(uint8_t, uint8_t, int, uint8_t);
void PCD8544_IntWString(uint8_t, uint8_t, int, char*, uint8_t);


#endif

