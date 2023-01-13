#include <IRremote.h>
#include <LedControl.h>
#include <DS3231.h>
#include <Wire.h>
// Init a Time-data structure
// 建立DS1302物件,並初始化
DS3231 rtc;
typedef struct time_s {
    byte hour;
    byte min;
    byte sec;
}Time;
Time currentTime,alarmTime;
bool h12,pm;
void displaytime(Time *t, int state);
void checkalarm(Time *t, Time *a);
/*
   Project1+Project2
   設定Arduino 的 IO腳位如下
 * IR Remote
   DIN      Pin 7
 */
int RECV_PIN = 7; // 
IRrecv irrecv(RECV_PIN);
decode_results results;
 
// 定義MAX7219顯示模組 I/O 怎麼接
#define MAX7219DataIn  10 //DIN
#define MAX7219LOAD  11   //CS
#define MAX7219CLK   12    //CLK
// 建立"LedControl"物件
// 以下 numDevices 是指要串接幾組 MAX7219顯示模組,目前只用1組做測試
LedControl lc=LedControl(MAX7219DataIn,MAX7219CLK,MAX7219LOAD,1);  //(DataIn, CLK1, LOAD, numDevices)
static unsigned long delaytime=500; //delay

/*◆ 接線圖：
//•模組引腳連接如下：
      【DS3231】        與      【Arduino】
	  GND          <-->         GND
 	  Vcc          <-->         +5V
 	  SDA          <-->         A4
 	  SCL          <-->         A5   

--------------------------------------------------------------------------*/
//定義 I/O Pin腳
#define RELAY_PIN  8 // high act
#define LED_PIN  13
/*********************** TIMER1 *************************************/
int timer1_counter;
volatile int timer1_end;
volatile  int timer1_count=0;  //0.5s unit
void timer1_setup(int timeend)
{
  timer1_count=0;
  timer1_end=timeend;
    // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1_counter to the correct value for our interrupt interval
  //timer1_counter = 64911;   // preload timer 65536-16MHz/256/100Hz
  //timer1_counter = 64286;   // preload timer 65536-16MHz/256/50Hz
  timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
  
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts}
}
ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  TCNT1 = timer1_counter;   // preload timer
  timer1_count++;
  if (timer1_count==timer1_end) {
 // digitalWrite(ledPin, digitalRead(ledPin) ^ 1);
    software_Reset();
     
  }
}
void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
    timer1_count=0;
    timer1_end=-1;
asm volatile ("  jmp 0");  
}  
void timer1_reset()
{
   timer1_count=0;
}
/*********************** TIMER1 END *************************************/


void setup()
{
  //定義 Out RELAY
  noInterrupts();           // disable all interrupts
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT); // Arduino Led pin 13
  digitalWrite(RELAY_PIN,  LOW );
  digitalWrite(LED_PIN,  LOW );
  //  
  irrecv.enableIRIn(); // Start the receiver
  // MAX7219啟動時預設為省電模式(shutdown mode),必須初始化
  // 設定正常運作模式
  lc.shutdown(0,false);   //模組0, true = shutdown mode ; false = normal operation
  // 設定亮度
  lc.setIntensity(0,8);   //模組0
  // 清除DSP
  lc.clearDisplay(0);     //模組0
  
  
     // 啟動I2C介面
    Wire.begin();
    rtc.setClockMode(false);     // 24 Hour Mode
 
  // 設定一開始時間
 // rtc.setDOW(SUNDAY);        // 設定星期幾(星期五)
 // rtc.setTime(23, 59, 55);   // 設定時間(23:59:55)(24小時制)讓您看到跨日變化
 // rtc.setDate(3, 1, 2016);   // 設定年月日(2015年1月2日); setDate(dd,mm,yy)
  alarmTime.hour=4;
  alarmTime.min=55;
  alarmTime.sec=0;
  timer1_setup(8*2); //開啟看門狗計時器功效，並設定重新開機 4sec
  
  Serial.begin(9600);
  Serial.println("v0703_nano ");        
  setcursor(7); printf("v703nano ");
  delay(1000);
}
// define Menu
typedef struct menuItems {
  int menuID;
  char *menuStr;
  char hotKey;
  void (*f)();
  int  siblings;
  int  sibling[3];
}MENUTYPE;
typedef MENUTYPE *pMENUTYPE;
static MENUTYPE menu[]={            
 {0x0000, "", '0', rootmenu,3,  {0x0001,0x0002,0x0003}},
 {0x0001, "1.Setup", '1', setuptime, 0, 0},
 {0x0002,"2.Alarm", '2', setuptime, 0, 0}, // setupAlarm
 {0x0003,"3.relay",  '3', checkalarmAction, 0, 0},
 {-1,"", '0', 0, 0, 0}
};
const int MENUCNT=sizeof(menu)/sizeof(MENUTYPE)-1;
#define preCur(x,y) (((x)-1+(y)) % (y))
#define nxtCur(x,y) (((x)+1) % (y))
static int currentMenuID = 0x0000; // current menuitem iD


