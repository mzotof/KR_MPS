#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

void init_UART(void)
{
    UBRRH=0;
    UBRRL=51;
    UCSRA=0;
    UCSRB=0b00011000;
    UCSRC=0b10000110;
}

void send_Uart(int c)//   Отправка байта
{
    PORTD |= (1<<PD2);
    UDR = c;
    while(!(UCSRA&(1<<UDRE))) {};
    while(!(UCSRA&(1<<TXC))) {};
    PORTD &= ~(1<<PD2);
}

volatile int queue[100], qlen = 0;

int recieve_Uart(void)
{
    while(!(UCSRA&(1<<RXC))) {};
    return UDR;
}

void insert(int a)
{
    queue[qlen] = a;
    qlen++;
}

int delete(void)
{
    int rett = queue[0];
    int i = 0;
    while (queue[i] != -1)
    {
        queue[i] = queue[i+1];
        i++;
    }
    qlen--;
    return rett;
}

int main()
{
    int i, j, data;
    for (i=0;i<100;i++)
        queue[i]=-1;
    int digits[] = { 0b00111111,
                     0b00000110,
                     0b01011011,
                     0b01001111,
                     0b01100110,
                     0b01101101,
                     0b01111101,
                     0b00000111,
                     0b01111111,
                     0b01101111  };
    init_UART();

    DDRA  = 0b01111111;
    DDRB  = 0b01111111;
    DDRC  = 0b00001111;
    DDRD  = 0b00000110;
    PORTD = 0b00000101;

    while(1)
    {
        send_Uart(0b10000000);
        //PORTD &= ~(1<<PD2);
        data = recieve_Uart();
        PORTD |= (1<<PD2);
        if (data != 0b11111111)
            insert(data&0b01111111);
        /*send_Uart(qlen);
        PORTD &= ~(1<<PD2);
        data = recieve_Uart();
        PORTD |= (1<<PD2);
        if (data == 1)
        {
            int a = delete();
            send_Uart(a);
            qlen--;
            for (j = 0; j < 3; j++)
            {
                for (i = 0; i < 50; i++)
                {
                    PORTC = 0b00001010;
                    PORTA = digits[a / 10];
                    PORTB = digits[a / 10];
                    _delay_ms(10);
                    PORTC = 0b00000101;
                    PORTA = digits[a % 10];
                    PORTB = digits[a % 10];
                    _delay_ms(10);
                } 
                PORTC = 0b00001111;
                _delay_ms(500);
            }
        }*/
        if (queue[0] != -1)
        {
            if (queue[1] != -1)
            {
                PORTC = 0b00001010;
                PORTA = digits[queue[0] / 10];
                PORTB = digits[queue[1] / 10];
            }
            else
            {
                PORTC = 0b00001110;
                PORTA = digits[queue[0] / 10];
            }
            _delay_ms(10);
            if (queue[1] != -1)
            {
                PORTC = 0b00000101;
                PORTA = digits[queue[0] % 10];
                PORTB = digits[queue[1] % 10];
            }
            else
            {
                PORTC = 0b00001101;
                PORTA = digits[queue[0] % 10];
            }
            _delay_ms(10);
        }
    }
}