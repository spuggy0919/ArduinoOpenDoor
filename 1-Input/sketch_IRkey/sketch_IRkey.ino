#include <IRremote.h>
/*
   Project1
   設定Arduino 的 IO腳位如下
 * IR Remote
   DIN      Pin 7
 */
int RECV_PIN = 7;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("irCode: testing ");           

}

//--------- 以下副程式 IR getKey---------
struct ParserKeyTable {
   long irCode;
   char symChar;
} keys[]= {
       0xF50A4FB0, 'I', // 上
       0xF50A26D9, 'K', // 右
       0xF50AC639, 'J', // 左
       0xF50ACF30, 'M', // 下
       0xF50AF708, 'X', // OK
       0xF50A05FA, '0',
       0xF50A857A, '1',
       0xF50A45BA, '2',
       0xF50AC53A, '3',
       0xF50A25DA, '4',
       0xF50AA55A, '5',
       0xF50A659A, '6',
       0xF50AE51A, '7',
       0xF50A15EA, '8',
       0xF50A956A, '9'
}; 
static char preKey=0;
static int keyRepeat=0;
char parserkey(decode_results *results)
{
  int i;
  int n=sizeof(keys)/sizeof(struct ParserKeyTable);
  for(i=0;i<n;i++){
    if (results->value == keys[i].irCode) {
       return keys[i].symChar;
    }
  }
  return 0;
}
static unsigned long timeMillis,elapse;
#define labs(x) ((x>=0) ? x:-1L*x)
void  beginElapse()
{
  timeMillis = millis();
}
unsigned long  getElapse()
{
   return labs(millis() - timeMillis);
}
char getkey(decode_results *results)
{
  int i;
  char c;
  int n=sizeof(keys)/sizeof(struct ParserKeyTable);
  if (irrecv.decode(results)) {         // 解碼成功，收到一組紅外線訊號
#if 0 //  調時間用 
    Serial.print(results->value, HEX);
    Serial.print(" ");
    Serial.print(keyRepeat, DEC);
    Serial.print(" ");
    Serial.println(getElapse(), DEC);
#endif
    c = parserkey(results);            // 顯示紅外線協定種類
    irrecv.resume();                     // 繼續收下一組紅外線訊號        
    if (c) {
      if (c==preKey) keyRepeat++; //是否一直壓著同一按鍵
      if (getElapse()>200) { //是否釋放超過200ms
         keyRepeat=0; preKey=-1;
      }
      beginElapse(); //重置碼錶
      if (c!=preKey || keyRepeat>3) { //  換鍵或一直壓同一鍵重覆
        preKey=c; 
        return c;
      }
    }  
  }
  return 0;
}
void loop() 
{
  char c;
  if (irrecv.decode(&results)) {         // 解碼成功，收到一組紅外線訊號
    c = getkey(&results);            // 顯示紅外線協定種類
    if (c) {
      Serial.print(c);
    }
   }  
}

