#define VERSION "194mstn01"
// v194 - иногда на мстн (на 57600) на запрос получает ответ от предыдущего запроса
// в КАТОДЕ:
// переделать функционал, на частичное разгребание протокола - отображение номеров функций, количество регистров дикретных и аналоговых 
// выводить количество разных запросов, 
//          количество повторов запросов: количество запросов с единичным повторением (должно быть 0, иначе шум или битый пакет при старте), 
//                                        максимальное количество повторений запросов и количество таких запросов (количество - должно быть = макс), 
//                                        количество запросов с (1 < количеством повторений < МАКС) (много шумов) - должно быть 0, значит шумов нет
// в АНОДЕ:
// считать период запросов: задать 3-4 переменные под миллис, считать средние периоды запросов, округлить до целого, отсортировать, определить количество разных 
// задавать запросы по рассчитанным таймерам, распределяя внутри таймера равномерно запросы
// через 10 секунд или минимум 3 повторений всех запросов определить максимальный период повторений запросов, перейти в режим, при котором при наличии паузы в два максимальных режима автоматически перейти в режим WORK MODE
#define MASTER
//#define KATOD
//#define ANOD
//#define SLAVE
//#define SHOWOLED
#define ARDUINO
#define ADDRESS 0 //0 1 2 - адрес контроллера (SLAVE)
const unsigned long baud=57600; 	// минимум 1200, иначе таймеры сделать лонгами
const int TimerZaprosTime=3000;       // (мс) период запроса данных MASTERом
const int TimerZapros1Time=1000;      // (мс) максимальное время ожидания MASTERом ответа от SLAVEа на свой запрос  
const int timeNS=8;         // время ожидания перед отправкой ответа SLAVE (некоторым мастерам надо не меньше 8 мс для переключения входа на чтение)
const int timeNS_MASTER=60; // время ожидания перед отправкой следующего запроса MASTERом
const int timeEOM=(int)((unsigned long)(1000000)*(8+2+1)*3/2/baud); // микросекундный таймер, время ожидания конца посылки сделать 1,5 символа (3,5 символа - это минимальная обязательная пауза между посылками (19200 - 52мкс/бод, 520мкс/символ, 0.9мс на 1.5символа; 9600 - 104мкс/бод, 1040мкс/символ, 1.7мс на 1.5 символа)
const int timeEOMkatod=timeEOM*5; // микросекундный таймер для КАТОДа в режиме работы для MSTN
int t1s5, t3s5; // задаются в setup() в зависимости от скорости baud (при baud>19200 таймеры фиксированные 750 мкс и 1750 мкс)

#ifdef MSTN100
//  #include "Arduino.h"
  #include "mstn_uart.h"
//  #include "mstn_rtc.h"
//  #include "main.h"
//  #include <math.h>
//  #include <stdio.h>
//  #include <stdint.h>
  #define RXTX_PIN1 6
  #define RXTX_PIN2 8
#endif

#ifndef MSTN100
#include <GyverTimer.h>
GTimer TimerZapros(MS);               // создать миллисекундный таймер
GTimer TimerZapros1(MS);              // создать миллисекундный таймер
GTimer TimerEOM(US);               // таймер ожидания конца посылки/сообщения (1,5 символа)
#ifndef MASTER
GTimer TimerEOM2(US);               // для КАТОДА таймер ожидания конца посылки/сообщения от АНОДа (1,5 символа)
#endif
GTimer TimerNS(MS);               // таймер ожидания после получения запроса и отправкой ответа для SLAVE и для МАСТЕРа
#else
bool ZTTimeoutUS(unsigned long startTime, unsigned long period);
bool ZTTimeoutMS(unsigned long startTime, unsigned long period);
#endif

//############################################################################################################
//############################################################################################################
//#########################################  ТАЙМЕРЫ НА ЗАМЕНУ  ###############################################
//############################################################################################################
//############################################################################################################
#ifdef MSTN100
unsigned long TimerZapros1start; // переменная для запоминания времени старта
unsigned int TimerZapros1period; // переменная для запоминания времени старта
#endif
void TimerZapros1setTimeout(unsigned int period) {
  #ifdef MSTN100
    TimerZapros1start=millis(); TimerZapros1period=period;
  #else
    TimerZapros1.setTimeout(period);
  #endif
  };
bool TimerZapros1isReady(void) {
  #ifdef MSTN100
    return ZTTimeoutMS(TimerZapros1start,TimerZapros1period);
  #else
    return TimerZapros1.isReady();
  #endif
};

#ifdef MSTN100
unsigned long TimerZaprosstart; // переменная для запоминания времени старта
unsigned int TimerZaprosperiod; // переменная для запоминания времени старта
#endif
void TimerZaprossetTimeout(unsigned int period) {
  #ifdef MSTN100
    TimerZaprosstart=millis(); TimerZaprosperiod=period;
  #else
    TimerZapros.setTimeout(period);
  #endif
  };
bool TimerZaprosisReady(void) {
  #ifdef MSTN100
    return ZTTimeoutMS(TimerZaprosstart,TimerZaprosperiod);
  #else
    return TimerZapros.isReady();
  #endif
};

