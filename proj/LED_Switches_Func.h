/////////////////////

#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define LCD_DATA GPIOB 
#define LCD_CTRL GPIOA  
#define RS 0x20 /* PORTA BIT5 mask */
#define RW 0x40 /* PORTA BIT6 mask */
#define EN 0x80 /* PORTA BIT7 mask */
#define KEYPAD_ROW GPIOE
#define KEYPAD_COL GPIOC 
#define Sign 0
#define Number 0x1
#define EqSign 0x2




void getSwitchesInput(unsigned long *SW1,unsigned long *SW2,unsigned char *PressDetctor);
void PortF_Init(void);
void PulseLED(int color);



