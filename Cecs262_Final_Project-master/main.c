#include <reg51.h>

void LCD_Init (void);
void delay(unsigned int count);
void LCD_Command (unsigned char cmd);
void LCD_Char (unsigned char char_data);
void LCD_String (unsigned char *str);
void Counter(unsigned short int lengthSeconds, unsigned char displayCount);

sfr lcd_data_port=0x90; //use P1 for lcd
sbit rs=P2^6;//reg select for lcd
sbit rw=P2^7; //read write for lcd
sbit en=P3^7; //enable for lcd

//sbits for lights
sbit ewG = P2^0;
sbit ewY = P2^1;
sbit ewR = P2^2;
sbit nsG = P2^3;
sbit nsY = P2^4;
sbit nsR = P2^5;
sbit lcdBacklight = P3^6;

//globals
unsigned char walkState = 0;
unsigned char trafficSign1[] = "6 feet apart";
unsigned char trafficSign2[] = "Or 6 feet under";

void main()
{
	unsigned char state = 0;
	IE = 0x81; //enable external interupt P3.2
	LCD_Init(); 
	
	while(1){
		//set lights to all off
		ewG = 0;
		ewY = 0;
		ewR = 0;
		nsG = 0;
		nsY = 0;
		nsR = 0;
		
		switch(state){
			case 0x00: //East West is green
			{
				nsY = 0; //turn off N-S yellow light from state 3
				ewR = 0; //turn off E-W red light from state 2
				ewG = 1; //turn on E-W green light
				nsR = 1; //turn on N-S red light
				
				if(walkState != 0){ //walk is on
					//display count down
					Counter(0x06, walkState);
					walkState = 0xff; //set walk state for yellow light
				}
				else{ //walk is off
					//display road sign
					Counter(0x06, 0x00);
				}
				state++; //next state
			}
			case 0x01: //East West is Yellow
			{
				ewG = 0; //turn off E-W green light from state 0
				ewY = 1; //turn on E-W yellow light
				
				if(walkState == 0xff){ //walk is on (from green light)
					//flash
					Counter(0x03, walkState);
					walkState = 0; //walk procedure done, reset walk state
				}
				else{ //walk is off
					//display road sign
					Counter(0x03, 0x00);
				}
				state++; //next state
			}
			case 0x02: //North South is Green
			{
				
				nsR = 0; //turn off N-S red light from state 0
				ewY = 0; //turn off E-W yellow light from state 1
				nsG = 1; //turn on N-S green light
				ewR = 1; //turn on E-W red light
				
				if(walkState != 0){ //walk is on
					//display count down
					Counter(0x06, walkState);
					walkState = 0xff; //set walk state for yellow light
				}
				else{ //walk is off
					//display road sign
					Counter(0x06, 0x00);
				}
				state++; //next state
			}
			case 0x03: //North South is Yellow
			{
				
				nsG = 0; //turn off N-S green light from state 2
				nsY = 1; //turn on N-S Yellow light
				
				if(walkState == 0xff){ //walk is on (from green light)
					//flash
					Counter(0x03, walkState);
					walkState = 0; //walk procedure done, reset walk state
				}
				else{ //walk is off
					//display road sign
					Counter(0x03, 0x00);
				}
				state = 0; //go to state 0
			}
		}
	}
}

void walk(void) interrupt 0{
	if(walkState == 0x00){
		walkState = 0x01; //set walkState to 1 on interupt
	}
}

void Counter(unsigned short int lengthSeconds, unsigned char walkState)
{
	int counter;
	if (walkState != 0){
		if(walkState == 0x01){
			//count down
			counter = 0;
			while(counter < lengthSeconds*1000) //repeat until total time is reached in ms
			{
				if((counter % 1000) == 0){
					//printByte(lengthSeconds - (counter/1000));
					LCD_Command (0x01);	//clear display
					LCD_Command (0x80); //move cursor to home
					LCD_String("Time to walk: ");
					LCD_Char((lengthSeconds - (counter / 1000)) + 0x30);
				}
				//counter
				TMOD = 0x01; //timer0 mode 1
				TL0 = 0x66; //timer start value
				TH0 = 0xFC; //timer start value
				TR0 = 1; //start timer
				while(~TF0); //wait for flag
				counter++; //increment counter
				TF0 = 0; //reset flag
			}
			TR0 = 0; //stop timer
		}
		else{
			//flash
			counter = 0;
			//flash lcd on-off
			LCD_Command (0x01);	//clear display
			LCD_Command (0x80); //move cursor to home
			LCD_String("Run!!!");
			while(counter < lengthSeconds*1000) //repeat until total time is reached in ms
			{
				if(counter % 200 == 0){
					lcdBacklight = ~lcdBacklight;
				}
				//counter
				TMOD = 0x01; //timer0 mode 1
				TL0 = 0x66; //timer start value
				TH0 = 0xFC; //timer start value
				TR0 = 1; //start timer
				while(~TF0); //wait for flag
				counter++; //increment counter
				TF0 = 0; //reset flag
			}
			TR0 = 0; //stop timer
			lcdBacklight = 0;
		}
	}
	else{
		
		counter = 0;
		//show road sign
		LCD_Command (0x01);	//clear display
		LCD_Command (0x80); //move cursor to home
		LCD_String(trafficSign1); //print first line
		LCD_Command(0xC0); //new line
		LCD_String(trafficSign2); //print second line
		while(counter < lengthSeconds*1000) //repeat until total time is reached in ms
		{
			//counter
			TMOD = 0x01; //timer0 mode 1
			TL0 = 0x66; //starting value for 1ms
			TH0 = 0xFC; //starting value for 1ms
			TR0 = 1; //start timer
			while(~TF0); // wait for flag
			counter++; //increment counter
			TF0 = 0; //reset flag
		}
		TR0 = 0; //stop timer
	}
}

void LCD_Init (void)
{	
	delay(20);		//Wait for power
	lcdBacklight = 0; //set lcd back light to full brightness
	LCD_Command (0x38);	//8bit mode
	LCD_Command (0x0C);	//display cursor off
	LCD_Command (0x06);	//auto increment cursor
	LCD_Command (0x01);	//clear
	LCD_Command (0x80);	//cursor home
}

void delay(unsigned int count)  //approx ~1ms
{
	int i,j;
	for(i=0;i<count;i++)
	for(j=0;j<112;j++);
}

void LCD_Command (unsigned char cmd)
{
	lcd_data_port= cmd;
	rs=0; //command reg
	rw=0; //write
	en=1; //enable
	delay(1); //wait for lcd reg to fill
	en=0; //disable
	delay(5); //wait for command
}

void LCD_Char (unsigned char char_data)
{
	lcd_data_port=char_data; //put char on output to lcd
	rs=1; //data reg
	rw=0; //write
	en=1; //enable		
	delay(1); //wait for lcd reg to fill
	en=0; //disable
	delay(5); //wait for print
}

void LCD_String (unsigned char *str)
{
	int i;
	for(i=0;str[i]!=0;i++)
	{
		LCD_Char (str[i]);
	}
}



