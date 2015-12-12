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
#define ledout RA0	//テスト用led駆動
#define ledin RA1	//テスト用led駆動
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

#define data_num 7 //受信データ量 7bit
#define data_period 598 //受信データの読み取り間隔 598μs default
#define morse_data_num 7 //受信データ量 7bit
static void pic_init(); // picの初期化情報を設定する
void signal_data_init(); // 受信データを格納する配列を初期化する
void detect_signal(int *detect_signal_result); // 受信データを感知してshotaが送られているかどうか判定してtrueなら1, falseなら0をdetect_signal_resultに代入する
void detect_state(int *detect_state_result); // 発信状態(ツートン)を検知して押している状態なら1, 離している状態なら0をdetect_state_resultに代入する
void detect_state_continue_pressed(int *detect_state_result); // 押し続けているかどうかを検知
void detect_state_continue_left(int *detect_state_result); // 離し続けているかどうかを検知
int morse_data[morse_data_num];
int signal_data[data_num]; // 赤外線で受信したデータ
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
  LCD_CreateChar(1,heart) ;
  LCD_SetCursor(0,0) ;
  LCD_Puts("Hellooo") ;
  LCD_Putc(0x01) ;
  LCD_SetCursor(2,1);
  LCD_Puts("PIC's");
  while(1)
  {
    // 信号受信フラグを宣言
    int detect_signal_result = 0;
    // 信号を検知
    detect_signal(&detect_signal_result);

    // 信号を検知したかどうかで場合分け
    if ( detect_signal_result == 1 ){
      ledout = 0;
      ledin = 1;
      __delay_ms(1000);
    } else {
      ledin = 0;
      ledout = 1;
    }
  }
}

static void pic_init() {
  TRISB = 0xF0;
  TRISA = 0x00;
  ANSELB= 0x00;	// set all RB to digital pin
  PORTB = 0x00;
  PORTA = 0x00;
  //OSCCON = 0x7A;	//内部クロック16 MHz 0111 1010b
  OSCCON = 0x70; //内部クロック 8 MHz 0111 1010b
  OPTION_REG = 0b00000000;
  ANSELB     = 0b00000000;
  TRISB      = 0b00110010;
  WPUB       = 0b00010010;
}

void signal_data_init() {
  for (int i = 0; i < data_num; i++ ) {
      signal_data[i] = 0;
  }
}

void detect_signal(int *detect_signal_result) {
  // データの初期化
  signal_data_init();

  if ( detector == 0 ){

    // 最初の信号分の遅延
    __delay_us(data_period*3);

    // 受信データを読み取り
    for (int i = 0; i < data_num; i++ ) {
      if ( detector == 0 ){
        signal_data[i] = 1;
        __delay_us(data_period);
      } else {
        signal_data[i] = 0;
        __delay_us(data_period);
      }
    }

    // 受信データがshotaかどうか配列の中身を比較
    if ( memcmp(signal_data, shota_data, sizeof(int) * data_num) == 0 ) {
      *detect_signal_result = 1;

    }else{
      *detect_signal_result = 0;
      ledout = 1;
      __delay_ms(1000);
      ledout = 0;
    }
  } else {
    *detect_signal_result = 0;
  }
}

void detect_state(int *detect_state_result); // 発信状態(ツートン)を検知して押している状態なら1, 離している状態なら0をdetect_state_resultに代入する
void detect_state_continue_pressed(int *detect_state_result); // 押し続けているかどうかを検知
void detect_state_continue_left(int *detect_state_result); // 離し続けているかどうかを検知

void detect_state(int *detect_state_result){
  if(detect_state_result == 0){
    detect_state_continue_left(detect_state_result);
  }else{
    detect_state_continue_pressed(detect_state_result);
  }
}

// 押し続けているかどうかを検知(ツーかトンか判定)
void detect_state_continue_pressed(int *detect_state_result){
  int detect_state_continue_pressed_f = 0;
  // 信号受信フラグを宣言
  int detect_signal_result = 0;
  for (int i = 0; i < 10; i++) {
    // 信号を検知
    __delay_ms(200);
    detect_signal(detect_signal_result);
    if( detect_signal_result == 0 ){
      detect_signal_result++;
      i = 0;
    }
  }
  if(detect_signal_result > 3){
    // 押し続けた際のデータを処理
    detect_state(detect_state_result);
  }else{
    // すぐに離した際のデータを処理
  }
  detect_state_result = 0;

}

void detect_state_continue_left(int *detect_state_result){

}