#ifdef MSTN100
unsigned long TimerNSstart;
unsigned int TimerNSperiod;
#endif
void TimerNSsetTimeout(unsigned int period) {
  #ifdef MSTN100
    TimerNSstart=millis(); TimerNSperiod=period;
  #else
    TimerNS.setTimeout(period);
  #endif
};
bool TimerNSisReady(void) {
  #ifdef MSTN100
    return ZTTimeoutMS(TimerNSstart,TimerNSperiod);
  #else
    return TimerNS.isReady();
  #endif
};

#ifdef MSTN100
unsigned long TimerEOMstart;
unsigned int TimerEOMperiod;
#endif
void TimerEOMsetTimeout(unsigned int period) {
  #ifdef MSTN100
    TimerEOMstart=micros(); TimerEOMperiod=period;
  #else
    TimerEOM.setTimeout(period);
  #endif
}; // перезапускаем таймер окончания посылки (3,5 символа по стандарту модбас)
bool TimerEOMisReady(void) {
  #ifdef MSTN100 
    return ZTTimeoutUS(TimerEOMstart,TimerEOMperiod);
  #else
    return TimerEOM.isReady();
  #endif
};

#ifndef MASTER
#ifdef MSTN100
unsigned long TimerEOM2start;
unsigned int TimerEOM2period;
#endif
void TimerEOM2setTimeout(unsigned int period) {
  #ifdef MSTN100
    TimerEOM2start=micros(); TimerEOM2period=period;
  #else
    TimerEOM2.setTimeout(period);
  #endif
}; // перезапускаем таймер окончания посылки (3,5 символа по стандарту модбас)
bool TimerEOM2isReady(void) {
  #ifdef MSTN100 
    return ZTTimeoutUS(TimerEOM2start,TimerEOM2period);
  #else
    return TimerEOM2.isReady();
  #endif
};
#endif // MASTER
//############################################################################################################
//#########################################  ТАЙМЕРЫ НА ЗАМЕНУ  ###############################################
//############################################################################################################

#define ser1pinRX 19
#define ser1pinTX 18
#define ser2pinRX 17
#define ser2pinTX 16
#ifndef __AVR_ATmega2560__
  #ifndef MSTN100
    #include <SoftwareSerial.h>
    #define ser1pinRX 6
    #define ser1pinTX 8     // рабочий пин для MASTER SLAVE
    #define ser2pinRX 7     // рабочий пин для MASTER SLAVE
    #define ser2pinTX 11
    SoftwareSerial Serial1( ser1pinRX ,  ser1pinTX ); // RX контроллера, TX контроллера
    SoftwareSerial Serial2( ser2pinRX ,  ser2pinTX ); // RX контроллера, TX контроллера
  #else
    #define ser1pinRX 0
    #define ser1pinTX 1
    #define ser2pinRX 11
    #define ser2pinTX 13
  #endif
#endif


//############################################################################################################
//############################################################################################################
//#########################################  SERIAL НА ЗАМЕНУ  ###############################################
//############################################################################################################
//############################################################################################################

void Serial1begin(unsigned long baud) 
 {
  #ifdef MSTN100
    UART_Begin(SERIAL1, baud);
  #else
    Serial1.begin(baud);
  #endif
 }
void Serial2begin(unsigned long baud) 
 {
  #ifdef MSTN100
    UART_Begin(SERIAL2, baud);
  #else
    Serial2.begin(baud);
  #endif
 }
int Serial1available() 
 {
  #ifdef MSTN100
    return UART_Available(SERIAL1);
  #else
    return Serial1.available();
  #endif
 }
int Serial2available() 
 {
  #ifdef MSTN100
    return UART_Available(SERIAL2);
  #else
    return Serial2.available();
  #endif
 }

int Serial1read()
{ 
  #ifdef MSTN100
    return UART_Read(SERIAL1);
  #else
    return Serial1.read();
  #endif
}
int Serial2read()
{ 
  #ifdef MSTN100
    return UART_Read(SERIAL2);
  #else
    return Serial2.read();
  #endif
}
void Serial1write(char c)
{ 
  #ifdef MSTN100
    UART_Write(SERIAL1,c);
  #else
    Serial1.write(c);
  #endif
}
void Serial2write(char c)
{ 
  #ifdef MSTN100
    UART_Write(SERIAL2,c);
  #else
    Serial2.write(c);
  #endif
}
void Serial1print(const char* s)
{ 
  #ifdef MSTN100
    UART_PrintStr(SERIAL1,s);
  #else
    Serial1.print(s);
  #endif
}
void Serial2print(const char* s)
{ 
  #ifdef MSTN100
    UART_PrintStr(SERIAL2,s);
  #else
    Serial2.print(s);
  #endif
}
//############################################################################################################
//#########################################  SERIAL НА ЗАМЕНУ  ###############################################
//############################################################################################################



#ifdef SHOWOLED
  #include "OLED_I2C.h"
  OLED myOLED(SDA, SCL, 8);
  //extern uint8_t RusFont[]; // Русский шрифт
    extern uint8_t SmallFont[];
          String strn1="";
          String strn2="";
          int x1=0,x2=0,y1=0,y2=0;
#endif

byte data=0;
#if defined(MASTER) || defined(SLAVE)
  String outString = ""; 
  const char *req="rq";   //строка запроса
  const char *otv="a";   //строка ответа
#endif

