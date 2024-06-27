#include <18F25K22.h>
#fuses INTRC_IO, NOMCLR, NOWDT, NOPLLEN

// internal clock = 8000000 = 8M
#use delay(internal = 8M) 
#byte PORTA = 0xF80
#byte TRISA = 0xF92
#byte PORTB = 0xF81
#byte TRISB = 0xF93
#byte PORTC = 0xF82
#byte TRISC = 0xF94
#bit limit_sensor_right = PORTB.3
#bit limit_sensor_left = PORTB.5

// PORT0 Address
#byte T0CON=0XFD5
#byte INTCON=0xFF2
#bit INT0IF = INTCON.1
#byte INTCON2=0XFF1
#byte TMR0L=0XFD6
#byte TMR0H=0XFD7
#bit  TMR0IF = INTCON.2
#bit  led = portc.7
#bit led2 = PORTC.6
#bit led3 = PORTC.5
#bit led4 = PORTC.4
#bit led5 = PORTC.3
#bit SW = PORTA.1
#byte     PIR1 = 0xF9E
#byte     PIE1 = 0xF9D
//Timer2_set
#byte PR2 = 0xFBB
#byte T2CON = 0xFBA
#bit   TMR2ON = T2CON.2
#byte TMR2 = 0xFBC
#byte PIE1 = 0xF9D
//#byte PIR1 = 0xF9E
//#byte IPR1 = 0xF9F
#byte IPR1 = 0xF9F
#byte PIR1 = 0xF9E
#bit   TMR2IE = PIE1.1
#bit   TMR2IP = IPR1.1
#bit   TMR2IF = PIR1.1
//Timer4_set
#byte PR4 = 0xF52
#byte T4CON = 0xF51
#bit TMR4ON = T4CON.2
#byte TMR4 = 0xF53
#byte PIE5 = 0xF7D
#bit TMR4IE = PIE5.0
#byte PIR5 = 0xF7E
#bit TMR4IF = PIR5.0
#byte IPR5 = 0xF7F
#bit TMR4IP = IPR5.0
//Timer6_set
#byte PR6 = 0xF4B
#byte T6CON = 0xF4A
#bit TMR6ON = T6CON.2
#byte TMR6 = 0xF4C
#byte PIE5 = 0xF7D
#bit TMR6IE = PIE5.2
#byte PIR5 = 0xF7E
#bit TMR6IF = PIR5.2
#byte IPR5 = 0xF7F
#bit TMR6IP = IPR5.2
//CCP1M
#byte CCP1CON = 0xFBD
#bit   Lsb1 = CCP1CON.5
#bit   Lsb2 = CCP1CON.4
#byte CCPR1L = 0xFBE
#byte CCPR1H = 0xFBF
#byte CCPTMRS0 = 0xF49
#bit PxM7 = CCP1CON.7
#bit PxM6 = CCP1CON.6

#byte BAUDCON1 = 0xFB8
#byte RCSTA1 = 0xFAB
#byte TXSTA1 = 0xFAC
#byte TXREG1 = 0xFAD
#byte RCREG1 = 0xFAE
#byte SPBRG1 = 0xFAF
#byte INTCON = 0xFF2

#bit TRMT = TXSTA1.1 
#bit RC1IE = PIE1.5

int16 count = 0;
unsigned int16 i = 0;  
unsigned int16 encoder_pulse = 0;
int16 Timer4_count = 0;
int16 Timer6_count = 0;
int8 sw_t = 0;
unsigned int8 safe_sensor_count = 0;
int8 door_open = 0;
int8 safe_on = 0;
char L1 = '0',L2 = '0',L3 = '0';
int8 sync_flag = 0;
int8 limit_sensor_left_flag = 0;
int8 limit_sensor_right_flag = 0;
int8 time6_value = 0;
int8 open = 0;
int8 close = 0;

void init_port()
{
   delay_ms(300);
   TRISA = 0b00010000;
   PORTA = 0;
   TRISB = 0x00;
   PORTB = 0x19;
   TRISC = 0x83;
   PORTC = 0;
   RC1IE = 1;
}
void TX()                   //���
{
   TXSTA1 = 0b00100010;
   RCSTA1 = 0b10010000;
   BAUDCON1 = 0b00000000;
   SPBRG1 = 12;
   
   RCREG1 = 0x00;
   TXREG1 = 0x00;
}

void init_timer0()          //����Ʈ ���� ���ͷ�Ʈ
{
   T0CON = 0b11101000;
   INTCON = 0b11110000;
   INTCON2 = 0b00000100;
   TMR0L = 242;
   //TMR0H = 0;
}
void Timer2_Set()
{
   TMR2ON = 1;
   TMR2IE = 1;
   TMR2 = 0;
   T2CON = 0b00000110;  //prescaler 1:16
   PR2 = 0x65;
}

void Timer4_Set()
{
   TMR4ON = 0;
   TMR4IE = 1;
   TMR4 = 0;
   T4CON = 0b01111111;  //prescaler 256
   PR4 = 254;    //1���ֱ�  78> 0.5�� �ֱ�
}
void Timer6_Set()
{
   TMR6ON = 1;
   TMR6IE = 1;
   TMR6 = 0;
   T6CON = 0b01111111;  //prescaler 256
   PR6 = 253; 
   TMR6IF = 0;
}

void PWM_set()
{
   CCPTMRS0 = 0b11011000; // PWM Timer2
   CCP1CON = 0b00001100; // PWM mode 5,4 bit Lsb  Full-bridge P1A = RC2 // P1B = RB2 // P1C = RB1 // P1D = RB4 ���缭 TRIS ������� => 0
}
void CW() //��ȸ��
{
      PxM7 = 0;
      PxM6 = 1;
      CCPR1L = 10;  //1 
      delay_ms(1000);
      CCPR1L = 40;  //1 
      delay_ms(1000);
      CCPR1L = 10;  //1 
      delay_ms(1000);
      CCPR1L = 0;  //1 
      delay_ms(1000);

}

