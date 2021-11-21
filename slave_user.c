#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned char counter = 0, // количество новых клиентов
                       send = 0;    // количество отправленных новых клиентов

void initUART(void) // инициализация UART
{
    UBRRL=51; //скорость 9600 Бод при f=8 МГц
    UCSRB=(1<<RXCIE)|(1<<RXEN)|(1<<TXEN); //разрешение прерывания по приему, приема и передачи
    UCSRC=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0); //8-битные посылки
}

void sendUart(unsigned char c) // отправка по UART
{
    PORTD |= (1<<PD2); // включение передачи
    UDR = c;
    while(!(UCSRA&(1<<UDRE))) {}; // ожидание опустошения UDR
    while(!(UCSRA&(1<<TXC))) {};  // ожидание отправки
    PORTD &= ~(1<<PD2); // включение приема
}

ISR(USART_RXC_vect) // прерывание по приему по UART
{
    unsigned char data = UDR;
    if (data==0b10000000) // если полученные данные относятся к этому МК
    {
        if (send < counter)
        {
            send++;
            sendUart(send|0b10000000); // добавить пользователя в очередь
        }
        else
            sendUart(0b11111111); // не добавлять никого в очередь
    }
}

void checkButton(unsigned char button_pin) // проверка нажатия на кнопку
{
    unsigned char i, j;
    const unsigned char digits[] = { 0b00111111,
                                     0b00000110,
                                     0b01011011,
                                     0b01001111,
                                     0b01100110,
                                     0b01101101,
                                     0b01111101,
                                     0b00000111,
                                     0b01111111,
                                     0b01101111 }; // соответствие цифр и их обозначения на дисплее
    if (!(PINB&button_pin)) // если кнопка нажата
    {
        counter++;
        if (counter == 100) // если счетчик новых клиентов переполнился
            counter = 1;
        for (j = 0; j < 3; j++) // вывод номера нового клиента
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

int main()
{ 
    sei(); // глобальное разрешение прерываний

    initUART();
    DDRA  = 0b01111111; // включение вывода для дисплея
    DDRB  = 0b00000011; // включение вывода для управляющих сигналов дисплея
    PORTB = 0b00000111; // включение подтягивающего резистора на порте 2 (кнопка) и выключение управляющих сигналов дисплея
    DDRD  = 0b00000110; // включение вывода на портах 2 (управление MAX485) и 1 (TX)
    PORTD = 0b00000001; // включение подтягивающего резистора на порте 0 (RX) и режима приема для MAX485

    while(1)
        checkButton(1<<PD2);
}