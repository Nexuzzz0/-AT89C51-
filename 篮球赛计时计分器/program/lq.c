#include "reg52.h"	

typedef unsigned int uint;	 
typedef unsigned char uchar;

sbit LSA = P2^0;      // 38������
sbit LSB = P2^1;
sbit LSC = P2^2;
sbit LSD = P2^3;      // ������
sbit F1 = P2^4;       // ����ʱ����ܿ�����
sbit F2 = P2^5;
sbit S1 = P2^6;
sbit S2 = P2^7;

uchar KeyValue;
uchar feng = 0;  
uint feng_count = 0; 

#define GPIO_DIG P0    // �Ʒ�����������
#define GPIO_KEY P1    // ��������
#define GPIO_TIME P3   // ����ʱ����������

uchar code DIG_CODE[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f}; // 0-9
unsigned char Show[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };    // �����������������
unsigned char TShow[4] = { 0, 0, 0, 0 };

uint Ascore, Bscore, Change;  
uchar Period;             
uchar m = 10, n = 0, c;    

void delay(uint i)
{
    while(i--);	
}

// �������
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
            while((a < 500) && (GPIO_KEY != 0xf0)) // �����ɿ�
            {
                delay(1000);
                a++;
            }
        }
    }
}

//  �ȷֿ���
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
            case(8): Period = Period + 1; KeyValue = 20; break;    // ��������
            case(12): TR0 = 1; m = 10;  n = 0; feng = 1; KeyValue = 20; break;     // ������ʼ
            case(13): TR0 = 1; feng = 1; KeyValue = 20; break;                           // ��������
            case(14): feng = 1; TR0 = 0; KeyValue = 20; break;            // ������ͣ
            case(15): feng = 0; KeyValue = 20; break;                    // �������ر�
        }
}

// ������
void fengMQ(void)
{
    if (feng == 1)  
    {
        LSD = 1; 
        feng_count++; 

        if (feng_count >= 2000)  // 2s(1ms��ʱһ��)
        {
            feng = 0;         
            feng_count = 0;   
            LSD = 0;          
        }
    }
}

// ���������
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

// ����ܶ�̬ɨ��
void DigDisplay()
{
    uchar i, w;
    for(i = 0; i < 8; i++)
    {
        switch(i)	  // λѡ��ѡ�����������ܣ��ȷ���ʾ
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
        GPIO_DIG = DIG_CODE[Show[i]]; // ���Ͷ���
        delay(10); 
        P0 = 0x00;
    }
    for(w = 0; w < 4; w++) // ����ʱ��ʾ
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

// ��ʱ����ʼ��
void Timer0Init()
{
    TMOD |= 0x01; // ������ʽ1
    TH0 = 0xFC;	 // ���ó�ʼֵ����ʱ1ms
    TL0 = 0x18; 
    EA = 1;			 // ���ж�
    ET0 = 1;		 // �򿪶�ʱ��0�ж�
    TR0 = 0;		 // ��ʼ����������ʱ��
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
    if(q == 1000) // ��ʱ1000�Σ�1s    65536 - 64536 = 1000  
    {
        q = 0;         
        if(n == 0) // ����Ϊ0������-1
        {
            n = 59; 
            m = m - 1;
        }
        n = n - 1; // ����-1
    }
    if(m == 0 && n == 0)  //  һ�ڽ���ʱ������
    {
        TR0 = 0; 
    }
}