void CCW() //��ȸ��
{
      PxM7 = 1;
      PxM6 = 1;
      CCPR1L = 10;  //1 
      delay_ms(2000);
      CCPR1L = 50;  //1 
      delay_ms(1000);
      CCPR1L = 30;  //1 
      delay_ms(1000);
      CCPR1L = 0;  //1 
      delay_ms(1000);

}

void main()
{
   
   init_port();
   init_timer0();
   Timer2_Set();    //��������PWN
   PWM_set();
   Timer4_Set();    //������ ���Ͱ� ���ư� �ð�������
   Timer6_Set();    //�������� �ð�
   TX();            //TX ���
   led = 0;
   

   while(true)
   {
      if(limit_sensor_left == 0){limit_sensor_left_flag = 1; }
      if(limit_sensor_right == 0){limit_sensor_right_flag = 1; }
   }
}

#INT_TIMER4
void Timer4()   //�������� �ð�
{
         Timer4_count++;
         TMR4 = 0;
         TMR4IF = 0;
}
#INT_TIMER6
void Timer6()   //���� ���� �ð� ����
{
         
         if(sync_flag == 1 && safe_on == 0)                //���==1 ���������� == 0
         {
            
            Timer6_count++;                               //Timer6 ī��Ʈ ����
            if(Timer6_count >= 0 && Timer6_count <=80 && limit_sensor_left_flag == 0)    //ccp10: 50 ccp5: 72
            {
               open = 1;                                  //���� ������ ����
               close = 0;
               PxM7 = 0; PxM6 = 1; CCPR1L = 10;           //���� �ӵ�
            }
            else if(Timer6_count >= 81 && Timer6_count <=153 && limit_sensor_left_flag == 0) 
            {
               PxM7 = 0; PxM6 = 1; CCPR1L = 5; 
            }
            if(limit_sensor_left_flag == 1)               //���� ����Ʈ ������ ������ ���
            {
               if(Timer6_count >= 140 && Timer6_count <=254) //140���ε� ������ ����Ʈ�� ��Ҵµ��� Ÿ�̸�6ī��Ʈ�� 153���� ������ ���Ͱ� ��ӵ��¿��������⶧��
               {
                  PxM7 = 0; PxM6 = 1; CCPR1L = 0;
                  time6_value = Timer6_count;             //�����µ��� �ð� �����ϱ�
               }
               else if(Timer6_count >= 255 && Timer6_count <=335)
               {
                  open = 0;
                  close = 1;                              //���� �����»��� �˷��ֱ�
                  PxM7 = 1; PxM6 = 1; CCPR1L = 10;
                  time6_value = Timer6_count;
               }
               else if(Timer6_count >= 336 && Timer6_count <=408)
               {
                  led2 = 1; 
                  PxM7 = 1; PxM6 = 1; CCPR1L = 5;
                  time6_value = Timer6_count;
               }
               limit_sensor_left_flag =0;               //Ȥ�� �ȵɰ�� Ÿ�̸�6ī��Ʈ409���Ǿȿ� �ֱ�
            }                                           //���ʸ���Ʈ���� 0���� �����
            else if(limit_sensor_right_flag == 1)       //�����ʸ���Ʈ������ ���������
            {
               PxM7 = 1; PxM6 = 1; CCPR1L = 0;          //�ϴ�����
               sync_flag = 0;                           //��� = 0
               time6_value = 0;                         //�����µ����ǽð� �����Ѱ��� �ʱ�ȭ
               timer6_count = 0;                        //Ÿ�̸�6 ī��Ʈ �ʱ�ȭ
               limit_sensor_right_flag = 0;             //������ ����Ʈ���� 0���� �����
            }
            TMR6 = 0;
            TMR6IF = 0;
         }
         else if(safe_on == 1)                         //������������ ���������
         {
            if(open == 1) {safe_on = 0;}              //���� �����µ��ȿ��� ����X
            else if(close == 1)                       //���� �����µ���
            {
               PxM7 = 0; PxM6 = 1; CCPR1L = 0;        //��������
               TMR4ON = 1;                            //Ÿ�̸�4 ����
               if(time6_value != Timer4_count)        //����� �����½ð��� Ÿ�̸�4�� ���ؼ� �ٸ���� ����
               {
                  PxM7 = 0; PxM6 = 1; CCPR1L = 10;    //�� �ݱ�
                  if(limit_sensor_left_flag == 1)     //���ʸ���Ʈ������ �ݾ������
                  {
                     PxM7 = 0; PxM6 = 1; CCPR1L = 0;  //��������
                     Timer6_count = 154;              //Ÿ�̸�6ī��Ʈ�� ���� ������ �������°� ���۵� �κ����� ���ư���
                     safe_on = 0;                     //���������� ��ȣ 0���� if�� ������
                     TMR4ON = 0;                      //Ÿ�̸�4 ����
                     Timer4_count = 0;                //Ÿ�̸�4 ī��Ʈ �ʱ�ȭ
                     
                  }
               }
            }
         }
}
#INT_TIMER0
void INT_ISR_T() //Ÿ�̸� �ð� ����
{
   count++; //
   TMR0L = 242;
   TMR0IF = 0;
}
#INT_EXT//���ͷ�Ʈ 0�� ���� �ڵ�
void flagup()  // ������ ����
{ 
   safe_on = 1;
   INT0IF = 0;
}
#INT_RDA
void RDA()  //���
      L1 = RCREG1;
      if(L1 =='1')
      {sync_flag = 1;}
      if(L1 == '0')
      {sync_flag = 0;}
}


