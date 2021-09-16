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
 *タッチセンサ機能確認テスト
 * 100msec間の発振周波数をTMR1でカウント
 * 100msecはTMR0を1msec割り込みで100カウントすることで実現
 */
#include "mcc_generated_files/mcc.h"

/*
                         Main application
 */
/*変数*/
static unsigned int delayCount;
/*************************************************************/
void delay_ms(unsigned int cnt){
    delayCount = cnt;
    
    while(delayCount);  //時間待ち
}
void TMR0_Interrupt(void){
    if (delayCount > 0)
        delayCount--;
}
void main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    
    TMR0_SetInterruptHandler(TMR0_Interrupt);

    unsigned int cap_value; // タッチセンサ値格納用
    unsigned int cap_threshold; // タッチ判定のしきい値格納用
    unsigned long cap_total; // キャリブレーション計算用

    unsigned char count; // for文カウントに使用

    bool sw_status = false; // スイッチ状態格納


    // タッチセンサ(Capacitive Sensor)の設定
    CPSCON0 = 0b00001000; // オシレータ(電流)は中に設定
    CPSCON1 = 0b00000000; // センサピンにCPS0を使用

    // タイマー１の設定
    T1CON = 0b11000001; // 容量検知オシレータでTIMER1をカウント、プリスケーラ1:1
    TMR1 = 0; // タイマー1の初期化

    // 割込み設定
    PEIE = 1; // 周辺装置割り込みを許可する
    GIE = 1; // 全割り込み処理を許可する 

    //
    // キャリブレーション
    //   最初に10回測定してスイッチタッチのしきい値を決める
    //

    // キャリブレーション中はLED点灯
    LATB0 = 1;

    // 10回測定 
    cap_total = 0;
    for (count = 0; count < 10; count++) {
        // 検知開始
        TMR1 = 0; // タイマー1の初期化
        CPSON = 1; // タッチセンサモジュール計測開始

        // 一定時間待つ
        delay_ms(100);      //100msec wait
        //__delay_ms(100);

        // 検知モジュールの値を読み込む
        CPSON = 0; // タッチセンサモジュール計測停止
        cap_total += TMR1; // カウント値を積算
    }

    // しきい値を10回の平均値の95%にする
    cap_threshold = cap_total / 10 * 0.95;

    // キャリブレーション終了後はLED消灯
    LATB0 = 0;

    //
    // タッチ検知処理
    //
    while (1) {
        // 検知開始
        TMR1 = 0; // タイマー1の初期化
        CPSON = 1; // タッチセンサモジュール計測開始

        // 一定時間待つ
        delay_ms(100);      //100msec wait
        //__delay_ms(100);

        // タッチセンサモジュールの計測を停止して、センサ値を一旦cap_valueに格納
        CPSON = 0; // タッチセンサモジュール計測停止
        cap_value = TMR1; // カウント値を読み込む

        // タッチ判定
        if (sw_status == false) {
            // 現在がOFFだったらON判定をする
            // ONの場合はLEDを点灯する
            if (cap_value < cap_threshold) {
                LATB0 = 1;
                sw_status = true;
            }
        } else {
            // 現在がONだったらOFF判定をする
            // OFFの場合はLEDを消灯する
            if (cap_value > cap_threshold) {
                LATB0 = 0;
                sw_status = false;
                /* cap_threshold更新 イニシャライズでの値と移動平均*/
                cap_threshold = (cap_value * 0.95 + cap_threshold) / 2;
            }
        }
    }
}
/**
 End of File
 */