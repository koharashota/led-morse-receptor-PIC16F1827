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
#define data_num 7 //受信データ量 7bit
#define data_period 598 //受信データの読み取り間隔 598μs default
#define morse_data_num 7 //モールス信号の受信データ量 7bit
static void pic_init(); // picの初期化情報を設定する
void signal_data_init(); // 受信データを格納する配列を初期化する
void morse_data_init(); // モールス信号受信データを格納する配列を初期化する
void detect_signal(int *detect_signal_result); // 受信データを感知してshotaが送られているかどうか判定してtrueなら1, falseなら0をdetect_signal_resultに代入する
void detect_state(int *detect_state_result, int *detect_state_result_count); // 発信状態(ツートン)を検知して押している状態なら1, 離している状態なら0, 終了状態なら2をdetect_state_resultに代入する
void detect_state_continue_pressed(int *detect_state_result, int *detect_state_result_count); // 押し続けているかどうかを検知
void detect_state_continue_left(int *detect_state_result, int *detect_state_result_count); // 離し続けているかどうかを検知
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

  while(1)
  {
    // 信号受信フラグを宣言
    int detect_signal_result = 0;
    // 信号を検知
    detect_signal(&detect_signal_result);

    // 信号を検知したかどうかで場合分け
    if ( detect_signal_result == 1 ){

      morse_data_init();
      int detect_state_result = 1;
      int detect_state_result_count = 0;

      LCD_SetCursor(0,0);
      LCD_Puts("input           ");

      // on, offなのでデータ量の倍のループ
      for (int i = 0; i < morse_data_num*2; i++) {
        if (detect_state_result_count < morse_data_num) {
          detect_state(&detect_state_result, &detect_state_result_count);
        }
      }
      LCD_SetCursor(0,0);
      for (int i = 0; i < morse_data_num; i++) {
        //LCD_Puts(morse_data[i]);
        if (morse_data[i]==0) {
          LCD_Puts("0");
        }else{
          LCD_Puts("1");
        }
      }
      LCD_Puts("      ");
      __delay_ms(3000);
    } else {
      LCD_SetCursor(0,0) ;
      LCD_Puts("restart       ") ;
      ledin = 0;
      ledout = 0;
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

void morse_data_init() {
  for (int i = 0; i < morse_data_num; i++ ) {
      morse_data[i] = 0;
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
    }
  } else {
    *detect_signal_result = 0;
  }
}

void detect_state(int *detect_state_result, int *detect_state_result_count){
  if(*detect_state_result == 0){
    // 離している状態なので、終了状態になっているかどうかを判定
    detect_state_continue_left(&*detect_state_result, &*detect_state_result_count);
  }else if(*detect_state_result == 1){
    // 押している状態なので、ツーかトンか判定
    detect_state_continue_pressed(&*detect_state_result, &*detect_state_result_count);
  }else{
    // 終了状態なのでなにもしない
  }
}

// 押し続けているかどうかを検知(ツーかトンか判定)
void detect_state_continue_pressed(int *detect_state_result, int *detect_state_result_count){

  int detect_state_continue_pressed_count = 0;

  // 1msに一度押しているかどうか判定して、押し続けている状態をカウント
  // 500ms秒間押している状態が検知できなかった場合終了
  // 以下で押し続けている状態のカウントが3以上であるかどうかで条件分岐
  //LCD_SetCursor(0,0);
  //LCD_Puts("on           ");

  // 600μs*830 = 0.5s
  for (int i = 0; i < 830; i++) {
    // 600μs
    __delay_us(data_period);
    // 押している状態なら、押し判定をカウント
    if( detector == 0 ){

      detect_state_continue_pressed_count++;
      i = 0;
    }
  }

  *detect_state_result = 0; // 離した状態に変更

  // 40msで一回送信されるので0.5s押しっぱなしとして10回受信
  if(detect_state_continue_pressed_count > 100){
    // 押し続けた際のデータを処理(ツーならば)
    morse_data[*detect_state_result_count]=1;
    //LCD_SetCursor(0,0) ;
    //LCD_Puts("set 1       ") ;
    //__delay_ms(1000);
  }else{
    // すぐに離した際のデータを処理(トンならば)
    morse_data[*detect_state_result_count]=0;
    //LCD_SetCursor(0,0) ;
    //LCD_Puts("set 0         ") ;
    //__delay_ms(1000);
  }
  *detect_state_result_count = *detect_state_result_count + 1; // カウントを1増加
}

// 離し続けているかどうかを検知
void detect_state_continue_left(int *detect_state_result, int *detect_state_result_count){

  int detect_state_continue_left_count = 0;

  // 1msに一度離しているかどうか判定して、離し続けている状態をカウント
  // 入力信号が検知された場合または、離し続けている状態のカウントが1000以上(1秒以上離されている状態)で判定終了
  // 以下で離し続けている状態のカウントが1000以上であるかどうかで条件分岐

  //LCD_SetCursor(0,0) ;
  //LCD_Puts("off       ") ;

  // 600μs*1660 = 1s
  for (int i = 0; i < 1660; i++) {
    // 600μs
    __delay_us(data_period);
    // 離している状態なら、離し判定をカウント
    if( detector != 0 ){
      detect_state_continue_left_count++;
      // 離し続けている状態のカウント数で場合分け
      if ( detect_state_continue_left_count > 3200) {
        i = 1660; // 終了
        *detect_state_result = 2; // 終了状態に変更
        LCD_SetCursor(0,0) ;
        LCD_Puts("end          ") ;
        __delay_ms(1000);
      }else{
        i = 0;
      }
    }else{
      i = 1660; // 終了
      *detect_state_result = 1; // 押した状態に変更
    }
  }
}
