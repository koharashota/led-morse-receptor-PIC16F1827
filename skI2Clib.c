/*******************************************************************************
*  skI2Clib - �h�Q�b�֐����C�u����                                             *
*             ���̃��C�u������I2C�f�o�C�X(RTC/EEPROM��)�Ɛڑ����s���ׂ̊֐��W  *
*                                                                              *
*    InterI2C       - �h�Q�b�֘A�̊��荞�ݏ���                                 *
*    InitI2C_Master - �h�Q�b�ʐM�̃}�X�^�[���[�h�ŏ��������s������             *
*    I2C_Start      - �X���[�u�ɃX�^�[�g�R���f�B�V�����𔭍s���鏈��           *
*    I2C_rStart     - �X���[�u�Ƀ��s�[�g�E�X�^�[�g�R���f�B�V�����𔭍s���鏈�� *
*    I2C_Stop       - �X���[�u�ɃX�g�b�v�R���f�B�V�����𔭍s���鏈��           *
*    I2C_Send       - �X���[�u�Ƀf�[�^���P�o�C�g���M���鏈��                   *
*    I2C_Receive    - �X���[�u�����f�[�^���P�o�C�g���M���鏈��                 *
*                                                                              *
*    �����FSDA/SCL�s���͕K���u�f�W�^�����̓s���v�ɐݒ����s���ĉ������B         *
*          �ʐM�N���b�N��100/400KHz(CPU8MHz)�ł̏������ł��B                   *
*          �}���`�}�X�^�[���Ƀo�X�Փ˂����������ꍇ�́A"I2C_Start"�����đ����� *
*          SSP2�𗘗p�����ꍇ�́AskI2Clib.h��"#define I2C_MSSP2_USE"���錾���� *
* ============================================================================ *
*  VERSION DATE        BY                    CHANGE/COMMENT                    *
* ---------------------------------------------------------------------------- *
*  1.00    2012-01-20  ���ޒ��H�[(���ނ���)  Create                            *
*  1.01    2013-02-16  ���ޒ��H�[(���ނ���)  XC8 C Compiler �Ή��ɕύX         *
*  2.00    2014-11-01  ���ޒ��H�[(���ނ���)  �}���`�}�X�^�[�ɑΉ�              *
*  2.10    2015-03-04  ���ޒ��H�[(���ނ���)  ��������100/400KHz�w���ɕύX    *
*  3.00    2015-04-20  ���ޒ��H�[(���ނ���)  SSP1/SSP2������16F193x�ɑΉ�      *
* ============================================================================ *
*  PIC 12F1822 16F18xx 16F193x 18F25K22                                        *
*  MPLAB IDE(V8.63) MPLAB X(v2.15)                                             *
*  MPLAB(R) XC8 C Compiler Version 1.00/1.32                                   *
*******************************************************************************/
#include <xc.h>
#include "skI2Clib.h"

int AckCheck ;           // ���肩����ACK�҂��p�t���O�ϐ�
int CollisionCheck ;     // ���̃}�X�^�[�Ƃ̃o�X�Փˌ��o�p�t���O�ϐ�

// �A�C�h�����Ԃ̃`�F�b�N
// ACKEN RCEN PEN RSEN SEN R/W BF ���S�ĂO�Ȃ��n�j
void I2C_IdleCheck(char mask)
{
     while (( I2C_SSPCON2 & 0x1F ) | (I2C_SSPSTAT & mask)) ;
}
/*******************************************************************************
*  InterI2C( void )                                                            *
*    �h�Q�b�֘A�̊��荞�ݏ���                                                  *
*     ���̊֐��̓��C���v���O�����̊����݊֐��ŌĂт܂�                         *
*******************************************************************************/
void InterI2C( void )
{
     if (I2C_SSPIF == 1) {       // SSP(I2C)���荞�ݔ������H
          if (AckCheck == 1) AckCheck = 0 ;
          I2C_SSPIF = 0 ;        // �t���O�N���A
     }
     if (I2C_BCLIF == 1) {       // MSSP(I2C)�o�X�Փˊ��荞�ݔ������H
          CollisionCheck = 1 ;
          I2C_BCLIF = 0 ;        // �t���O�N���A
     }
}
/*******************************************************************************
*  InitI2C_Master(speed)                                                       *
*    �h�Q�b�ʐM�̃}�X�^�[���[�h�ŏ��������s������                              *
*                                                                              *
*    speed : I2C�̒ʐM���x���w��(0=100KHz 1=400KHz)                            *
*                                                                              *
*    ��)�N���b�N8MHz�ł̐ݒ��ł��A���̃N���b�N��SSPADD���ύX�����K�v���L���܂� *
*             4MHz  8MHz  16MHz  32MHz  40MHz  48MHz                           *
*    100KHz   0x09  0x13   0x27   0x4F   0x63   0x77                           *
*    400kHz         0x04   0x09   0x13   0x18   0x1D                           *
*******************************************************************************/
void InitI2C_Master(int speed)
{
     I2C_SSPSTAT= 0b10000000 ;     // �W�����x���[�h�ɐݒ肷��(100kHz/1MHz)
     I2C_SSPCON1= 0b00101000 ;     // SDA/SCL�s����I2C�Ŏg�p���A�}�X�^�[���[�h�Ƃ���
     if (speed == 0) {
          I2C_SSPADD = 0x13       ;   // �N���b�N=FOSC/((SSPADD + 1)*4) 8MHz/((0x13+1)*4)=0.1(100KHz)
     } else {
          I2C_SSPADD = 0x04       ;   // �N���b�N=FOSC/((SSPADD + 1)*4) 8MHz/((0x04+1)*4)=0.4(400KHz)
          I2C_SSPSTAT_SMP = 0 ;       // �������x���[�h�ɐݒ肷��(400kHz)
     }
     I2C_SSPIE = 1 ;               // SSP(I2C)���荞�݂�������
     I2C_BCLIE = 1 ;               // MSSP(I2C)�o�X�Փˊ��荞�݂�������
     PEIE      = 1 ;               // ���ӑ��u���荞�݂�������
     GIE       = 1 ;               // �S���荞�ݏ�����������
     I2C_SSPIF = 0 ;               // SSP(I2C)���荞�݃t���O���N���A����
     I2C_BCLIF = 0 ;               // MSSP(I2C)�o�X�Փˊ��荞�݃t���O���N���A����
}
/*******************************************************************************
*  ans = I2C_Start(adrs,rw)                                                    *
*    �X���[�u�ɃX�^�[�g�R���f�B�V�����𔭍s���鏈��                            *
*                                                                              *
*    adrs : �X���[�u�̃A�h���X���w�肵�܂�                                     *
*    rw   : �X���[�u�ɑ΂��铮���̎w�������܂�                                 *
*           0=�X���[�u�ɏ����݂Ȃ����v���@1=�X���[�u�ɑ��M���Ȃ����v��         *
*    ans  : 0=����                                                             *
*           1=�ُ�(���肩��ACK���Ԃ��Ă��Ȃ�) -1=���̃}�X�^�[�Ƃ̃o�X�Փ˔���  *
*******************************************************************************/
int I2C_Start(int adrs,int rw)
{
     CollisionCheck = 0 ;
     // �X�^�[�g(START CONDITION)
     I2C_IdleCheck(0x5) ;
     I2C_SSPCON2_SEN = 1 ;
     // [�X���[�u�̃A�h���X]�𑗐M����
     I2C_IdleCheck(0x5) ;
     if (CollisionCheck == 1) return -1 ;
     AckCheck = 1 ;
     I2C_SSPBUF = (char)((adrs<<1)+rw) ;     // �A�h���X + R/W�𑗐M
     while (AckCheck) ;                      // ���肩����ACK�ԓ����҂�
     if (CollisionCheck == 1) return -1 ;
     return I2C_SSPCON2_ACKSTAT ;
}
/*******************************************************************************
*  ans = I2C_rStart(adrs,rw)                                                   *
*    �X���[�u�Ƀ��s�[�g�E�X�^�[�g�R���f�B�V�����𔭍s���鏈��                  *
*                                                                              *
*    adrs : �X���[�u�̃A�h���X���w�肵�܂�                                     *
*    rw   : �X���[�u�ɑ΂��铮���̎w�������܂�                                 *
*           0=�X���[�u�ɏ����݂Ȃ����v���@1=�X���[�u�ɑ��M���Ȃ����v��         *
*    ans  : 0=����                                                             *
*           1=�ُ�(���肩��ACK���Ԃ��Ă��Ȃ�) -1=���̃}�X�^�[�Ƃ̃o�X�Փ˔���  *
*******************************************************************************/
int I2C_rStart(int adrs,int rw)
{
     CollisionCheck = 0 ;
     // ���s�[�g�E�X�^�[�g(REPEATED START CONDITION)
     I2C_IdleCheck(0x5) ;
     I2C_SSPCON2_RSEN = 1 ;
     // [�X���[�u�̃A�h���X]�𑗐M����
     I2C_IdleCheck(0x5) ;
     if (CollisionCheck == 1) return -1 ;
     AckCheck = 1 ;
     I2C_SSPBUF = (char)((adrs<<1)+rw) ;     // �A�h���X + R/W�𑗐M
     while (AckCheck) ;                      // ���肩����ACK�ԓ����҂�
     if (CollisionCheck == 1) return -1 ;
     return I2C_SSPCON2_ACKSTAT ;
}
/*******************************************************************************
*  ans = I2C_Stop()                                                            *
*    �X���[�u�ɃX�g�b�v�R���f�B�V�����𔭍s���鏈��                            *
*    ans  :  0=����                                                            *
*           -1=���̃}�X�^�[�Ƃ̃o�X�Փ˔���                                    *
*******************************************************************************/
int I2C_Stop()
{
     CollisionCheck = 0 ;
     // �X�g�b�v(STOP CONDITION)
     I2C_IdleCheck(0x5) ;
     I2C_SSPCON2_PEN = 1 ;
     if (CollisionCheck == 1) return -1 ;
     else                     return  0 ;
}
/*******************************************************************************
*  ans = I2C_Send(dt)                                                          *
*    �X���[�u�Ƀf�[�^���P�o�C�g���M���鏈��                                    *
*                                                                              *
*    dt  : ���M�����f�[�^���w�肵�܂�                                          *
*    ans  : 0=����                                                             *
*           1=�ُ�(���肩��ACK���Ԃ��Ă��Ȃ�����NOACK���Ԃ���)                 *
*          -1=���̃}�X�^�[�Ƃ̃o�X�Փ˔���                                     *
*******************************************************************************/
int I2C_Send(char dt)
{
     CollisionCheck = 0 ;
     I2C_IdleCheck(0x5) ;
     if (CollisionCheck == 1) return -1 ;
     AckCheck = 1 ;
     I2C_SSPBUF = dt ;                  // �f�[�^�𑗐M
     while (AckCheck) ;                 // ���肩����ACK�ԓ����҂�
     if (CollisionCheck == 1) return -1 ;
     return I2C_SSPCON2_ACKSTAT ;
}
/*******************************************************************************
*  ans = I2C_Receive(ack)                                                      *
*    �X���[�u�����f�[�^���P�o�C�g���M���鏈��                                  *
*                                                                              *
*    ack  : �X���[�u�ւ̕ԓ��f�[�^���w�肵�܂�                                 *
*           0:ACK���Ԃ��@1:NOACK���Ԃ�(���M�f�[�^���Ō��Ȃ�1)                  *
*    ans  : ���M�����f�[�^���Ԃ�                                               *
*           -1=���̃}�X�^�[�Ƃ̃o�X�Փ˔���                                    *
*******************************************************************************/
int I2C_Receive(int ack)
{
     int dt ;

     CollisionCheck = 0 ;
     I2C_IdleCheck(0x5) ;
     I2C_SSPCON2_RCEN = 1 ;           // ���M��������
     I2C_IdleCheck(0x4) ;
     if (CollisionCheck == 1) return -1 ;
     dt = I2C_SSPBUF ;                // �f�[�^�̎��M
     I2C_IdleCheck(0x5) ;
     if (CollisionCheck == 1) return -1 ;
     I2C_SSPCON2_ACKDT = ack ;        // ACK�f�[�^�̃Z�b�g
     I2C_SSPCON2_ACKEN = 1 ;          // ACK�f�[�^���Ԃ�
     return dt ;
}