// define Keycode
#define KEY_UP 'I'
#define KEY_DOWN 'M'
#define KEY_LEFT 'J'
#define KEY_RIGHT 'K'
#define KEY_ENTER 'X'


static int rootcursor=-1;
static int curMenuIndex=0;
static int rootstate = 0;
void loop() 
{
//       Serial.print(cur, DEC);
//       Serial.print(":");
       (*menu[hashindex(currentMenuID)].f)();
       timer1_reset();     //清掉看門狗計時器的計時值
       
       // check task function
}
int hashindex(int siblingidx) // supose to be hash , now is linear search
{
  int i;
  for(i=0;i<MENUCNT;i++) {
     if (siblingidx==menu[i].menuID) return i;
  }
  return 0; // if not found back to root
}

void returnroot()
{
  rootcursor=-1;
  currentMenuID=0;
  rootstate = 0;
      Serial.print("cursor"); Serial.print(rootcursor, DEC); 
       Serial.print("root"); Serial.print(rootstate, DEC); 
       Serial.print("ID"); Serial.print(currentMenuID, DEC); Serial.print("\n"); 
}
void rootmenu()
{
    int i;
    char c;
    switch(rootstate) {
      case 0: // initialize current time
         rootcursor=-1;
         curMenuIndex=0;
         currentTime.hour = rtc.getHour(h12,pm);
         currentTime.min = rtc.getMinute();
         currentTime.sec = rtc.getSecond();
         displaytime(&currentTime,rootstate);
         checkalarm(&currentTime, &alarmTime);
         break;
      case 1:  // alarm display state
         currentTime.hour = rtc.getHour(h12,pm);
         currentTime.min = rtc.getMinute();
         currentTime.sec = rtc.getSecond();
         displaytime(&alarmTime,rootstate);
         checkalarm(&currentTime, &alarmTime);
         break;
      case 2:  // menu state
        invaliddisplaytime();
         break;
      default:
        invaliddisplaytime();
          rootstate = 0;
          
    }
    c = getkey(&results,false);   
    if (c) {
      Serial.print(c); // dump c
      switch(c){
      case KEY_UP:
              if (rootcursor<0) rootcursor=1;
              rootcursor = preCur(rootcursor,menu[currentMenuID].siblings);
              curMenuIndex = hashindex(menu[currentMenuID].sibling[rootcursor]);
              Serial.println(menu[curMenuIndex].menuStr);
              printf("\n%s",menu[curMenuIndex].menuStr);
              rootstate = 2;
            break;
      case KEY_DOWN:
              rootcursor = nxtCur(rootcursor,menu[currentMenuID].siblings);
              curMenuIndex = hashindex(menu[currentMenuID].sibling[rootcursor]);
              Serial.println(menu[curMenuIndex].menuStr);
              printf("\n%s",menu[curMenuIndex].menuStr);
              rootstate = 2;
            break;
      case KEY_LEFT:
           rootstate--; rootstate=cyclecheck(rootstate,0,1);
           break;
      case KEY_RIGHT:
            rootstate++; rootstate=cyclecheck(rootstate,0,1);
           break;
      case KEY_ENTER:
            currentMenuID = menu[curMenuIndex].menuID;
            (*(menu[curMenuIndex].f))(); rootstate = 0;
            break;
      case '1': // hot key
      case '2': // hot key
      case '3': // hot key
        for(i=0;i< menu[rootcursor].siblings ;++i) { 
          int curMenuIndex = hashindex(menu[rootcursor].sibling[i]);
          if (menu[curMenuIndex].hotKey == c) {
               currentMenuID = menu[curMenuIndex].menuID;
               (*(menu[curMenuIndex].f))(); rootstate = 0;
          }
        }
        break;
      default: // other key escape menu
         rootstate = 0;
      }
    }
}
static Time curDispay;
static int curState=-1;
static int triggercnt=0;
void invaliddisplaytime()
{
  curState = -1;
  curDispay.hour=-1;
  curDispay.min=-1;
  curDispay.sec=-1;
}
void displaytime(Time *t, int state)
{
   if ((state!=curState)||(t->hour!=curDispay.hour)||(t->min!=curDispay.min)||(t->sec!=curDispay.sec)) {
        Serial.print("T ");
        Serial.print("A ");
        Serial.print(state,DEC);
        Serial.print(" ");
        Serial.print(t->hour,DEC);
        Serial.print(":");
        Serial.print(t->min,DEC);
        Serial.print(":");
        Serial.print(t->sec,DEC);
        Serial.print("\n");
    }
    if (state!=curState) {
        if (state==0) {
        setcursor(7); printf("%02d",triggercnt/2); //printf("T ");
        }else if (state==1){
        setcursor(7); printf("A ");
        }else{
        setcursor(7); printf("%02d",state);
        } 
        curState = state;
    }
    if (t->hour!=curDispay.hour) {
        setcursor(5); printf("%02d.",t->hour);
        curDispay.hour = t->hour;
    }
    if (t->min!=curDispay.min) {
        setcursor(3); printf("%02d.",t->min);
        curDispay.min = t->min;
    }
    if (t->sec!=curDispay.sec) {
        setcursor(1); printf("%02d",t->sec);
        curDispay.sec = t->sec;

    }
 
}
static int alarmState = -1;
void checkalarm(Time *t, Time *a)
{
    if ((t->hour== a->hour) && (t->min== a->min)&& (alarmState<0)) {
          printf("\nAlarm1!!");
          delay(1000);
          invaliddisplaytime(); 
          RelayActiion();
          alarmState=1;
    }else if ((t->hour== a->hour) && (t->min== a->min+2)&& (alarmState<0)) {
          printf("\nAlarm2!!");
          delay(1000);
          invaliddisplaytime(); 
          RelayActiion();
          alarmState=1;
    }else if ((t->hour!= a->hour) || ((t->min!= a->min)&&(t->min!= a->min+2))){
      alarmState = -1;
    }

    
}
void checkalarmAction()
{
    returnroot();
    checkalarm(&alarmTime, &alarmTime);
}
void RelayActiion()
{

        Serial.print("Relay action\n ");
        triggercnt++;
        lc.shutdown(0,true); // shutdown LED display
        delay(2000);
        digitalWrite(RELAY_PIN,  HIGH );
        delay(1000);
        digitalWrite(RELAY_PIN,  LOW );
        delay(2000);
        lc.shutdown(0,false); // shutdown LED display

        invaliddisplaytime();
}
// Menu 1.setupCurrentTime 