#define ButtonPin 12
#ifdef MSTN100
  #define ButtonPin BTN_BUTTON_PIN_NUM
#endif

int digitalReadButtonPin() {
  #ifdef MSTN100
    return !BTN_UserButtonRead();
  #else
    return digitalRead(ButtonPin);
  #endif
}

int meslen=0;
int meslen2=0;
const int maxadr=3; //количество адресов (устройств)
const int maxfun=3; // количество разных запросов
int x=0; // адрес устройства
int y=0; // номер функции
bool knopka=false;
bool knopka_prep=false;
int curaddr=0;
bool MODE=false; //false - режим обучения, true - режим диода
  byte status=0;
  byte status1=10;
  byte status2=20;
bool zaprosready=false;
bool inputopen=true;
bool otvetready=false;
bool newzapros=false;
bool zapros=false;
bool newotvet=false;
const int maxlenzap=10; // максимальная длина запроса в символах (1Б адрес + 1Б функция + 2Б адрес перв.регистра + 2Б адрес посл.регистра + 2Б контр.сумма = 8 Байт для функций чтения 0х01, 0х02, 0х03, 0х04)
const int maxlenotv=256; // длина ответа в символах (он же буфер для чтения порта)
char strzap[maxlenzap]="";
char char1, char2; // символы для чтения из компорта1 и компорта2 соответственно

const int maxzapros=15; // сколько максимально всего разных запросов может поступить, включая те, на которые нет ответов
char   zp[maxzapros][maxlenzap]; // массив для хранения строк запросов
unsigned char  zplen[maxzapros]; // массив для хранения длины каждого запроса
unsigned int zpcount[maxzapros]; // массив для хранения количества повторений запросов
//  int curnumzp=0;       // текущий номер запроса/ответа
int kolzp=0;          // сколько разных запросов, не должен превышать maxzapros!!!
int zpcountmax=0;     // подсчёт максимального количества повторений запросов
const int zpcountready=3; // количество повторений запросов, чтобы зажечь зел. СД
const int zpcountmin=2; // сколько раз должен быть получен запрос, чтобы стать легетимным и работать в АНОДе в рабочем режиме. 2-значит запрос получен 2 раза, то есть был 1 повтор.
bool flag1=false, flag2=false; // флаг, что считан байт из порта 1 или 2 соответственно. После использования значения флаг сбрасывается
int zapnum=0; // номер запроса в массиве


char str2[maxlenotv]="";// для считывания катодом ответа от анода в WORK (LEARN) режиме: посылает запрос и ответ подряд

bool Flag_Prestore_Otvet=false; // если флаг тру, то в режиме обучения ответы сохраняются, тогда при переключении в работу на запросы уже будут готовые ответы, иначе несколько запросов могут выпасть по таймауту, пока анод не соберёт/передаст ответы
const int   maxotvet=maxzapros; // максимальное количество ответов которое может быть от контроллера 3фун * 5 (по 100 регистров) = 15 ответов
char otvet[maxotvet][maxlenotv]; 
unsigned int  otvlen[maxotvet]; 
int zapnum2=0; // номер запроса в массиве
unsigned long timezp[maxzapros];   // время [мс] когда пришёл последний запрос
unsigned long periodzp[maxzapros]; // минимальный период запроса - определяются при обучении, используются в рабочем режиме

#define OVERFLOW_KOL_ZAPROS 1 //код ошибки переполнения количества разных запросов для массива хранения запросов
#define OVERFLOW_LONG_OTVET 2 //код ошибки переполнения длины ответа (посылки модбас)
#define OVERFLOW_LONG_ZAPROS 3 //код ошибки переполнения длины запроса
#define OVERFLOW_LONG_ZAPROS2 4 //код ошибки переполнения длины запроса
#define GREENLED 5 //код ошибки переполнения длины запроса
#define NO_ANSWER 6
#define UNKNOWN_OTVET 7
#define REDLED 8
#define NOT_RESET_READ1_FLAG 9
#define NOT_RESET_READ2_FLAG 10

