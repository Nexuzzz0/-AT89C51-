#include "reg52.h"	

typedef unsigned int uint;	 
typedef unsigned char uchar;

sbit LSA = P2^0;      // 38译码器
sbit LSB = P2^1;
sbit LSC = P2^2;
sbit LSD = P2^3;      // 蜂鸣器
sbit F1 = P2^4;       // 倒计时数码管控制器
sbit F2 = P2^5;
sbit S1 = P2^6;
sbit S2 = P2^7;

uchar KeyValue;
uchar feng = 0;  
uint feng_count = 0; 

#define GPIO_DIG P0    // 计分数码管输出口
#define GPIO_KEY P1    // 按键输入
#define GPIO_TIME P3   // 倒计时数码管输出口

uchar code DIG_CODE[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f}; // 0-9
unsigned char Show[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };    // 存放向数码管输入的数
unsigned char TShow[4] = { 0, 0, 0, 0 };

uint Ascore, Bscore, Change;  
uchar Period;             
uchar m = 10, n = 0, c;    

void delay(uint i)
{
    while(i--);	
}

// 按键检测
void KeyDown(void)
{
    char a = 0;
    GPIO_KEY = 0x0f;
    KeyValue = 20; 
    
    if(GPIO_KEY != 0x0f) 
    {
        delay(100);
        if(GPIO_KEY != 0x0f)
        {	
            // ???
            GPIO_KEY = 0X0F;
            switch(GPIO_KEY)
            {
                case(0X0E): KeyValue = 0; break;
                case(0X0D): KeyValue = 4; break;
                case(0X0B): KeyValue = 8; break;
                case(0X07): KeyValue = 12; break;
            }
            GPIO_KEY = 0XF0;
            switch(GPIO_KEY)
            {
                case(0X70): KeyValue = KeyValue; break;
                case(0XB0): KeyValue = KeyValue + 1; break;
                case(0XD0): KeyValue = KeyValue + 2; break;
                case(0XE0): KeyValue = KeyValue + 3; break;
            }
            while((a < 500) && (GPIO_KEY != 0xf0)) // 按键松开
            {
                delay(1000);
                a++;
            }
        }
    }
}

//  比分控制
void Score()
{
    if(KeyValue != 20)
        switch(KeyValue) // A
        {
            case(0): Ascore = Ascore + 1; KeyValue = 20; break;  
            case(3): Ascore = Ascore - 1; KeyValue = 20; break;
        }
        switch(KeyValue) // B
        {
            case(4): Bscore = Bscore + 1; KeyValue = 20; break;
            case(7): Bscore = Bscore - 1; KeyValue = 20; break;
        }
        switch(KeyValue)
        {
            case(8): Period = Period + 1; KeyValue = 20; break;    // 节数增加
            case(12): TR0 = 1; m = 10;  n = 0; feng = 1; KeyValue = 20; break;     // 比赛开始
            case(13): TR0 = 1; feng = 1; KeyValue = 20; break;                           // 比赛继续
            case(14): feng = 1; TR0 = 0; KeyValue = 20; break;            // 比赛暂停
            case(15): feng = 0; KeyValue = 20; break;                    // 蜂鸣器关闭
        }
}

// 蜂鸣器
void fengMQ(void)
{
    if (feng == 1)  
    {
        LSD = 1; 
        feng_count++; 

        if (feng_count >= 2000)  // 2s(1ms计时一次)
        {
            feng = 0;         
            feng_count = 0;   
            LSD = 0;          
        }
    }
}

// 数码管数字
void Conut(void)
{
    Show[0] = Period % 10;
    Show[1] = Ascore / 100;
    Show[2] = Ascore / 10 % 10;
    Show[3] = Ascore % 10;
    
    Show[4] = 0;
    Show[5] = Bscore / 100;
    Show[6] = Bscore / 10 % 10;
    Show[7] = Bscore % 10;
    
    TShow[0] = m / 10;
    TShow[1] = m % 10;
    TShow[2] = n / 10;
    TShow[3] = n % 10;
}

// 数码管动态扫描
void DigDisplay()
{
    uchar i, w;
    for(i = 0; i < 8; i++)
    {
        switch(i)	  // 位选，选择点亮的数码管，比分显示
        {
            case(0):
                LSA = 0; LSB = 0; LSC = 0; break;
            case(1):
                LSA = 1; LSB = 0; LSC = 0; break;
            case(2):
                LSA = 0; LSB = 1; LSC = 0; break;
            case(3):
                LSA = 1; LSB = 1; LSC = 0; break;
            case(4):
                LSA = 0; LSB = 0; LSC = 1; break;
            case(5):
                LSA = 1; LSB = 0; LSC = 1; break;
            case(6):
                LSA = 0; LSB = 1; LSC = 1; break;
            case(7):
                LSA = 1; LSB = 1; LSC = 1; break;	
        }
        GPIO_DIG = DIG_CODE[Show[i]]; // 发送段码
        delay(10); 
        P0 = 0x00;
    }
    for(w = 0; w < 4; w++) // 倒计时显示
    {
        switch(w)	 
        {
            case(0):
                F1 = 0; F2 = 1; S1 = 1; S2 = 1; break;
            case(1):
                F1 = 1; F2 = 0; S1 = 1; S2 = 1; break;
            case(2):
                F1 = 1; F2 = 1; S1 = 0; S2 = 1; break;
            case(3):
                F1 = 1; F2 = 1; S1 = 1; S2 = 0; break;	
        }
        GPIO_TIME = DIG_CODE[TShow[w]];
        delay(10); 
        P3 = 0x00;   
    }
}

// 定时器初始化
void Timer0Init()
{
    TMOD |= 0x01; // 工作方式1
    TH0 = 0xFC;	 // 设置初始值，定时1ms
    TL0 = 0x18; 
    EA = 1;			 // 打开中断
    ET0 = 1;		 // 打开定时器0中断
    TR0 = 0;		 // 初始化不启动定时器
}

void main()
{	
    Timer0Init();
    while(1)
    {	
        KeyDown();
        Score();					
        Conut();    		
        fengMQ();			
        DigDisplay(); 		
    }		
}

void Timer0() interrupt 1   
{
    static uint q;  
    TH0 = 0xFC;	
    TL0 = 0x18; 
    q++;
    if(q == 1000) // 计时1000次，1s    65536 - 64536 = 1000  
    {
        q = 0;         
        if(n == 0) // 秒钟为0，分钟-1
        {
            n = 59; 
            m = m - 1;
        }
        n = n - 1; // 秒针-1
    }
    if(m == 0 && n == 0)  //  一节结束时间重置
    {
        TR0 = 0; 
    }
}