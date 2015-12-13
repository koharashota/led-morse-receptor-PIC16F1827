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

#define detector RB5	// 赤外線感知
#define ledout RA0	// テスト用led駆動
#define ledin RA1	// テスト用led駆動
#define _XTAL_FREQ 8000000
#define data_num 7 // 受信データ量 7bit
#define data_period 598 // 受信データの読み取り間隔 598μs default
#define morse_data_num 5 // モールス信号の受信データ量 5bit
#define morse_word_meaning_num 12 // モールス信号に対応する文字の登録文字限度数 8
#define morse_word_meaning_registration_num 10 // モールス信号に対応する文字の登録数
#define morse_char_meaning_registration_num 10 // モールス信号に対応する文字の登録数 26
#define morse_display_chars_num 14 // 液晶に表示する文字数

static void pic_init(); // picの初期化情報を設定する
void signal_data_init(); // 受信データを格納する配列を初期化する
void morse_data_init(); // モールス信号受信データを格納する配列を初期化する
void morse_display_chars_init(); // モールス信号を受けて表示する文字を格納する配列を初期化
void detect_signal(int *detect_signal_result); // 受信データを感知してshotaが送られているかどうか判定してtrueなら1, falseなら0をdetect_signal_resultに代入する
void detect_state(int *detect_state_result, int *detect_state_result_count); // 発信状態(ツートン)を検知して押している状態なら1, 離している状態なら0, 終了状態なら2をdetect_state_resultに代入する
void detect_state_continue_pressed(int *detect_state_result, int *detect_state_result_count); // 押し続けているかどうかを検知
void detect_state_continue_left(int *detect_state_result, int *detect_state_result_count); // 離し続けているかどうかを検知
void display_morse_input_num(); // 受信したモールス信号のツートンを表示
void display_morse_input(); // 受信したモールス信号を解析した情報を表示
void morse_correspond(); // モールス信号の対応

int morse_data[morse_data_num]; // 受信したモールス信号のツートンを格納する配列 空0, トン1, ツー2
int signal_data[data_num]; // 赤外線で受信した波形情報を2進数で格納する配列
int shota_data[]={ 1, 1, 1, 0, 0, 1, 1}; //shota文字列の2進数
/**
//モールス信号の対応表 アルファベット1文字
typedef struct {
    int morse_data[morse_data_num];
    char meaning;
} morse_correspondence_char_t;
morse_correspondence_char_t morse_correspondence_chars[morse_char_meaning_registration_num];
*/
char morse_display_chars[morse_display_chars_num]; // モールス信号を受けて表示する文字を格納する配列

//モールス信号の対応表 単文章
typedef struct {
    int morse_data[morse_data_num];
    char meaning[morse_word_meaning_num];
} morse_correspondence_word_t;
morse_correspondence_word_t morse_correspondence_words[morse_word_meaning_registration_num];

void interrupt InterFunction( void )
{
    InterI2C();
}