void setup() { // ------------------------------------- setup() -------------------------------
  if (baud>19200) {t1s5=750; t3s5=1750;} // мкс по стандарту модбас для скоростей выше 19200 интервалы фиксированы
    else  { t1s5=(int)((unsigned long)(1000000)*(8+2+1)*3/2/baud);
	    t3s5=(int)((unsigned long)(1000000)*(8+2+1)*7/2/baud);
	  };


#ifdef MSTN100
  BTN_UserButtonInit();
  pinMode(LED_RED_PINNUM, OUTPUT);
  pinMode(LED_GRN_PINNUM, OUTPUT);
  #ifdef ANOD
    digitalWrite(LED_RED_PINNUM, HIGH);
    delay(2000);
    digitalWrite(LED_RED_PINNUM, LOW);
  #endif
  #ifdef KATOD
    digitalWrite(LED_GRN_PINNUM, HIGH);
    delay(2500);
    digitalWrite(LED_GRN_PINNUM, LOW);
  #endif
//int BTN_UserButtonRead( void );
#endif
  pinMode(LED_BUILTIN, OUTPUT); // зажигаем при переполнении массива запросов/ответов
  pinMode(ButtonPin, INPUT_PULLUP); //кнопка - смена адреса устройства / данных ответа
#ifndef MSTN100
  analogReference(EXTERNAL); // для схемы с экраном, чтоб не спалить AREF, удалить потом строку за ненадобностью
#endif
  Serial1begin(baud);
  Serial2begin(baud);
  #ifdef SHOWOLED
    myOLED.begin();
    myOLED.setFont(SmallFont);
    myOLED.invert(true);
    #ifdef MASTER
      myOLED.print("MASTER-SIRIUS v"+String(VERSION),10,1);  
      myOLED.print("Serial pins to KATOD:",1,27);
      myOLED.print("RX="+String(ser2pinRX)+", TX="+String(ser1pinTX),1,45);
    #endif
    #ifdef KATOD
      myOLED.print("DIOD-KATOD v"+String(VERSION),10,1);  
      myOLED.print("Serial_1 to MASTER:",1,24);
      myOLED.print("  pinRX="+String(ser1pinRX)+"  pinTX="+String(ser1pinTX),1,33);  
      myOLED.print("Serial_2 to ANOD:",1,46); 
      myOLED.print("  pinRX="+String(ser2pinRX)+"  pinTX="+String(ser2pinTX),1,56); 
    #endif
    #ifdef ANOD
      myOLED.print("DIOD-ANOD v"+String(VERSION),10,1);  
      myOLED.print("Serial_1 to KATOD:",1,24);
      myOLED.print("  pinRX="+String(ser1pinRX)+"  pinTX="+String(ser1pinTX),1,33);  
      myOLED.print("Serial_2 to SLAVE:",1,46); 
      myOLED.print("  pinRX="+String(ser2pinRX)+"  pinTX="+String(ser2pinTX),1,56); 
      #define ANOD1
    #endif
    #ifdef SLAVE
      #define SLAVE1
      myOLED.print("SLAVE-CONTROLLER v"+String(VERSION),1,1);  
      myOLED.print("Serial pins to ANOD: ",1,30);
      myOLED.print("RX="+String(ser2pinRX)+", TX="+String(ser1pinTX),1,45);  
    #endif
    myOLED.print(String(baud)+" baud",40,10);  
    myOLED.update();
    delay(500);
    myOLED.invert(false);
    myOLED.clrScr();
  #endif // showled
  TimerZaprossetTimeout(TimerZaprosTime); // интервал отправки запроса для MASTER

#if defined(MASTER) || defined(SLAVE)
  //outString.reserve(100);
#endif

} // ============================ end setup() ============================



