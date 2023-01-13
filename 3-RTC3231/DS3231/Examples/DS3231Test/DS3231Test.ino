// ---- DS3231_test1.pde ----

/*-------- The following description edited by DIY_LAB ----------
//•DS3231介紹:
   DS3231是具有溫度補償之高精度即時時鐘(RTC)，同時內建一個精確的數位溫度感測器，
   可利用I2C介面與Arduino等單片機通信。
   本模組包含電池輸入端，即使電源關閉時仍可保持精確的計時。
   RTC保存秒、分、時、星期、日期、月和年資訊及閏年的修正。
   時鐘的工作格式可以是24小時或帶/AM/PM指示的12小時格式。
   提供兩個可設置的日曆鬧鐘和一個可設置的方波輸出。
   位址與資料通過I2C雙向匯流排串列傳輸。
   
//•實驗目的:
   利用 DS3231高精度即時鐘(RTC)模組 來讀出當時之年月日溫度等資料.
   格式如右: 2016-11-07 19:1:50	Temperature=34
  
//•所需模組: DS3231(RTC)模組 & Arduino模板
   Arduino請下載 DS3231_test1.pde 範例程式,編譯完後,按upload進行上傳,
   按serial monitor進行觀察時，monitor右下角要設成9600baud,否則可能會出現亂碼.

//•模組引腳連接如下：
      【DS3231】        與      【Arduino】
	  GND          <-->         GND
 	  Vcc          <-->         +5V
 	  SDA          <-->         A4
 	  SCL          <-->         A5   
----------------------------------------------------------------*/


#include <Wire.h>
#include <DS3231.h>

DS3231 Clock;
bool Century=false;
bool h12;
bool PM;
byte ADay, AHour, AMinute, ASecond, ABits;
bool ADy, A12h, Apm;

void setup() {

    // 啟動I2C介面
	Wire.begin();
	// 初始設定(將預設之年月日資料利用I2C介面傳給EEPROM AT24C32)
	// 以下設定一開始時間為:2014-08-31 19:01:50
    Clock.setSecond(50);//設定秒 
    Clock.setMinute(01);//設定分 
    Clock.setHour(19);  //設定時 
    //Clock.setDoW(1);    //設定星期幾(一週第幾天)
    Clock.setDate(7);  //設定日期(每月第幾天)
    Clock.setMonth(11);  //設定月
    Clock.setYear(16);  //設定年(西元後2位數)

	Serial.begin(9600);
	Serial.println("-- DS3231_test ---");
}


void loop()
{
    Read_Show_DS3231data(); //讀取並秀出年月日溫度等資料
	delay(1000);            //delay 1秒
}


//========= 以下副程式 ==========
void Read_Show_DS3231data()
{
  int second,minute,hour,date,month,year,temperature; 
  // 讀取資料
  second=Clock.getSecond();     //讀取秒
  minute=Clock.getMinute();     //讀取分
  hour=Clock.getHour(h12, PM);  //讀取時
  date=Clock.getDate();         //讀取日
  month=Clock.getMonth(Century);//讀取月
  year=Clock.getYear();         //讀取年
  temperature=Clock.getTemperature();//讀取溫度
  
  // 印出資料
  Serial.print("20");       //印出20xx年之20
  Serial.print(year,DEC);   //印出年
  Serial.print('-');
  Serial.print(month,DEC);  //印出月
  Serial.print('-');
  Serial.print(date,DEC);   //印出日
  Serial.print(' ');
  Serial.print(hour,DEC);   //印出時
  Serial.print(':');
  Serial.print(minute,DEC); //印出分
  Serial.print(':');
  Serial.print(second,DEC); //印出秒
  
  Serial.print('\t');       //空格
  Serial.print("Temperature=");
  Serial.print(temperature);//印出溫度
  Serial.print('\n');
}
