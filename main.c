/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC16F1827
        Driver Version    :  2.00
 */

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
 */
/*
 * タッチセンサ機能確認テスト
 * 100msec間の発振周波数をTMR1でカウント
 * 100msecはTMR0を1msec割り込みで100カウントすることで実現
 * 初期で10回のノーマル値を測定し平均値を記憶
 * スレッショルド値をノーマル値から減算
 * タッチセンサ検出でOFFなら、取得値とノーマル値との平均をとり新たなノーマル値とする
 */
#include "mcc_generated_files/mcc.h"

/*
                         Main application
 */
/** 変数 **/
static unsigned int delayCount; //遅延量カウンタ　msec指定
static unsigned int Norm; //タッチSW　OFF時の値格納変数
static bool touchSwStatus = false;  //タッチSWの状態フラグ

/** 定数 **/
#define TRIP    550     //ONOFF判定スレッショルド値　520～1000　小さいほど感度高い
#define HYST    64      //OFF時のヒステリシス値

/** 関数プロトタイプ **/
void delay_ms(unsigned int);
void TMR0_Interrupt(void);
unsigned int Get_Value(void);
void Touch_SW(void);

/**  関数　**/

/*******************************************************************************　
 *  遅延関数
 *  第1引数：　ディレイ値　msec
 *  
 ******************************************************************************/
void delay_ms(unsigned int cnt)
{
    delayCount = cnt;

    while (delayCount); //時間待ち
}

/*******************************************************************************
 *  TMR0割り込み関数
 *  第1引数：　なし
 *  1msec毎に処理され遅延時間のカウント
 *
 ******************************************************************************/
void TMR0_Interrupt(void)
{
    if (delayCount > 0)
        delayCount--;
}

/*******************************************************************************
 *  タッチスイッチの容量値測定関数
 *  第1引数：　なし
 *  TMR1のカウント値を取得して発振周波数を測定し容量値としてリターン
 *
 ******************************************************************************/
unsigned int Get_Value(void)
{
    TMR1 = 0; // タイマー1の初期化
    CPSON = 1; // タッチセンサモジュール計測開始

    /* 一定時間待つ */
    delay_ms(100); //100msec wait

    /*タッチセンサモジュールの計測を停止して、センサ値を格納*/
    CPSON = 0; // タッチセンサモジュール計測停止
    return (TMR1);
}

/*******************************************************************************
 *  タッチSWのON　OFF判定関数
 *  第1引数: なし
 *  フラグにてON OFFを指示、OFFの場合、Norm変数との平均値を取り、Normを補正する
 *  戻り値: 無し
 *
 ******************************************************************************/
void Touch_SW(void)
{
    unsigned int i, CapRead;
    CapRead = 0;

    for (i = 0; i < 2; i++) {
        CapRead += Get_Value() / 2; //i回の平均値とする　これでi*100msec掛かる 
    }

    /* タッチSWのON　OFF判定 */
    if (CapRead < (Norm - TRIP)) {//TRIPは初期定数として設定している
        //SW ON判定
        touchSwStatus = true;
    } else if (CapRead > (Norm - TRIP + HYST)){
        //SW OFF判定
        touchSwStatus = false;
        Norm = (Norm + CapRead) / 2; //OFF時の値とNormとの平均値で補正
       
    }
}

/*******************************************************************************
 *  main関数
 *  タッチセンサ初期化、OFF時の値を10回測定し平均値をNormに設定
 *  Normの初期値測定中はLED　ON
 *  100msec毎にタッチSWの値を取得する
 *  
 ******************************************************************************/
void main(void)
{
    int i;

    // initialize the device
    SYSTEM_Initialize();

    TMR0_SetInterruptHandler(TMR0_Interrupt);

    // タッチセンサ(Capacitive Sensor)の初期設定
    CPSCON0 = 0b00001000; // オシレータ(電流)は中に設定
    CPSCON1 = 0b00000000; // センサピンにCPS0を使用

    // タイマー１の初期設定
    T1CON = 0b11000001; // 容量検知オシレータでTIMER1をカウント、プリスケーラ1:1
    TMR1 = 0; // タイマー1の初期化

    // 割込み設定
    PEIE = 1; // 周辺装置割り込みを許可する
    GIE = 1; // 全割り込み処理を許可する 

    /* 容量性タッチセンサの初期キャリブレーション */
    //   最初に10回測定してOFF時のNorm値を測定する

    LATB0 = 1; // キャリブレーション中はLED点灯

    Norm = 0;
    for (i = 0; i < 10; i++) {
        Norm += Get_Value() / 10; //10回の平均値を取得する
    }

    LATB0 = 0; // キャリブレーション終了後はLED消灯

    /* 通常のタッチSW検出 */
    while (1) {
        // 検知開始
        Touch_SW();
        
        if (touchSwStatus == true) {//ONなら
            LATB0 = 1; //LED点灯
        } else {
            LATB0 = 0; //LED消灯
        }

    }
}
/**
 End of File
 */