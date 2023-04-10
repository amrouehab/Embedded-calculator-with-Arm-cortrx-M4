
#include "TM4C123GH6PM.h"
#include "LED_Switches_Func.h"
#include "LCD_func.h"
#include "Calculator_Func.h"
#include "KeyPad.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>






void PortF_Init(void) {

  SYSCTL->RCGCGPIO |= 0x20;     // 1) F clock
  GPIOF->LOCK = 0x4C4F434B;   // 2) unlock PortF PF0
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  GPIOF->AMSEL = 0x00;        // 3) disable analog function
  GPIOF->PCTL = 0x00000000;   // 4) GPIO clear bit PCTL
  GPIOF->DIR = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output
  GPIOF->AFSEL = 0x00;        // 6) no alternate function
  GPIOF->PUR = 0x11;          // enable pullup resistors on PF4,PF0
  GPIOF->DEN = 0x1F;          // 7) enable digital pins PF4-PF0
}
void resetTheData(int *DigitNo, char OperandsSavedData[]) {

  memset(OperandsSavedData, 0, sizeof(*OperandsSavedData)); //clear array to avoid data interfernce//reset
  *DigitNo = 0;
  resetDisplay();

}
void writeToScreen( char key) {
  LCD_data(key);      /* display the key label */
  LCD_command(0x6);      /* LCD cursor increment */


}
void pulseLED(int color) {

  GPIOF->DATA = color;
  delayMs(110);
  GPIOF->DATA = 0x00;       // LED is off

}
//this functions suppeosed to handl the switches inputs but there is an error that we cant figure :S
void getSwitchesInput(unsigned long *SW1, unsigned long *SW2, unsigned char *PressDetctor) {

  *SW1 = GPIOF->DATA & 0x10;   // read PF4 into SW1
  * SW2 = GPIOF->DATA & 0x01;   // read PF0 into SW2
  if (*SW1 || *SW2) {      /* if a key is pressed */
    if (!(*PressDetctor == 1)) {
      if (*SW1 && *SW2) {                 // both pressed

      } else {
        if (*SW1 && (!*SW2)) {      // just SW1 pressed
          pulseLED(0x04);
          LCD_command(10);
        } else {
          if ((!*SW1) && *SW2) {          // just SW2 pressed
            LCD_command(14);


          }
        }
      }
      *PressDetctor = 1;
    }

  } else
    *PressDetctor = 0;
}


