/*  
   LCD16x2 8 bit 8051 interface
   http://www.electronicwings.com
*/


#include<reg51.h>

sfr lcd_data_port=0x90;		/* P1 port as data port */
sbit rs=P2^0;			/* Register select pin */
sbit rw=P2^1;			/* Read/Write pin */
sbit en=P2^2;			/* Enable pin */


void delay(unsigned int count)  /* Function to provide delay Approx 1ms */
{
	int i,j;
	for(i=0;i<count;i++)
	for(j=0;j<112;j++);
}

void LCD_Command (unsigned char cmd)  /* LCD16x2 command funtion */
{
	lcd_data_port= cmd;
	rs=0;			/* command reg. */
	rw=0;			/* Write operation */
	en=1; 
	delay(1);
	en=0;
	delay(5);
}

void LCD_Char (unsigned char char_data)  /* LCD data write function */
{
	lcd_data_port=char_data;
	rs=1;			/* Data reg.*/
	rw=0;			/* Write operation*/
	en=1;   				
	delay(1);
	en=0;
	delay(5);
}

void LCD_String (unsigned char *str) /* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)  /* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);  /* Call LCD data write */
	}
}

void LCD_String_xy (char row, char pos, char *str)  /* Send string to LCD function */
{
	if (row == 0)
	LCD_Command((pos & 0x0F)|0x80);
	else if (row == 1)
	LCD_Command((pos & 0x0F)|0xC0);
	LCD_String(str);	/* Call LCD string function */
}

void LCD_Init (void)		/* LCD Initialize function */
{	
	delay(20);		/* LCD Power ON Initialization time >15ms */
	LCD_Command (0x38);	/* Initialization of 16X2 LCD in 8bit mode */
	LCD_Command (0x0C);	/* Display ON Cursor OFF */
	LCD_Command (0x06);	/* Auto Increment cursor */
	LCD_Command (0x01);	/* clear display */
	LCD_Command (0x80);	/* cursor at home position */
}

void main()
{

	LCD_Init();		/* initialization of LCD*/

	LCD_String("ElectronicWINGS");  /* write string on 1st line of LCD*/
	LCD_Command(0xC0);
	LCD_String("Hello World");  /*write string on 2nd line*/

	while(1);		/* Infinite loop. */
	
}