void loop() { // ---------------------------------- loop() ----------------------------------------------


#if defined(KATOD) || defined(ANOD)

if (Serial1available())
{
  if (flag1) SHOWERROR(NOT_RESET_READ_FLAG); // если флаг не сброшен, то символ не обработан - алгоритмическая ошибка
  flag1=true;
  char1=(char)Serial1read();
}
if (Serial2available())
{
  if (flag2) SHOWERROR(NOT_RESET_READ_FLAG); // если флаг не сброшен, то символ не обработан - алгоритмическая ошибка
  flag2=true;
  char2=(char)Serial2read();
}

    if (!MODE) // надо параллельно принимать с двух портов и отвечать обратно мастеру 
    {// LEARN MODE одинаковый для КАТОД и АНОД, кроме рассчёта таймера АНОДа и возможно предварительного сохранения ответов КАТОДА
      if (flag1) Serial2write(char1);
      if (flag2) Serial1write(char2);
      if (status==0)
	{
	  if (flag1) 
	    {
		meslen=0;
		status=1;
	    }
 	  if (flag2) {SHOWERROR(ANOD_MUSOR); flag2=false;}			
	}
      if (status==1)
	{
 	  if (flag1) 
	    {
		if (meslen<maxlenzap) strzap[meslen++]=char1; else SHOWERROR(OVERFLOW_ZAPROS_LEN);
	  	Timer1s5Start();
	  	Timer3s5Start();
	  	Timer1secStart();
	  	flag1=false;
	    }
	  if (Timer1s5Ready())
	    {  //СОХРАНИТЬ ЗАПРОС
	 	zapnum=findzapros(strzap,meslen); // -1 запрос не найден 
	        if (zapnum<0) zapnum=savenewzapros(strzap,meslen); // -1 запрос не сохранён, массив кончился
		  else if ((++zpcount[zapnum])>zpcountready) SHOWERROR(KATOD_READY);
		if (zapnum<0)
	         {
	          SHOWERROR(OVERFLOW_KOL_ZAPROS);
	          status=0; // запрос игнорируется, ответ также будет игнорирован до следующего запроса
	         }
		  else 
			{
			  #ifdef ANOD
			    UpdateZaprosTimer(zapnum);
			  #endif
			  status=2;
			}

	    }
	  if (flag2) {SHOWERROR(ANOD_MUSOR); flag2=false;}
 	} //status==1
      if (status==2)
	{
	  if (Timer1secReady()) status=0; //!!!!!!!  при поступлении данных от МАСТЕРа за это время можно попытаться автоматически определить таймаут, например по первым 3-10 запросам, на которые например принципиально не отвечать
	  if (flag1) {SHOWERROR(MASTER_MUSOR); flag1=false;}
	  if (flag2) 
		{ if (Timer3s5Ready()) {meslen=0; status=3;}
		    else {SHOWERROR(ANOD_MUSOR); flag2=0;}
		}
	}
      if (status==3)
	{
	  if (flag2)
		{ // поступает ответ, сохранить, если надо
		  #ifdef KATOD
		  if (Flag_Prestore_Otvet)
 		    if (meslen<maxlenotv) str2[meslen++]=char2; else SHOWERROR(OVERFLOW_OTVET_LEN);
		  #endif
		  Timer1s5Start();
		  Timer3s5Start();
		  flag2=false;
		}
	  if (Timer1s5Ready())
		{ // ответ считан 
		  #ifdef KATOD
                  if (Flag_Prestore_Otvet) // если включен флаг сохранить ответ для безшовного переключения в режим работы, чтобы ответы не были пустыми, а выдавались последние сохранённые
		    {
			for (int j=0; ( (j<meslen) && (j<maxlenotv) ); j++) // копирование в массив считанной строки
                  	  otvet[zapnum][j]=str2[j];
                  	otvlen[zapnum]=meslen; // сохранение длины ответа для режима РАБОТА
		    }
		  #endif
		  status=4;
		}
	  if (flag1) {SHOWERROR(MASTER_MUSOR); flag1=false;
	}
      if (status==4)
	{
	  if (flag1) {SHOWERROR(MASTER_MUSOR); flag1=false;}
	  if (flag2) {SHOWERROR(ANOD_MUSOR); flag2=false;}
	  if (Timer3s5Ready()) status=0;	  
	}
      MODE=knopkamode();
      #ifdef SHOWOLED
	if (MODE) 
          { 
            myOLED.print(String(" WORK MODE"),60,35); 
            myOLED.update();
	  }
      #endif
    }//  LEARN MODE
    else // -------------------------------------------------------
#ifdef KATOD
    { // KATOD WORK MODE
      if (status1==10) // Status1=10 – ожидание следующего запроса от мастера
	{
	  if (flag1) 
		{
		  Timer1s5Start();
		  Timer3s5Start();
		  meslen=0;
		  status1=11;
		}
	}
      if (status1==11) // Status1=11 – Принимается запрос, ожидание окончание запроса (1,5 символа)
	{
	  if (flag1)
		{
		  if (meslen<maxlenzap) strzap[meslen++]=char1; else SHOWERROR(OVERFLOW_ZAPROS_LEN);
		  Timer1s5Start();
		  flag1=false;
		}
	  if (Timer1s5Ready())
		{ // НАЙТИ ЗАПРОС и ОПРЕДЕЛИТЬ НОМЕР ОТВЕТА zapnum
		  zapnum=findzapros(strzap,meslen); // -1 значит что запрос не найден

	          #ifdef SHOWOLED
	          // закоменчено, так как посылки криво в компорт улетает при выводе на экран
	          //inputStr[meslen]=0; // 
	          //myOLED.print(String(strzap)+String(" ")+String(otvet[zapnum]),1,45);
	          //myOLED.print(String(meslen)+String(" ")+String(zapnum)+String(" "),1,55);
	          //myOLED.update();
	          #endif
		  if (zapnum<0) status1=10; // запрос не найден
		    else
			{ // копируем ответ, чтоб не затереть его новым частично
			  otvet4sendlen=otvlen[zapnum];
			  for (int i=0; i<otvet4sendlen; i++) otvet4send[i]=otvet[zapnum][i];
			  meslen=0;
			  Timer3s5Start(); // через 3,5 символа начать отправку сообщения
			  Timer8msStart(); // минимум 8мс паузу выдержать перед отправкой
			  Timer0s9Start(); // интервал передачи символа на отправку, чтоб не переполнить буфер передатчика
			  status1=12;			  
			}
		}
	}
      if (status1==12) // Status1=12 – Запрос окончен (прошло 1,5 символа с последнего байта), ожидаем 3,5 символа (добавлять 8 мс?) перед отправкой ответа МАСТЕРу по запросу.
	{
	  if (flag1) {SHOWERROR(MASTER_MUSOR); flag1=false;}
	  if (Timer3s5Ready() && Timer8msReady()) status=13;
	}
      if (status1==13) // Status1=13 – Отправка ответа мастеру, каждый символ через интервал 0,9-0,99символа, чтобы не переполнить буфер передатчик. После окончания Status=10
	{
	  if (Timer0s9Ready())
	    {
	      if (meslen<otvet4sendlen)
		{
		  Timer0s9Start();
		  Timer3s5Start();
		  Serial1write(otvet4send[meslen++]);
		}
	      else if (Timer3s5Ready) status1=10;
	    }
	  if (flag1) {SHOWERROR(MASTER_MUSOR); flag1=false;}
	}
      if (status2==20) // Status2=20 – ожидание следующего запроса2 от АНОДа
	{
	  if (flag2) 
	    {
		Timer2_1s5Start();
		meslen2=0;
		status2=21;
	    }
	}
      if (status2==21) // Status2=21 – принимается запрос2 от АНОДа, ожидание окончания (1,5 символа) и перевод в Status2=22.
	{
	  if (flag2)
	    {
		if (melsen2<maxlenzap) str2[meslen2++]=char2; 
		  else SHOWERROR(OVERFLOW_ZAPROS_LEN);
		Timer2_1s5Start();
		Timer2_3s5Start();
		Timer1secStart();
		flag2=false;
	    }
	  if (Timer2_1s5Ready())
	    {
	      // НАЙТИ ЗАПРОС и ОПРЕДЕЛИТЬ ЕГО НОМЕР otvet2num
	      zapnum2=findzapros(str2,meslen2); // -1 запрос не найден в сохранённых, следовательно ответ будет некуда сохранять
	      if (zapnum2<0) status20; // запрос не найден
		else {meslen2=0; status2=22;} // запрос найден
	    }
	}// status21
      if (status2==22) // Status2=22 – Ожидаем ответа через 3,5 символа, (если раньше то мусор,) или таймаут 1сек, затем статус 23 сохранить ответ и длину
	{
	  if (Timer1secReady()) status2=23;
	  if (flag2) {if (Timer2_3s5Ready()) {Timer2_1s5Start(); status2=23;} else SHOWERROR(ANOD_MUSOR);}
	}
      if (status2==23) // Status2=23 – Принимается ответ от АНОДА, ожидание окончания 1,5 символ и сохранение в массив, Status2=20
	{
	  if (flag2) 
	    {
	      if (meslen2<maxlenotv) str2[meslen2++]=char2; else SHOWERROR(OVERFLOW_OTVET_LEN);
	      Timer2_1s5Start();
	      flag2=false;
	    }
	  if (Timer2_1s5Ready()) 
	    {// ОБНОВИТЬ ОТВЕТ в ОБЩЕМ МАССИВЕ ОТВЕТОВ
	      for (int i=0; i<meslen2; i++) otvet[zapnum2][i]=str2[i];
	      otvlen[zapnum2]=meslen2;
	      status2=20;
	    }
	}
    } //KATOD WORK MODE
#endif // ########################################## KATOD ###############################

#ifdef ANOD // ************************************* ANOD ********************************
    { // ANOD WORK MODE
      if (status2==20) // Status2=20 – ожидание ТАЙМЕРа, для отправки запроса СЛЭЙВу и КАТОДу, все входные данные считаются мусором
	{
	  do { zapnum++; zapnum %= kolzp;} while (zpcount[zapnum]<=zpcountmin); // фильтрация одиночных запросов или случайно повторённых 2 раза, zpcountmin должно быть не меньше 2 но и не больше 3-4
 	  if (ZTTimeoutMS(timezp[zapnum],periodzp[zapnum]))
	    {
	      timezp[zapnum]=millis();
	      Timer0s9Start();
	      meslen=0;
	      status2=21;
	    }
	  if (flag2) {SHOWERROR(SLAVE_MUSOR); flag2=false;}
	}
      if (status2==21) // Status2=21 – Отправляется запрос в СЛЭЙВ и КАТОД побайтно параллельно через интервал 0,9-0,99символа, чтобы не переполнить буфер передатчик, затем Status2=22
	{
	  if (Timer0s9Ready())
	    {
	      if (meslen<zplen[zapnum])
		{
		  Timer0s9Start();
		  Timer3s5Start();
		  char charz=zp[zapnum][meslen++];
		  Serial1write(charz);
		  Serial2write(charz);
		}
	      else {Timer1secStart(); status2=22;}
	    }
	  if (flag2) {SHOWERROR(SLAVE_MUSOR); flag2=false;}
	}
      if (status2==22) // Status2=22 – Пауза 3,5 символа и затем ожидание ответа от СЛЭЙВа, всё что раньше придёт в мусор. С первым символом от СЛЭЙВА после 3,5 символа Status2=23
	{
	  if (Timer1secReady()) status2=20; // ответ не пришёл по таймауту
	  if (flag2) {if (Timer3s5Ready()) status2=23; else {SHOWERROR(SLAVE_MUSOR); flag2=false;}} // пошёл ответ через 3,5 символа
	}
      if (status2==23) // Status2=23 – Полученный байт от СЛЭЙВа отправляется КАТОДу, без проверок длины, только ожидание 1,5 символа, затем Status2=24.
	{
	  if (flag2)
	    {
		Timer1s5Start();
		Timer3s5Start();
		Timer8msStart();
		Serial1write(char2);
		flag2=false;
	    }
	  if (Timer1s5Ready()) status2=24; // ответ передан КАТОДу
	}
      if (status2==24) // Status2=24 – Обязательная пауза 3,5 символа перед отправкой следующего запроса, если вдруг уже ТАЙМЕР подошёл, тогда Status2=20
	{
	  if (flag2) {SHOWERROR(SLAVE_MUSOR); flag2=false;}
	  if ( Timer3s5Ready() && Timer8msReady() ) status2=20;  // добавляем таймер 8 мс, для медленных контроллеров
	}
      if (flag1) {SHOWERROR(KATOD_MUSOR); flag1=false;}
    } // ANOD WORK MODE
#endif // ########################################### ANOD *************************************
#endif // defined(KATOD) || defined(ANOD)

#if defined(MASTER) || defined(SLAVE)
knopka=!(digitalReadButtonPin()); 
if ( !knopka_prep && knopka) {curaddr++; curaddr=curaddr%maxadr; knopka_prep=true; } // триггер на нажатие кнопки
if ( knopka_prep && !knopka) {/* триггер на отпускание кнопки*/  knopka_prep=false;}
#endif

#ifdef SLAVE // *************************************** SLAVE *******************************
    unsigned long zaprosotvetslave; //время 
    if (!zaprosready && Serial2available())
      { // если есть данные запроса от КАТОДа
        meslen=0;
        zaprosready=true;
        do 
        {
          while (Serial2available()) 
          {
            char inChar = (char)Serial2read(); // считываем символ из КАТОДа
            str2[meslen++]=inChar;
            str2[meslen]=0; //сохраняем сразу конец строки, для облегчения вывода строки на экран
            TimerEOMsetTimeout(timeEOM); // перезапускаем таймер окончания посылки (3,5 символа по стандарту модбас)
          }
        } while (!TimerEOMisReady()); //ждём пока таймер окончания посылки отработает (3,5 символа по стандарту модбас)
        zaprosotvetslave=millis();
        TimerNSsetTimeout(timeNS);               // старт таймера ожидания начала ответа 8мс
      }
    if (TimerNSisReady() && zaprosready)// ждём пока отработает таймер задержки ответа 8мс
    { // парсер адреса и данных 
      int curadr=str2[sizeof(req)]-'0';
      int data=str2[sizeof(req)+1]-'0';
      if (curadr == ADDRESS)
      { 
        String dopstr="_";
        if (knopka) dopstr=String("#"); 
        outString=String(str2);
        outString+=dopstr;
        outString+=String(millis()-zaprosotvetslave);
        outString+=dopstr+millis(); 
        Serial1.print(outString);
        #ifdef SHOWOLED
          myOLED.print(outString+String("*     "),1,data*10+1);
          myOLED.update();
          myOLED.print(outString+String(" "),1,data*10+1);
          myOLED.update();
        #endif
      } 
      else 
      {
        Serial1print("error");
        #ifdef SHOWOLED
          myOLED.clrScr();
          myOLED.print(String(" addr: ")+String(curadr),1,1);
          myOLED.print(String(str2),1,10);
          myOLED.update();
        #endif
      };
      zaprosready=false;
    }
#endif // ######################################## SLAVE #####################################

#ifdef MASTER // ************************************ MASTER ***********************************
  char dy=10;
  knopkaproc();
  unsigned long zaprosotvet;
  if (TimerZaprosisReady()) // запуск серии запросов по таймеру 
  {  TimerZaprossetTimeout(TimerZaprosTime); // интервал отправки запроса для MASTER
    for (int i=0; i<maxfun; i++) 
    {
       outString=String(req);
       outString+=String(curaddr);
       outString+=String(i);
       Serial1.print(outString); 
       #ifdef SHOWOLED
         zaprosotvet=millis();
       #endif
       TimerZapros1setTimeout(TimerZapros1Time); //максимальное время ожидание ответа на запрос
       while (!Serial2available() && (!TimerZapros1isReady())) { knopkaproc(); }; //ждём пока не появятся данные на входе - ответ, но не дольше чем следующий таймер
       TimerEOMsetTimeout(timeEOM); 
       while (!TimerEOMisReady())
       {//ждём пока таймер окончания посылки отработает (3,5 символа по стандарту модбас)
         while (Serial2available()) 
          {
            char inChar = (char)Serial2read();
            str2[meslen++] = inChar;
            str2[meslen] = 0; // дописываем конец строки для упрощения вывода на экран
            TimerEOMsetTimeout(timeEOM);
          }
       }
       #ifdef SHOWOLED
         zaprosotvet=millis()-zaprosotvet;
         myOLED.print(outString+String("*"),1,i*10+1+dy); //индикатор отправки запроса
         myOLED.update();
         myOLED.print(outString+String(" "),1,i*10+1+dy); //индикатор обновления отправки запроса
         myOLED.update();
       #endif
       #ifdef SHOWOLED
          int shiftx=35;
          myOLED.print(String("del ")+String(zaprosotvet)+String("ms "),1,50);
          myOLED.print(String(str2)+String("* "),shiftx,(i)*10+1+dy);
          myOLED.update();
          myOLED.print(String(str2)+String("    "),shiftx,(i)*10+1+dy);
          myOLED.update();
      #endif
      meslen=0; 
      TimerNSsetTimeout(timeNS_MASTER);               // старт таймера ожидания начала ответа 8мс
      while(!(TimerNSisReady()));// ждём пока отработает таймер задержки ответа 8мс
    }
  }
#endif // ############################################ MASTER ###############################
}

int savenewzapros(char *str, int mlen) 
		{ int zapn=-1; // -1 если массив закончился
		  if (kolzp<maxzapros) // если запрос новый (не найден в списке сохранённых) и их количество не превышает массив, то сохраняем его
	           { 
	            for (int j=0; ( (j<mlen) && (j<maxlenzap) ); j++) // копирование в массив считанной строки
	              zp[kolzp][j]=str[j];
	            zplen[kolzp]=mlen;
		    otvlen[kolzp]=0; // для нового запроса обнуляем длину ответа
	            zpcount[kolzp]=1; // начинаем отсчёт количества повторений запроса
	            zapn=kolzp; // номер запроса в массиве
	            #ifdef SHOWOLED
	              zp[kolzp][mlen]=0; // записать конец строки для упрощёного вывода на экран, для отправки строки целиком 
	              strn1=String(zp[kolzp])+String(" ")+String(zpcount[kolzp]);
	              x1=1; y1=1+kolzp*9; strn2=String("kolzp=")+String(kolzp+1); x2=80; y2=55;
        	    #endif
	            kolzp++;
		    #ifdef ANOD
		      timezp[zapn]=millis();
		    #endif
	           }
		  return zapn;
		}


int findzapros(char * str, int mlen)
{ int zapn=-1;
	        for (int i=0; i<kolzp; i++) 
	          { // ищем в сохранённых запросах 
	            int j=0; //счётчик длины одинаковых сиволов в запросе
	            if (mlen==zplen[i]) // естественно что длины должны совпадать
		     {	
	              for (j=0; (j<mlen) && (str[j]==zp[i][j]); j++); // посимвольное сравнение запросов
	              if ((j==mlen)) // если все символы в запросе совпали и длины запросов, то запрос найден, просто подсчитываем его
	               { 
	                zapn=i; // номер запроса в массиве
	                #ifdef SHOWOLED
	                  strn1=String(zpcount[i]); x1=28; y1=1+i*9;
	                #endif
	                break;
	               } 
		     }
	          }
  return zapn;
}

int knopkaproc()
{
  knopka=!(digitalReadButtonPin()); // в цикле, чтобы реакция на кнопку максимально быстрой была
  if ( !knopka_prep && knopka) 
  {
    curaddr++; curaddr=curaddr%maxadr; knopka_prep=true;
    #ifdef SHOWOLED
      myOLED.print(String("ADDR="+String(curaddr)),1,1);
      myOLED.update();
    #endif
  } // триггер на нажатие кнопки
  if ( knopka_prep && !knopka) {/* триггер на отпускание кнопки*/ knopka_prep=false;}
  return (knopka);
}

int knopkamode() // возвращает 1, если нажата кнопка
{
            return !(digitalReadButtonPin()); 
}

void SHOWERROR(int ERRORNUMBER)
{
  #ifdef MSTN100
    //digitalWrite(LED_RED_PINNUM,HIGH);
    if (ERRORNUMBER==OVERFLOW_LONG_ZAPROS2)  {digitalWrite(LED_GRN_PINNUM,HIGH); digitalWrite(LED_RED_PINNUM,HIGH);}
    if (ERRORNUMBER==REDLED)  digitalWrite(LED_RED_PINNUM,HIGH);
    if (ERRORNUMBER==GREENLED)  digitalWrite(LED_GRN_PINNUM,HIGH);
    if (ERRORNUMBER==OVERFLOW_LONG_ZAPROS)  digitalWrite(LED_RED_PINNUM,HIGH);
    if (ERRORNUMBER==OVERFLOW_LONG_OTVET)  digitalWrite(LED_RED_PINNUM,HIGH);
    if (ERRORNUMBER==NO_ANSWER)  digitalWrite(LED_RED_PINNUM,HIGH);
    if (ERRORNUMBER==OVERFLOW_KOL_ZAPROS) digitalWrite(LED_RED_PINNUM,HIGH);
    if (ERRORNUMBER==UNKNOWN_OTVET)  {digitalWrite(LED_GRN_PINNUM,HIGH); }
    if (ERRORNUMBER==NOT_RESET_READ1_FLAG)  {digitalWrite(LED_RED_PINNUM,HIGH); }
    if (ERRORNUMBER==NOT_RESET_READ2_FLAG)  {digitalWrite(LED_RED_PINNUM,HIGH); }
#else
    digitalWrite(LED_BUILTIN,HIGH);
  #endif
  #ifdef SHOWOLED
    String textmsg="ERROR";
    if (ERRORNUMBER==OVERFLOW_KOL_ZAPROS) textmsg=String("OVERFLOW! max kol zapros");
    if (ERRORNUMBER==OVERFLOW_LONG_OTVET) textmsg=String("OVERFLOW! LONG OTVET");
    if (ERRORNUMBER==OVERFLOW_LONG_ZAPROS) textmsg=String("OVERFLOW! LONG ZAPROS");
    myOLED.print(textmsg,1,45);
    myOLED.update();
  #endif
}

void UpdateZaprosTimer(int zapn, period)
{
   if (zpcount[zapn]==2) period[zapn]=periodtime(timezp[zapn],millis())
   if ( (zpcount[zapn]>2) && (periodtime(timezp[zapn],millis())<periodzp[zapn]) ) period[zapn]=periodtime(timezp[zapn],millis())
   timezp[zapn]=millis();
}

unsigned long periodtime(unsigned long startTime, unsigned long currentTime)
{  if (currentTime >= startTime) { return (currentTime - startTime);} 
  else { return ((4294967295 - startTime) + currentTime);}
}

bool overful(unsigned long startTime, unsigned long period, unsigned long currentTime)
{  if (currentTime >= startTime) { return (currentTime >= (startTime + period));} 
  else { return (currentTime >= (4294967295-startTime+period));}
}

bool ZTTimeoutMS(unsigned long startTime, unsigned long period)
{ return (overful(startTime, period, millis()));}

bool ZTTimeoutUS(unsigned long startTime, unsigned long period)
{ return (overful(startTime, period, micros()));}