static int setupState=0;
char cyclecheck(char x, int mi, int mx)
{
  if (x>mx) x = mi;
  if (x<mi) x = mx; 
  return x;
}
int rangecheck(int x, int mi, int mx)
{
  if (x>mx) x = mx;
  if (x<mi) x = mi; 
  return x;
}
static int  keynumber =0;
void setuptime(){
    uint8_t *adjust;
    int rangeMin=0;
    int rangeMax=23;
    char c=0;
    Time *ptime;
    ptime = (currentMenuID == 0x0001) ? &currentTime:&alarmTime;
    switch (setupState) { // initialize _TODO get current time for setup
    case 0:
//      currentTime = rtc.getTime();
         currentTime.hour = rtc.getHour(h12,pm);
         currentTime.min = rtc.getMinute();
         currentTime.sec = rtc.getSecond();
      if  (currentMenuID == 0x0001) {
        ptime->hour=currentTime.hour;
        ptime->min=currentTime.min;
        ptime->sec=currentTime.sec;
      }
      displaytime(ptime,setupState);
      setupState = 1;
      keynumber = 0;
      Serial.println("InSetup"); // dump function name
      break;
    case 1:
      adjust =  &(ptime->hour); rangeMin=0; rangeMax=23;
    break;
    case 2: 
      *adjust= cyclecheck( *adjust, rangeMin,rangeMax);
      adjust = &(ptime->min);  rangeMin=0; rangeMax=59;
     break;
    case 3: 
      *adjust= cyclecheck( *adjust, rangeMin,rangeMax);
      adjust = &(ptime->sec);  rangeMin=0; rangeMax=59;
      break;
    case 4: 
      *adjust= cyclecheck( *adjust, rangeMin,rangeMax);
      if (currentMenuID == 0x0001) {
  //      rtc.setTime(currentTime.hour,currentTime.min, currentTime.sec);
        rtc.setSecond(currentTime.sec);
        rtc.setMinute(currentTime.min);
        rtc.setHour(currentTime.hour);
        
      }
    default: 
      Serial.println("XXXSetup"); // dump function name
        setupState = 0;   // TODO setup Timer
        currentMenuID = 0; // return to root   
      
      return;
    }
    if (setupState>0 ) {         // 解碼成功，收到一組紅外線訊號
      c = getkey(&results,false);   
      if (c) {
        Serial.print(c); // dump function name
         switch(c){
        case KEY_UP:
               (*adjust)++; 
               *adjust= cyclecheck( *adjust, rangeMin,rangeMax);
               break; 
        case KEY_LEFT:
               setupState--;
               setupState=rangecheck(setupState,1,3);
               break;
        case KEY_DOWN:
               (*adjust)--; 
               *adjust= cyclecheck( *adjust, rangeMin,rangeMax);
               break;
        case KEY_RIGHT:
               setupState++;
               setupState=rangecheck(setupState,1,3);
               break;
        case KEY_ENTER:
               setupState=4; // end 
               break;
        default:
             setupState=5;
           break;
        }
              displaytime(ptime,(setupState)+currentMenuID*10);

      //  delay(100);
      }
    }

}
void setupalarm()
{
  printf("\n2.InAlarm");
   Serial.println("InAlarm"); // dump function name
}
void displayTime(){
  printf("\n3.InTime");
   Serial.println("InTime"); // dump function name
}

