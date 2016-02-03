/*Project 4 Line Following Robot
 *Authors: Ben Mazur & Alex Addeo
 */

#include <p32xxxx.h>
#include <plib.h>
#include <math.h>

#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_8 

#define Btn1    PORTAbits.RA6

#define S4      PORTBbits.RB10
#define S3	PORTBbits.RB11
#define S2	PORTBbits.RB12
#define S1	PORTBbits.RB13

//JA + JB SSD
#define SegA    PORTEbits.RE0
#define SegB    PORTEbits.RE1
#define SegC    PORTEbits.RE2
#define SegD    PORTEbits.RE3
#define SegE    PORTGbits.RG9
#define SegF    PORTGbits.RG8
#define SegG    PORTGbits.RG7
#define DispSel PORTGbits.RG6

enum Mode {INIT, STRAIGHT, RIGHT, HARD_RIGHT, SOFT_RIGHT, LEFT, HARD_LEFT, SOFT_LEFT, STOP};
enum Mode mode = INIT;

unsigned int lastStraight = 0;
unsigned int distance = 00;

unsigned int ir_sensors = 0b0000;
unsigned int delay = 0;

unsigned int i;

unsigned int timer_increment = 0; // CHANGE
int timer_check = 0;  //CHANGE

void __ISR(_OUTPUT_COMPARE_1_VECTOR, ipl7) OC1_IntHandler (void)
{
    //OC1RS=0x0136;
    IFS0CLR = 0x0040; // Clear the OC1 interrupt flag
}

void __ISR(_OUTPUT_COMPARE_2_VECTOR, ipl7) OC2_IntHandler (void)
{
    //OC1RS=0x0136;
    IFS0CLR = 0x0080; // Clear the OC1 interrupt flag
}



void __ISR(_TIMER_5_VECTOR, ipl1) _T5Interrupt(void){
    if(timer_check==1){
        timer_increment++;
    }
    IFS0CLR = 0x100000;
}


void displayDigit (unsigned char sel, unsigned char value){ DispSel = sel;
SegA =value&1;
SegB =(value>>1)&1;
SegC =(value>>2)&1;
SegD =(value>>3)&1;
SegE =(value>>4)&1;
SegF =(value>>5)&1;
SegG =(value>>6)&1;
}

unsigned char SSD_number[]={
0b0111111,    //0
0b0000110,    //1
0b1011011,    //2
0b1001111,    //3
0b1100110,    //4
0b1101101,    //5
0b1111101,    //6
0b0000111,    //7
0b1111111,    //8
0b1101111,    //9
0b0000000,    //clear
};


void showNumber(int display_value){
    int i;
    displayDigit ( 0, SSD_number[display_value%10] );
    for (i = 0; i < 1500; i++);
    displayDigit ( 1, SSD_number[display_value/10] ); 
    for (i = 0; i < 2000; i++);
}

int readADC()
{
    AD1CON1bits.SAMP = 1;
    for (TMR1=0; TMR1<100; TMR1++);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    return ADC1BUF0;
}

int upperlimit = 322;
int lowerlimit = 155;

void stop(){
    OC1RS = 230; // Initialize secondary Compare Register
    OC2RS = 230;
}
void straight(){
    OC2RS = 155;
    OC1RS = 258;
}
void hardright(){
    OC1RS = 258; // Initialize secondary Compare Register
    OC2RS = 258;
}
void hardleft(){
    OC1RS = 155; // Initialize secondary Compare Register
    OC2RS = 155;
}
void left(){
    OC1RS = 233; // Initialize secondary Compare Register
    OC2RS = 155;
}
void right(){
    OC1RS = 312; // Initialize secondary Compare Register
    OC2RS = 233;
}


