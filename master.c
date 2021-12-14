#include <avr/io.h>
#include <util/delay.h>

volatile unsigned char queue[100], //очередь
                       qlen = 0; // длина очереди
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

void initUART(void) // инициализация UART
{
    UBRRL=51; //скорость 9600 Бод при f=8 МГц
    UCSRB=(1<<RXEN)|(1<<TXEN); //разрешение приема и передачи
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

unsigned char recieveUart(void) // прием по UART
{
    while(!(UCSRA&(1<<RXC))) {}; // ожидание приема
    PORTD |= (1<<PD2); // включение передачи
    return UDR;
}

void addToQueue(unsigned char a) // добавление пользователя в очередь
{
    queue[qlen] = a;
    qlen++;
}

void goToMK2(void) // общение с МК2
{
    sendUart(0b10000000);
    unsigned char data = recieveUart();
    if (data != 0b11111111)
        addToQueue(data&0b01111111);
}

unsigned char deleteFromQueue(void) // удаление человека из очереди
{
    unsigned char rett = queue[0];
    unsigned char i = 0;
    while (queue[i] != 0)
    {
        queue[i] = queue[i+1];
        i++;
    }
    qlen--;
    return rett;
}

void goToMK3(void) // общение с МК3
{
    unsigned char i, j, data;

    if (qlen > 0) // если очередь не пуста
        sendUart(queue[0]); // отправка номера следующего в очереди человека
    else
        sendUart(0b01111111); // отправка сигнала, что очередь пуста

    data = recieveUart();

    if (data == 1)
    {
        unsigned char a = deleteFromQueue();
        for (j = 0; j < 3; j++) // вывод вызываемого пользователя
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
    }
}

void outputQueue(void) // вывод очереди
{
    if (qlen > 0)
    {
        if (qlen > 1)
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
        if (qlen > 1)
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

int main()
{
    initUART();
    DDRA  = 0b01111111; // включение вывода для 1 дисплея
    DDRB  = 0b01111111; // включение вывода для 2 дисплея
    DDRC  = 0b00001111; // включение вывода для управляющих сигналов дисплеев
    DDRD  = 0b00000110; // включение вывода на портах 2 (управление MAX485) и 1 (TX)
    PORTD = 0b00000101; // включение подтягивающего резистора на порте 0 (RX) и режима отправки для MAX485

    while(1)
    {
        goToMK2();
        goToMK3();
        outputQueue();
    }
}