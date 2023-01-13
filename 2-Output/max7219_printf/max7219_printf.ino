#include <LedControl.h>

// 範例:max7219_test1.ino (max7219模組顯示測試)

//•模組引腳連接如下：
  【max7219模組#1】    與      【Arduino】
 	   Vcc            <-->        +5V
 	   GND            <-->        GND	
 	   DIN            <-->        P10
 	   CS             <-->        P11   
 	   CLK            <-->        P12

----------------------------------------------------------------*/
 

#include "LedControl.h"

// 定義MAX7219顯示模組 I/O 怎麼接
#define DataIn  10 //DIN
#define LOAD  11   //CS
#define CLK  12    //CLK


// 建立"LedControl"物件
// 以下 numDevices 是指要串接幾組 MAX7219顯示模組,目前只用1組做測試
LedControl lc=LedControl(DataIn,CLK,LOAD,1);  //(DataIn, CLK1, LOAD, numDevices)
static unsigned long delaytime=500; //delay
void setup() 
{
  // MAX7219啟動時預設為省電模式(shutdown mode),必須初始化
  // 設定正常運作模式
  lc.shutdown(0,false);   //模組0, true = shutdown mode ; false = normal operation
  // 設定亮度
  lc.setIntensity(0,8);   //模組0
  // 清除DSP
  lc.clearDisplay(0);     //模組0
  
}


void loop() { 
  // write7Segments();
  writeTest();
}



//--------- 以下副程式 ---------

/*==============================
// 顯示"Arduino" & "diy_LAb"字樣
//==============================
// 用到函式 setChar(int addr=模組#, int digit, char value, boolean dp);

// 用到函式
setRow(int addr=模組#, int row, byte value)
value:二進制位元(B76543210)
bit0 => 7段顯示 G (中間橫線) ; 即value=B00000001 則7段顯示器之segment G會亮
bit1 => 7段顯示 F            ; 即value=B00000010 則7段顯示器之segment F會亮
bit2 => 7段顯示 E            ; 即value=B00000100 則7段顯示器之segment E會亮
bit3 => 7段顯示 D            ; 即value=B00001000 則7段顯示器之segment D會亮
bit4 => 7段顯示 C            ; 即value=B00010000 則7段顯示器之segment C會亮
bit5 => 7段顯示 B            ; 即value=B00100000 則7段顯示器之segment B會亮
bit6 => 7段顯示 A            ; 即value=B01000000 則7段顯示器之segment A會亮
bit7 => 7段顯示 DP(小數點)   ; 即value=B10000000 則7段顯示器之segment DP會亮
================================*/ 
void write7Segments() {

  // MAX7219模組#0 顯示"Arduino"
  lc.setChar(0,7,'a',false); //'A'
  delay(delaytime);
  lc.setRow(0,6,0x05);       //'r'
  delay(delaytime);
  lc.setChar(0,5,'d',false); //'d'
  delay(delaytime);
  lc.setRow(0,4,0x1c);       //'u'
  delay(delaytime);
  lc.setRow(0,3,B00010000);  //'i'
  delay(delaytime);
  lc.setRow(0,2,0x15);       //'n' 
  delay(delaytime);
  lc.setRow(0,1,0x1D);       //'o'
  delay(delaytime); 
  
  lc.clearDisplay(0);        //模組#0 DSP off
  delay(delaytime);  
 
 
  //MAX7219模組#0 顯示"diy_LAb"
  lc.setChar(0,7,'d',false); //'d'
  delay(delaytime);
  lc.setRow(0,6,B00010000);  //'i', segment C亮
  delay(delaytime);
  lc.setDigit(0,5,4,false);  //'y'; 4看起來像'y'
  delay(delaytime);
  lc.setRow(0,4,B00001000);  //'_'; segment D亮 
  delay(delaytime); 
  lc.setChar(0,3,'L',false); //'L'
  delay(delaytime);
  lc.setChar(0,2,'a',false); //'A'
  delay(delaytime);
  lc.setRow(0,1,B00011111);  //'b'
  delay(delaytime);
  
  lc.clearDisplay(0);        //模組#0 DSP off
  delay(delaytime);

} 



/*==============================
// 在7段顯示器顯示16進制數值
// 用到函式 setDigit(int addr=模組#, int digit=DSP#, byte value, boolean dp); 
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
               if (*(string+1) == '.')
		 printchare (out, *string,false);
               else {
		 printchare (out, *string,true); ++pc; string++;
               }
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

int printf(const char *format, ...)
{
        va_list args;
        
        va_start( args, format );
        return print( 0, format, args );
}
void writeTest() {
  int  i=123;
  int  f=123;
  printf("1.setup\r");
  delay(delaytime);
  printf("2.Alarm12\r");
  delay(delaytime);
  printf("3.Time\r");
  delay(delaytime);
  delay(delaytime);

}