void main (void){

//    mic configuration
    AD1PCFG = 0xFFFD;//set RB1 as analog other digital
    AD1CON1 = 0x0; // automatic conversion after sampling
    AD1CHS = 0x00010000; // Connect RB11/AN11 as CH0 input
    AD1CSSL = 0; // no scanning required
    AD1CON2 = 0; // use MUXA, AVss/AVdd used as Vref+/-
    AD1CON3 = 0x1f02; // Tsamp = 128 x Tpb, Sample time = 31 Tad
    AD1CON1bits.ADON = 1;// turn on the ADC

    //TRISA = 0xFFFF;//set port out put
    TRISB = 0x3C00;//set port out put
    TRISE = 0x0000;//set port out put
    TRISF = 0x0104;//set port out put
    TRISG = 0x0000;//set port out put
    TRISD = 0xC000;//set port out put
    TRISC = 0x0000;//set port out put

    //PORTB = 0x0000;//set port output to 0
    PORTE = 0x0000;//set port output to 0
    PORTF = 0x0104;//set port output to 0
    PORTG = 0x0000;//set port output to 0
    PORTD = 0xC000;//set port output to 0
    PORTC = 0x0000;//set port output to 0


    TRISA = 0x50;
   //PORTD = 0x0;
    //TRISD = 0;

    INTDisableInterrupts();

    PR2 = 0x0A2B; //60Hz
    //PR2 = 3124; //50Hz

    OC1CON = 0x0000;
    OC1R = 230; //was 233
    OC1RS = 230;//was 233, 
    OC1CON = 0x0006;
    IFS0CLR = 0x00000040;
    IEC0SET = 0x00000040;
    IPC1SET = 0x001C0000;
    OC1CONSET = 0x8000;

    OC2CON = 0x0000;
    OC2R = 230; //was 233
    OC2RS = 230;// was 233, 
    OC2CON = 0x0006;
    IFS0CLR = 0x00000080;
    IEC0SET = 0x00000080;
    IPC2SET = 0x001C0000;
    OC2CONSET = 0x8000;

    TMR2 = 0;
    T2CON = 0x0060;
    T2CONSET = 0x8000;

    
    T5CONbits.ON = 0; // Stop timer, clear control registers
    TMR5 = 0; // Timer counter
    PR5 = 0x9869; //Timer count amount for interupt to occur - 2048Hz frequency
    IPC5bits.T5IP = 1; //prioirty 5
    IFS0bits.T5IF = 0; // clear interrupt flag
    T5CONbits.TCKPS = 0; // prescaler at 1:256, internal clock sourc
    T5CONbits.ON = 1; // Timer 5 module is enabled
    IEC0bits.T5IE = 1; //enable Timer 5

    INTEnableSystemMultiVectoredInt();



    //unsigned short mode = 0;
    //change the name of this
    int adcstuff = 0;

    


    for(i=0; i < 200000; i++);

    while (1){

        //change sensors
        ir_sensors = ((S1 << 3) | (S2 << 2) | (S3 << 1) | (S4));

        //CURRENT STATE LOGIC
        if(mode == INIT)
        {
            distance = 00;
            stop();
            //showNumber(00);
        }

        else if(mode == STRAIGHT)
        {
            straight();
            distance++;
            //showNumber((distance/200)%100);
        }

        else if(mode == LEFT)
        {
            left();
            //lastStraight = 0;
            distance++;
            delay = 0;
            //showNumber((distance/200)%100);
        }

        else if(mode == RIGHT)
        {
            right();
            //lastStraight = 0;
            distance++;
            delay = 0;
            //showNumber((distance/200)%100);
        }

        else if(mode == HARD_RIGHT)
        {
            hardright();
            delay = 0;
            //lastStraight = 0;
        }

        else if(mode == HARD_LEFT)
        {
            hardleft();
            delay = 0;
            //lastStraight = 0;
        }

        else if(mode == STOP)
        {

        }


        //NEXT STATE LOGIC
        if(mode == INIT)
        {
            if(Btn1 || (readADC() > 970))
            {
                mode = STRAIGHT;
                //straight();
            }
        }

        else if(mode == STRAIGHT)
        {
            if(ir_sensors == 0b1100)
            {
                mode = RIGHT;
            }
            else if(ir_sensors == 0b0011)
            {
                mode = LEFT;
            }
            else if(ir_sensors == 0b0001)
            {
                mode = HARD_LEFT;
            }
            else if(ir_sensors == 0b1000)
            {
                mode = HARD_RIGHT;
            }
            else if(ir_sensors == 0b0000)// && lastStraight == 1)
            {
                delay++;
                mode = STOP;
            }
        }

        else if(mode == LEFT)
        {
            if(ir_sensors == 0b1001)
            {
                mode = STRAIGHT;
            }
            else if(ir_sensors == 0b1100)
            {
                mode = RIGHT;
            }
            else if(ir_sensors == 0b0001)
            {
                mode = HARD_LEFT;
            }
            else if(ir_sensors == 0b1000)
            {
                mode = HARD_RIGHT;
            }
            else if(ir_sensors == 0b0000)// && (lastStraight == 1))
            {
                delay++;
                mode = STOP;
            }
        }

        else if(mode == RIGHT)
        {
            if(ir_sensors == 0b1001)
            {
                mode = STRAIGHT;
            }
            else if(ir_sensors == 0b0011)
            {
                mode = LEFT;
            }
            else if(ir_sensors == 0b1000)
            {
                mode = HARD_RIGHT;
            }
            else if(ir_sensors == 0b0001)
            {
                mode = HARD_LEFT;
            }
            else if(ir_sensors == 0b0000)// && (lastStraight == 1))
            {
                delay++;
                mode = STOP;
            }
        }
        //might have to change the hard_right and hard_left stuff in these next two

        else if(mode == HARD_RIGHT)
        {
            if(ir_sensors == 0b1001)
            {
                mode = STRAIGHT;
            }
            else if(ir_sensors == 0b0001)
            {
                mode = HARD_LEFT; //might need to flip hard left and left
            }
            else if(ir_sensors == 0b0011) //flipped from 1100 to 0011
            {
                mode = LEFT;
            }
            else if(ir_sensors == 0b1000)
            {
                mode = RIGHT;
            }
            else if(ir_sensors == 0b0000)// && (lastStraight == 1))
            {
                delay++;
                mode = STOP;
            }
        }
        else if(mode == HARD_LEFT)
        {
            if(ir_sensors == 0b1001)
            {
                mode = STRAIGHT;
            }
            else if(ir_sensors == 0b1000)
            {
                mode = HARD_RIGHT; //might need to flip hard right and right
                delay = 0;
            }
            else if(ir_sensors == 0b1100) //flipped to 1100 from 0011
            {
                mode = RIGHT;
                delay = 0;
            }
            else if(ir_sensors == 0b0001)
            {
                mode = LEFT;
                delay = 0;
            }

            else if(ir_sensors == 0b0000)// && (lastStraight == 1))
            {
                delay++;
                mode = STOP;
            }
        }


        else if(mode == STOP)
        {
            if(delay >= 100)
            {
                stop();
                mode = INIT;
                delay = 0;
            }

            else{
                mode = STRAIGHT;
            }
           
        }
        showNumber((distance/100)%100);
         
    }
}