int main(void)
{
  unsigned char key;
  char OperandsSavedData[16] = {'0', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
  unsigned char KeyIsPressedOnce = 0, CalcDone = 0, SwitchPressDetctor = 0;
  int DigitNo = 0 , res = 0;
  unsigned long *SW1, *SW2; // input from PF4,PF0



  keypad_init();
  LCD_init();
  resetDisplay();
  PortF_Init();
  while (1)
  {
    getSwitchesInput(SW1, SW2, &SwitchPressDetctor);
    key = keypad_getkey();  /* read the keypad */
    if (key != 0) {         /* if a key is pressed */
      if (!(KeyIsPressedOnce == 1)) { //toBlock multipe writing just write 1Key only per one press
        if (DigitNo > 16) {
          resetDisplay();  // LED is red//show over flow error if noDigits>16 digits
          writeCHarData("Error OverfLOW ");
          LCD_command(0xC0);
          writeCHarData("PressAnyToReset");
          GPIOF->DATA = 0x02;
          CalcDone = 1;
          DigitNo = 0;
          delayMs(100);
        }
        else {
          switch (getCharType(key)) {
            case Number : {
                if (CalcDone == 1) //check if any privious calc to reset the program
                {

                  resetTheData(&DigitNo, OperandsSavedData); //reset
                  CalcDone = 0;
                  res = 0;

                }
                OperandsSavedData[DigitNo] = key;
                DigitNo++;
                writeToScreen(key);
                pulseLED(0x04);     // LED is blue

                break;
              }
            case Sign: {
                //handle pressing sign withou entering data
                if (CalcDone == 1) {
                  resetTheData(&DigitNo, OperandsSavedData); //reset
                  sprintf(OperandsSavedData , "%d", res);
                  DigitNo = writeCHarData(OperandsSavedData);
                  DigitNo++;
                  OperandsSavedData[DigitNo] = key;
                  CalcDone = 0;


                }
                OperandsSavedData[DigitNo] = key;
                DigitNo++;
                writeToScreen(key);
                pulseLED(0x06);


                break;
              }
            case EqSign: {
                OperandsSavedData[DigitNo] = key;
                if (DigitNo != 0)res = calc(OperandsSavedData, 0);
                CalcDone = 1;
                GPIOF->DATA = 0x08;       // LED is green
                //free all
                DigitNo = 0;
                memset(OperandsSavedData, 0, sizeof(OperandsSavedData)); //clear array to avoid data interfernce

              }
              break;
          }
          KeyIsPressedOnce = 1;

        }


      }
    }
    else
      KeyIsPressedOnce = 0;
    delayMs(20);        /* wait for a while */
  }
}


void LCD_init(void)
{
  SYSCTL->RCGCGPIO |= 0x01;  /* enable clock to GPIOA */
  SYSCTL->RCGCGPIO |= 0x02;  /* enable clock to GPIOB */

  LCD_CTRL->DIR |= 0xE0;     /* set PORTA pin 7-5 as output for control */
  LCD_CTRL->DEN |= 0xE0;     /* set PORTA pin 7-5 as digital pins */
  LCD_DATA->DIR = 0xFF;      /* set all PORTB pins as output for data */
  LCD_DATA->DEN = 0xFF;      /* set all PORTB pins as digital pins */

  delayMs(20);            /* initialization sequence */
  LCD_command(0x30);//wakeup
  delayMs(5);
  LCD_command(0x30);
  delayUs(100);
  LCD_command(0x30);

  LCD_command(0x38);      /* set 8-bit data, 2-line, 5x7 font */
  LCD_command(0x06);      /* move cursor right */
  LCD_command(0x01);      /* clear screen, move cursor to home */
  LCD_command(0x0F);      /* turn on display, cursor blinking */
}

void LCD_command(unsigned char command)
{
  LCD_CTRL->DATA = 0;     /* RS = 0, R/W = 0 */
  LCD_DATA->DATA = command;
  LCD_CTRL->DATA = EN;    /* pulse E  pin7=1*/
  delayUs(0);
  LCD_CTRL->DATA = 0;
  if (command < 4)
    delayMs(2);         /* command 1 and 2 needs up to 1.64ms */
  else
    delayUs(40);        /* all others 40 us */
}

void LCD_data(unsigned char data)
{
  LCD_CTRL->DATA = RS;    /* RS = 1, R/W = 0 */
  LCD_DATA->DATA = data;
  LCD_CTRL->DATA = EN | RS;   /* pulse E */
  delayUs(0);
  LCD_CTRL->DATA = 0;
  delayUs(40);
}

/* delay n milliseconds (16 MHz CPU clock) */
void delayMs(int n)
{
  int i, j;
  for (i = 0 ; i < n; i++)
    for (j = 0; j < 3180; j++)
    {}  /* do nothing for 1 ms */
}

/* delay n microseconds (16 MHz CPU clock) */
void delayUs(int n)
{
  int i, j;
  for (i = 0 ; i < n; i++)
    for (j = 0; j < 3; j++)
    {}  /* do nothing for 1 us */
}

/* This function is called by the startup assembly code to perform system specific initialization tasks. */
void SystemInit(void)
{
  /* Grant coprocessor access */
  /* This is required since TM4C123G has a floating point coprocessor */
  SCB->CPACR |= 0x00f00000;
}




/* to be used in conjunction with Program 3-2 */






/* this function initializes the ports connected to the keypad */
void keypad_init(void)
{
  SYSCTL->RCGCGPIO |= 0x04;   /* enable clock to GPIOC */
  SYSCTL->RCGCGPIO |= 0x10;   /* enable clock to GPIOE */

  KEYPAD_ROW->DIR |= 0x0F;     /* set row pins 3-0 as output */
  KEYPAD_ROW->DEN |= 0x0F;     /* set row pins 3-0 as digital pins */
  KEYPAD_ROW->ODR |= 0x0F;     /* set row pins 3-0 as open drain */

  KEYPAD_COL->DIR &= ~0xF0;     /* set column pin 7-4 as input */
  KEYPAD_COL->DEN |= 0xF0;     /* set column pin 7-4 as digital pins */
  KEYPAD_COL->PUR |= 0xF0;     /* enable pull-ups for pin 7-4 */
}

/* This is a non-blocking function to read the keypad. */
/* If a key is pressed, it returns the key label in ASCII encoding. Otherwise, it returns a 0 (not ASCII 0). */
unsigned char keypad_getkey(void)
{
  const unsigned char keymap[4][4] = {
    { '1', '2', '3', '+'},
    { '4', '5', '6', '-'},
    { '7', '8', '9', '/'},
    { '=', '0', '^', 'X'},
  };

  int row, col;

  /* check to see any key pressed first */
  KEYPAD_ROW->DATA = 0;               /* enable all rows */
  col = KEYPAD_COL->DATA & 0xF0;      /* read all columns */
  if (col == 0xF0) return 0;          /* no key pressed */

  /* If a key is pressed, it gets here to find out which key. */
  /* Although it is written as an infinite loop, it will take one of the breaks or return in one pass.*/
  while (1)
  {
    row = 0;
    KEYPAD_ROW->DATA = 0x0E;        /* enable row 0 */
    delayUs(2);                     /* wait for signal to settle */
    col = KEYPAD_COL->DATA & 0xF0;
    if (col != 0xF0) break;

    row = 1;
    KEYPAD_ROW->DATA = 0x0D;        /* enable row 1 */
    delayUs(2);                     /* wait for signal to settle */
    col = KEYPAD_COL->DATA & 0xF0;
    if (col != 0xF0) break;

    row = 2;
    KEYPAD_ROW->DATA = 0x0B;        /* enable row 2 */
    delayUs(2);                     /* wait for signal to settle */
    col = KEYPAD_COL->DATA & 0xF0;
    if (col != 0xF0) break;

    row = 3;
    KEYPAD_ROW->DATA = 0x07;        /* enable row 3 */
    delayUs(2);                     /* wait for signal to settle */
    col = KEYPAD_COL->DATA & 0xF0;
    if (col != 0xF0) break;

    return 0;   /* if no key is pressed */
  }

  /* gets here when one of the rows has key pressed */
  if (col == 0xE0) return keymap[row][0]; /* key in column 0 */
  if (col == 0xD0) return keymap[row][1]; /* key in column 1 */
  if (col == 0xB0) return keymap[row][2]; /* key in column 2 */
  if (col == 0x70) return keymap[row][3]; /* key in column 3 */
  return 0;   /* just to be safe */
}
void resetDisplay() {
  LCD_command(0x80);
  LCD_command(0x1);
}
//writes the char array data to the LCD
int writeCHarData(  char Data[]) {
  int i;
  for (i = 0; i < strlen(Data); i++) {
    LCD_data(Data[i]);
  }
  return i;
}

//gets the operand as an interger  from its  char array
int getOP( char Op[]) {
  int op;
  sscanf(Op, "%d", &op);

  return op;

}
//this function takes 2 operands and a sign and return the Result of the operation by this sign
int calculate( int op1, int op2, char sign) {
  int result;
  switch (sign) {
    case '+': result = op1 + op2; break;
    case '-': result = op1 - op2; break;
    case 'X': result = op1 * op2; break;
    case '/': result = op1 / op2; break;
    case '^': result = pow(op1, op2); break;


  }

  return result;
}
//returns the type of this char   numer or sign or an equal sign
unsigned char getCharType(unsigned char Key) {

  if (Key == '+' || Key == '-' || Key == '/' || Key == '^' || Key == 'X')return Sign;
  else if (Key == '=')return (EqSign);
  else return (Number);
}

//this is the main func that takes the Char array inputs of the user and gets from it the data to calculate
int calc(char InputEq[], int StartIndex) {
  int i = StartIndex;
  char op1[14], op2[14], ResultChar[16], SavedOp1[14];
  char sign, key, SavedSign, LastSign = 'n' ;
  int op1Obtained = 0;
  int OP1, OP2;
  short DOne = 0;
  SavedOp1[0] = 'N';
  while (!(DOne == 1)) {
    key = InputEq[i];

    switch (getCharType(InputEq[i])) {

      case Number : {
          if (op1Obtained > 0) op2[i - op1Obtained - 1] = key;
          else op1[i] = key;
          break;
        }
      case Sign: {
          sign = key;
          //if ob1 is obtained for ex will be at this stage 1+2(+)
          if (op1Obtained > 0) {
            //calculate  call eq
            OP2 = getOP(op2);
            memset(op2, 0, sizeof(op2)); //clear op2 array to avoid data interfernce
            //if the current sign is first grade and the previous is second
            if ((sign == 'X' || sign == '/') && (LastSign == '+' || LastSign == '-')) { /////////////
              sprintf(SavedOp1 , "%d", OP1);  //Save Op1

              //save the Last sign
              SavedSign = LastSign;
              OP1 = OP2; //change op2 to op1

              //OP2=calc(InputEq,i);
            } else {
              //if there is a Saved Op ex 1+2X3(+)

              if (SavedOp1[0] != 'N' && LastSign != sign) {
                //get Op2 from the calc of the first grade operation
                OP2 = calculate(OP1, OP2, LastSign);
                //calculate the previos operation and save it as the first operand of the next operation
                OP1 = calculate(getOP(SavedOp1), OP2, SavedSign);
                memset(SavedOp1, 0, sizeof(SavedOp1)); //clear savedOp1 array to avoid data interfernce
                SavedOp1[0] = 'N'; //setFlag ;
              }
              // if there is no saved op (normal behaviour) ex 1+2+3+4
              else OP1 = calculate(OP1, OP2, LastSign);
            }
            //if op1 isnt obtained yet for ex will be at this stage  1(+)
          } else {
            //obtain op1
            OP1 = getOP(op1);
            memset(op1, 0, sizeof(op1)); //clear savedOp1 array to avoid data interfernce

          }//////////////
          LastSign = sign;
          op1Obtained = i;
          break;

        }
      case EqSign: {
          //calculate  call eq
          OP2 = getOP(op2);
          sprintf(ResultChar , "%d", calculate(OP1, OP2, sign));
          if (SavedOp1[0] != 'N') {
            OP2 = calculate(OP1, OP2, sign);
            sprintf(ResultChar , "%d", calculate(getOP(SavedOp1), OP2, SavedSign));
          }

          if (strlen(ResultChar) > 16)writeCHarData("OverFlow  Error  press = toReset"); //show over flow error
          else {
            LCD_command(0xc0);
            writeCHarData(ResultChar);
          }
          DOne = 1;
          break;
        }
    }


    i++;

  }
  return getOP(ResultChar);
}

