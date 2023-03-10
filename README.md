# ArduinOpenDoor


這一系列主要用Arduino nano做一個自動鬧鐘開鐵捲門系統


# 自動鬧鐘鐵捲門開門系統

## 1-輸入篇
   如何使用紅外線遙控器當作鍵盤使用，這裡我用自己家裡的遙控器，你必須換成自己家用的，最好帶有數字鍵盤的紅外線遙控器。
![紅外線接收器](1-Input/irrecv.png)


## 2-輸出篇
   如何使用客製化的Printf  移植 C printf 函數
   利用移植 C printf 輸出到 MX7219 八位數碼管
![MAX7219接線](2-Output/max7219.png)

## 3-RTC3231
   本系統利用 RTC3231 做為時鐘
![RTC3231接線](3-RTC3231/rtccir.png)



## 4-繼電器控制
   控制繼電器((Relay) 我是用1-Channel Relay, High Active, 直接把Pin D8 接到繼電器的, 鐵門遙控器上的按鍵接點分別街繼電器的長開及Common Pin.

![繼電器控](4-RelayControl/Relay.png)
![繼電器控制捲門遙控器連接](4-RelayControl/PIC.JPG)
**注意**要把D8 直接接繼電器 IN, 可以忽略HCSR501 和 NPN 

## 5-Timer計時中斷篇
這個部分目前沒有說明，有一個Arduino nano完整範例，完成所有整合加上使用者介面的應用範例。 提供下列功能：

1. 設定時間
2. 設定鬧鐘時間
3. 直接控制Relay 啟動

原理就把前面學習的全部用上，也順便提供給各位自行發揮，整合1~4並寫一個由紅外線遙控器控制的UI流程 可以設定時間 設定鬧鐘的控制relay開関的應用。

![鬧鐘開鐵捲門](5-OpenDoor/circuit.png)



