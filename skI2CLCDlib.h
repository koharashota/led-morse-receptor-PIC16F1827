/*******************************************************************************
*  skI2CLCDlib.h - �h�Q�b�ڑ��k�b�c�֐����C�u�����p�C���N���[�h�t�@�C��        *
*             ST7032i�̃R���g���[��(8����x2�s�^16����x2�s)�^�C�v�Ȃ��n�j�ł��B *
*                                                                              *
* ============================================================================ *
*  VERSION  DATE        BY                    CHANGE/COMMENT                   *
* ---------------------------------------------------------------------------- *
*  1.00     2013-07-25  ���ޒ��H�[(���ނ���)  Create                           *
*  2.00     2015-03-01  ���ޒ��H�[(���ނ���)  ST7032i�R���g���[��LCD�ɑΉ�     *
*******************************************************************************/
#ifndef _SKI2CLCDLIB_H_
#define _SKI2CLCDLIB_H_

#ifndef _XTAL_FREQ
 // Unless already defined assume 16MHz system frequency
 // This definition is required to calibrate __delay_us() and __delay_ms()
 #define _XTAL_FREQ 8000000  // �g�p����PIC���ɂ��蓮�����g���l���ݒ肷��
#endif
#define ST7032_ADRES 0x3E     // I2C�ڑ����^LCD���W���[���̃A�h���X

/******************************************************************************/
/*  �萔�̒��`                                                                */
/******************************************************************************/
#define LCD_VDD3V                  1   // LCD�̓d����3.3V�Ŏg�p����(�������HON)
#define LCD_VDD5V                  0   // LCD�̓d����5.0V�Ŏg�p����(�������HOFF)
#define LCD_USE_ICON               1   // �A�C�R�����g��(�A�C�R���tLCD�̂�)
#define LCD_NOT_ICON               0   // �A�C�R���͎g���Ȃ�

/******************************************************************************/
/*  LCD�̃A�C�R���p�̃A�h���X���`(ST7032i�t���R���g���[��)                    */
/******************************************************************************/
#define LCD_ICON_ANTENNA           0x4010    // �A���e�i
#define LCD_ICON_PHONE             0x4210    // �d�b
#define LCD_ICON_COMMUNICATION     0x4410    // �ʐM
#define LCD_ICON_INCOMING          0x4610    // ���M
#define LCD_ICON_UP                0x4710    // ��
#define LCD_ICON_DOWN              0x4708    // ��
#define LCD_ICON_LOCK              0x4910    // ��
#define LCD_ICON_KINSHI            0x4B10    // �֎~(���M��OFF�H)
#define LCD_ICON_BATTERY_LOW       0x4D10    // �d�r�����Ȃ��[�d���悤��
#define LCD_ICON_BATTERY_HALF      0x4D08    // �d�r���܂��܂��g�����悧
#define LCD_ICON_BATTERY_FULL      0x4D04    // �d�r���܂��ς�
#define LCD_ICON_BATTERY_EMPTY     0x4D02    // �d�r�������ۂ�������
#define LCD_ICON_HANAMARU          0x4F10    // ���ς悭�o���܂���(���A�c�[�������H)

/******************************************************************************/
/*  �֐��̃v���g�^�C�v�錾                                                    */
/******************************************************************************/
void LCD_Clear(void) ;
void LCD_SetCursor(int col, int row) ;
void LCD_Putc(char c) ;
void LCD_Puts(const char * s) ;
void LCD_CreateChar(int p,char *dt) ;
void LCD_Init(int icon, int contrast, int bon, int colsu) ;
void LCD_Contrast(int contrast) ;
void LCD_IconClear(void) ;
void LCD_IconOnOff(int flag, unsigned int dt) ;
int  LCD_PageSet(int no) ;
void LCD_PageClear(void) ;
void LCD_PageSetCursor(int col, int row) ;
int  LCD_PageNowNo(void) ;


#endif
