#ifndef LCD_PCD8544
#define LCD_PCD8544

// ���� ��������� ����������� ���������������� �� �������� ���������� SPI
// ���� ����������������� - �� ���������� SPI2
// �� �������� � ���� ���������� ��� ���������� SPI � 5 ���� ������� �������� ����� (SPI2 = 18 ��, ������ SOFT-SPI=97 ��)
#define LCD_SPI_HARD

// ������������� DMA ��� ������ � ��������
//#define LCD_SPI_DMA_HARD

// ��� �������� �������� ��� ����������� SPI ��� ����� ���������� � ������ ������� ������ ���� ���������� � ������ ����� �����\������ !
// ���� �� ����� ����� ����������� SPI2 ��, �� ���� ���� ������ ���� ���������� ����� RST, A0, CSE
// � ��������� ����� (SDA, SCK) ������ ���� ���������� � GPIOB (���� 15 � 13 ��������������)
#define LCD_GPIO GPIOB
#define LCD_AHB1_GPIO RCC_AHB1Periph_GPIOB
// ���� ������� (����������)
#define LCD_RST_PIN 14
#define LCD_A0_PIN 12
#define LCD_CSE_PIN 10
//
// � ������ ���� ������������ SPI2 ����������� �� ����� SDA ������� ������ ���� ��������� � GPIOB_Pin_15, � SCK � GPIOB_Pin_13
// ��������� ����� ��� SPI2 ����������� ����� ���� ���������� � ������ ����� (�� ������ � GPIOB) ��. ����
// ���� ������� (������)

#define LCD_SDA_PIN 15
#define LCD_SCK_PIN 13

// ����������� ��� ��������
#define IO_BB_ADDR(io_reg_addr, bit_number) ((uint32_t *)(PERIPH_BB_BASE + (((uint32_t)io_reg_addr - PERIPH_BASE) << 5) + (bit_number << 2)))

#define LCD_RST_BB_ADDR IO_BB_ADDR(&LCD_GPIO->ODR, LCD_RST_PIN)
#define LCD_A0_BB_ADDR IO_BB_ADDR(&LCD_GPIO->ODR, LCD_A0_PIN)
#define LCD_CSE_BB_ADDR IO_BB_ADDR(&LCD_GPIO->ODR, LCD_CSE_PIN)
#define LCD_SDA_BB_ADDR IO_BB_ADDR(&LCD_GPIO->ODR, LCD_SDA_PIN)
#define LCD_SCK_BB_ADDR IO_BB_ADDR(&LCD_GPIO->ODR, LCD_SCK_PIN)

// ���������� ������ LCD_RST
#define LCD_RST1 *LCD_RST_BB_ADDR = 0x00000001
#define LCD_RST0 *LCD_RST_BB_ADDR = 0x00000000
// ���������� ������ LCD_DC
#define LCD_DC1 *LCD_A0_BB_ADDR = 0x00000001
#define LCD_DC0 *LCD_A0_BB_ADDR = 0x00000000
// ���������� ������ LCD_CS
#define LCD_CS1 *LCD_CSE_BB_ADDR = 0x00000001
#define LCD_CS0 *LCD_CSE_BB_ADDR = 0x00000000

#ifndef LCD_SPI_HARD
// ���������� ������ LCD_SCK
#define LCD_SCK1 *LCD_SCK_BB_ADDR = 0x00000001
#define LCD_SCK0 *LCD_SCK_BB_ADDR = 0x00000000
// ���������� ������ LCD_MOSI
#define LCD_MOSI1 *LCD_SDA_BB_ADDR = 0x00000001
#define LCD_MOSI0 *LCD_SDA_BB_ADDR = 0x00000000
#endif

void LCD_Init();
void LCD_data(uint8_t);
void LCD_Refresh();
void LCD_Clear();

void LCD_Pixel(uint8_t, uint8_t, uint8_t);
void LCD_Line(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void LCD_Rectangle(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

void LCD_Char(uint8_t, uint8_t, uint8_t, uint8_t);
void LCD_String(uint8_t, uint8_t y, char *, uint8_t);
void LCD_Int(uint8_t, uint8_t, int, uint8_t);
void LCD_IntWString(uint8_t, uint8_t, int, char *, uint8_t);

#endif
