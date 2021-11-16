#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

int init_UART(void)
{
    //  Установка скорости 9600
    UBRRH=0;    //  UBRR=f/(16*band)-1 f=8000000Гц band=9600, 
    UBRRL=51;   //  нормальный асинхронный двунаправленный режим работы
    
//          RXC         -   завершение приёма
//          |TXC        -   завершение передачи
//          ||UDRE      -   отсутствие данных для отправки
//          |||FE       -   ошибка кадра
//          ||||DOR     -   ошибка переполнение буфера
//          |||||PE     -   ошибка чётности
//          ||||||U2X   -   Двойная скорость
//          |||||||MPCM -   Многопроцессорный режим
//          76543210
    UCSRA=0b00000000;

//          RXCIE       -   прерывание при приёме данных
//          |TXCIE      -   прерывание при завершение передачи
//          ||UDRIE     -   прерывание отсутствие данных для отправки
//          |||RXEN     -   разрешение приёма
//          ||||TXEN    -   разрешение передачи
//          |||||UCSZ2  -   UCSZ0:2 размер кадра данных
//          ||||||RXB8  -   9 бит принятых данных
//          |||||||TXB8 -   9 бит переданных данных
//          76543210
    UCSRB=0b10011000;   //  разрешен приём и передача по UART

//          URSEL       -   всегда 1
//          |UMSEL      -   режим:1-синхронный 0-асинхронный
//          ||UPM1      -   UPM0:1 чётность
//          |||UPM0     -   UPM0:1 чётность
//          ||||USBS    -   топ биты: 0-1, 1-2
//          |||||UCSZ1  -   UCSZ1:2 размер кадра данных
//          ||||||UCSZ0 -   UCSZ0:2 размер кадра данных
//          |||||||UCPOL-   в синхронном режиме - тактирование
//          76543210
    UCSRC=0b10000110;   //  8-битовая посылка
}

void send_Uart(int c)//   Отправка байта
{
    PORTD |= (1<<PD2);
    UDR = c;
    while(!(UCSRA&(1<<UDRE))) {};
    while(!(UCSRA&(1<<TXC))) {};
    PORTD &= ~(1<<PD2);
}

volatile int counter = 0, send = 0/*, queue[100], qlen = 0*/;

ISR(USART_RXC_vect)
{
    int data = UDR;
    /*queue[qlen] = data;
    qlen++;*/
    if (data==0b10000000)
    {
        if (counter != send)
        {
            send++;
            send_Uart(send|0b10000000);
        }
        else
            send_Uart(0b11111111);
    }
}

int main()
{ 
    sei();
    int i, j;
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
    DDRB  = 0b00000011;
    PORTB = 0b00000111;
    DDRD  = 0b00000110;
    PORTD = 0b00000001;

    while(1)
    {
        if (!(PINB&0b00000100))
        {
            counter++;
            if (counter == 100)
                counter = 1;
            for (j = 0; j < 3; j++)
            {
                for (i = 0; i < 50; i++)
                {
                    PORTB = 0b00000110;
                    PORTA = digits[counter / 10];
                    _delay_ms(10);
                    PORTB = 0b00000101;
                    PORTA = digits[counter % 10];
                    _delay_ms(10);
                } 
                PORTB = 0b00000111;
                _delay_ms(500);
            }
        }
    }
}