//--------- 以下副程式 IR getKey---------
#define KEY_UP 'I'
#define KEY_DOWN 'M'
#define KEY_LEFT 'J'
#define KEY_RIGHT 'K'
#define KEY_ENTER 'X'
static struct ParserKeyTable {
   long irCode;
   char symChar;
} keys[]= {
       0xF50A4FB0, 'I',
       0xF50A26D9, 'K',
       0xF50AC639, 'J',
       0xF50ACF30, 'M',
       0xF50AF708, 'X',
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
char getkey(decode_results *results,boolean repFlag)
{
  int i;
  char c;
  int n=sizeof(keys)/sizeof(struct ParserKeyTable);
  if (irrecv.decode(results)) {         // 解碼成功，收到一組紅外線訊號
    c = parserkey(results);            // 顯示紅外線協定種類
    if (c) {
      if (repFlag || c!=preKey || (getElapse()>280)) {
        preKey=c; 
        beginElapse();
        return c;
      }else if (c==preKey) {
        beginElapse(); // clear repeat key 
      }
    }  
    irrecv.resume();                     // 繼續收下一組紅外線訊號        
  }
  return 0;
}

//--------- 以下副程式在7段顯示器 printf ---------

/*==============================
// 在7段顯示器 printf
// 用到函式 setChar(int addr=模組#, int digit=DSP#, char c, boolean dp); 
================================*/ 
static int pos = 7;
static int upos = 7;
static int uc = 0;
static boolean ub = 7;
static void printchare(char **str, int c, boolean b)
{
	
	if (str) {
		**str = c;
		++(*str);
	}
	else {
         if (c=='\n') {
            pos = 7;
            lc.clearDisplay(0); //模組#0 DSP off
         }else if  (c=='\r') {
            pos = 7;
         } else if (toupper(c) == '.' && !ub){
           lc.setChar(0,upos, uc,true); upos = pos;
         }else {
          lc.setChar(0,pos, toupper(c),b);
          upos = pos; uc = toupper(c); ub= b;
          pos= (pos-1+8) % 8;
         }
       }
  
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';
	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchare (out, padchar,false);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		 printchare (out, *string,false);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchare (out, padchar,false);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12
static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchare (out, '-',false);
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}

static int print(char **out, const char *format, va_list args )
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = (char *)va_arg( args, int );

				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		}
		else {
		out:
			printchare (out, *format,false);
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}
int setcursor(int position)
{
      pos = rangecheck(position,0,7);
      return pos;
}
int printf(const char *format, ...)
{
        va_list args;
        
        va_start( args, format );
        return print( 0, format, args );
}
