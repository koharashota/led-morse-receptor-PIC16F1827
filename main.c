/*
 * File:   main.c
 * Author: shota
 *
 * Created on December 5, 2015, 1:50 PM
 */


// 'C' source lin e config statements

#include <xc.h>
#include <string.h>
#include "skI2Clib.h"
#include "skI2CLCDlib.h"

// コンフィギュレーション１の設定
#pragma config FOSC = INTOSC    // 内部ｸﾛｯｸを使用する(INTOSC)
#pragma config WDTE = OFF       // ｳｵｯﾁﾄﾞｯｸﾞﾀｲﾏｰ無し(OFF)
#pragma config PWRTE = ON       // 電源ONから64ms後にﾌﾟﾛｸﾞﾗﾑを開始する(ON)
#pragma config MCLRE = ON      // 外部ﾘｾｯﾄ信号は使用せずにﾃﾞｼﾞﾀﾙ入力(RA5)ﾋﾟﾝとする(OFF)
#pragma config CP = OFF         // ﾌﾟﾛｸﾞﾗﾑﾒﾓﾘｰを保護しない(OFF)
#pragma config CPD = OFF        // ﾃﾞｰﾀﾒﾓﾘｰを保護しない(OFF)
#pragma config BOREN = ON       // 電源電圧降下常時監視機能ON(ON)
#pragma config CLKOUTEN = OFF   // CLKOUTﾋﾟﾝをRA6ﾋﾟﾝで使用する(OFF)
#pragma config IESO = OFF       // 外部・内部ｸﾛｯｸの切替えでの起動はなし(OFF)
#pragma config FCMEN = OFF      // 外部ｸﾛｯｸ監視しない(FCMEN_OFF)

// コンフィギュレーション２の設定
#pragma config WRT = OFF        // Flashﾒﾓﾘｰを保護しない(OFF)
#pragma config PLLEN = OFF      // 動作クロックを32MHzでは動作させない(OFF)
#pragma config STVREN = ON      // スタックがオーバフローやアンダーフローしたらリセットをする(ON)
#pragma config BORV = HI        // 電源電圧降下常時監視電圧(2.5V)設定(HI)
#pragma config LVP = OFF        // 低電圧プログラミング機能使用しない(OFF)

#define detector RB5	//赤外線感知
#define led RB0	//テスト用led駆動
//#define buzzer RB1	//ブザー等高電圧用出力
//#define SDA RB4	//SDAに利用
//#define SCL RB1	//SCLに利用
#define _XTAL_FREQ 8000000

char heart[7] = {
  0b01010,
  0b11011,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
} ;

#define data_num 7 //受信データ量 7
#define data_period 598 //受信データの読み取り間隔 598μs default
static void pic_init();
void data_init();
int detect_signal(); // 感知できたなら1できなければ0
int data[data_num]; // 赤外線で受信したデータ
int shota_data[]={ 1, 1, 1, 0, 0, 1, 1}; //shota文字列の２進数

void interrupt InterFunction( void )
{
    InterI2C() ;
}

void main(void)
{
  pic_init();
  InitI2C_Master(0);
  LCD_Init(LCD_NOT_ICON,32,LCD_VDD3V,8);
  while(1)
  {
    // データの初期化
    data_init();
    // 初期化
    if ( detector == 0 ){
      // 最初の信号分の遅延
      __delay_us(data_period);
      __delay_us(data_period);
      // 受信データを読み取り
      for (int i = 0; i < data_num; i++ ) {
        if ( detector == 0 ){
          data[i] = 1;
          __delay_us(data_period);
        } else {
          data[i] = 0;
          __delay_us(data_period);
        }
      }
      // 受信データがshotaかどうか配列の中身を比較

      if ( memcmp(data, shota_data, sizeof(int) * data_num) == 0 ) {
        //buzzer=1;
        led = 1;
        __delay_ms(1000);
        led = 0;
      }else{
      }

      /**
      for (int i = 0; i < data_num; i++) {
        if (data[i] == 0) {
         led=1;
         __delay_ms(1000);
         led=0;
         __delay_ms(1000);
        } else {
          buzzer=1;
          __delay_ms(1000);
          buzzer=0;
          __delay_ms(1000);
        }
      }
      led = 0;
      buzzer = 0;
      __delay_ms(4000);
      **/

    } else {
      led = 0;
      //buzzer = 0;
      LCD_CreateChar(1,heart) ;
      LCD_SetCursor(0,0) ;
      LCD_Puts("Hellooo") ;
      LCD_Putc(0x01) ;
      LCD_SetCursor(2,1);
      LCD_Puts("PIC's");
    }
  }
}

static void pic_init() {
  TRISB = 0xF0;
  ANSELB= 0x00;	// set all RB to digital pin
  PORTB = 0x00;
  //OSCCON = 0x7A;	//内部クロック16 MHz 0111 1010b
  OSCCON     = 0b01110010; //内部クロック 8 MHz 0111 1010b
  OPTION_REG = 0b00000000;
  ANSELB     = 0b00000000;
  TRISB      = 0b00110010;
  WPUB       = 0b00010010;
}

void data_init() {
  for (int i = 0; i < data_num; i++ ) {
      data[i] = 0;
  }
}