void main(void)
{
  pic_init();
  InitI2C_Master(0);
  LCD_Init(LCD_NOT_ICON,32,LCD_VDD3V,8);

  LCD_Clear();
  LCD_Puts("start") ;
  ledin = 0;
  ledout = 0;

  while(1)
  {
    // 信号受信フラグを宣言
    int detect_signal_result = 0;
    // 信号を検知
    detect_signal(&detect_signal_result);

    // 信号を検知したかどうかで場合分け
    if ( detect_signal_result == 1 ){

      morse_data_init();
      morse_display_chars_init();
      
      int detect_state_result = 1;
      int detect_state_result_count = 0;

      LCD_Clear();
      LCD_Puts("receiving...");

      // on, offなのでデータ量の倍のループ
      for (int i = 0; i < morse_data_num*2; i++) {
        if (detect_state_result_count < morse_data_num) {
          detect_state(&detect_state_result, &detect_state_result_count);
        }
      }

      LCD_Clear();
      LCD_Puts("end");

      display_morse_input_num();
      morse_correspond();
      display_morse_input();

    } else {
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

void morse_display_chars_init() {
  memset(morse_display_chars, '\0', strlen(morse_display_chars));
}

void display_morse_input_num(){
  LCD_Clear();
  LCD_Puts("morse code is");
  LCD_SetCursor(0, 1);
  for (int i = 0; i < morse_data_num; i++) {
    if (morse_data[i]==0) {
      LCD_Puts("0");
    }else{
      LCD_Puts("1");
    }
  }
  __delay_ms(2000);
}

void display_morse_input(){
  LCD_Clear();
  LCD_Puts("this means");
  LCD_SetCursor(0, 1);
  LCD_Puts(morse_display_chars);
  __delay_ms(3000);
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

  // 600μsに一度押しているかどうか判定して、押し続けている状態をカウント
  // 0.5s秒間押している状態が検知できなかった場合終了
  // 以下で押し続けている状態のカウントが100以上であるかどうかで条件分岐

  // 600μs*830 = 0.5s
  for (int i = 0; i < 830; i++) {
    // 600μs
    __delay_us(data_period);
    // 押している状態なら、押し判定をカウント
    if( detector == 0 ){
      detect_state_continue_pressed_count++;
      i = 0; // さらに0.5秒感知
    }
  }

  *detect_state_result = 0; // 離した状態に変更

  // 押し続けている状態のカウントが100以上かどうか判定
  if(detect_state_continue_pressed_count > 100){
    // 押し続けた際のデータを処理(ツーならば)
    morse_data[*detect_state_result_count]=2;
  }else{
    // すぐに離した際のデータを処理(トンならば)
    morse_data[*detect_state_result_count]=1;
  }
  *detect_state_result_count = *detect_state_result_count + 1; // カウントを1増加
}

// 離し続けているかどうかを検知
void detect_state_continue_left(int *detect_state_result, int *detect_state_result_count){

  int detect_state_continue_left_count = 0;

  // 600μsに一度離しているかどうか判定して、離し続けている状態をカウント
  // 入力信号が検知された場合または、離し続けている状態のカウントが3320以上(2秒以上離されている状態)で判定終了
  // 以下で離し続けている状態のカウントが3200以上であるかどうかで条件分岐

  // 600μs*3320 = 2s
  for (int i = 0; i < 3320; i++) {
    // 600μs
    __delay_us(data_period);
    // 離している状態なら、離し判定をカウント
    if( detector != 0 ){
      detect_state_continue_left_count++;
      // 離し続けている状態のカウント数で場合分け
      if ( detect_state_continue_left_count > 3320) {
        i = 3320; // 終了
        *detect_state_result = 2; // 終了状態に変更
      }else{
        i = 0;
      }
    // 押した状態を感知したらループ終了
    }else{
      i = 3320; // 終了
      *detect_state_result = 1; // 押した状態に変更
    }
  }
}

void morse_correspond(){
  /**
  for (int i = 0; i < morse_char_meaning_registration_num; i++) {
    // モールス信号からcharを推定
    if ( memcmp(morse_data, morse_correspondence_chars[i].morse_data, sizeof(int) * morse_data_num) == 0 ) {
    }
  }
  **/
  for (int i = 0; i < morse_word_meaning_registration_num; i++) {
    // モールス信号から文字を推定
    if ( memcmp(morse_data, morse_correspondence_words[i].morse_data, sizeof(int) * morse_data_num) == 0 ) {
      for (int j = 0; j < morse_word_meaning_num; j++) {
        // 表示用の配列に文字を代入
        morse_display_chars[j] = morse_correspondence_words[i].meaning[j];
      }
    }
  }
}


morse_correspondence_word_t morse_correspondence_words[morse_word_meaning_registration_num] = {
  {{1, 1, 1, 1, 1}, "help"},
  {{1, 1, 1, 1, 2}, "slack"},
  {{1, 1, 1, 2, 1}, "lunch"},
  {{1, 1, 1, 2, 2}, "dinner"},
  {{1, 1, 1, 2, 2}, "starbucks"}
};

/**
pic容量的に使えなかった
morse_correspondence_char_t morse_correspondence_chars[morse_char_meaning_registration_num] = {
  {{1, 2, 0, 0, 0}, 'a'},
  {{2, 1, 1, 1, 0}, 'b'},
  {{2, 1, 2, 1, 0}, 'c'},
  {{2, 1, 1, 0, 0}, 'd'},
  {{1, 0, 0, 0, 0}, 'e'},
  {{1, 1, 2, 1, 0}, 'f'},
  {{2, 2, 1, 0, 0}, 'g'},
  {{1, 1, 1, 1, 0}, 'h'},
  {{1, 1, 0, 0, 0}, 'i'},
  {{1, 2, 2, 2, 0}, 'j'},
  {{2, 1, 2, 0, 0}, 'k'},
  {{1, 2, 1, 1, 0}, 'l'},
  {{2, 2, 0, 0, 0}, 'm'},
  {{2, 1, 0, 0, 0}, 'n'},
  {{2, 2, 2, 0, 0}, 'o'},
  {{1, 2, 2, 1, 0}, 'p'},
  {{2, 2, 1, 2, 0}, 'q'},
  {{1, 2, 1, 0, 0}, 'r'},
  {{1, 1, 1, 0, 0}, 's'},
  {{2, 0, 0, 0, 0}, 't'},
  {{1, 1, 2, 0, 0}, 'u'},
  {{1, 1, 1, 2, 0}, 'v'},
  {{1, 2, 2, 0, 0}, 'w'},
  {{2, 1, 1, 2, 0}, 'x'},
  {{2, 1, 2, 2, 0}, 'y'},
  {{2, 2, 1, 1, 0}, 'z'}
};
**/
