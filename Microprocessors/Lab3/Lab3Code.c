#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "delay.h"
#include "gpio.h"
#include "leds.h"
#include "platform.h"
#include "queue.h"
#include "timer.h"
#include "uart.h"
#define BOUD_RATE 115200
#define BUFF_SIZE 128
#define PASSWORD "1234"
#define AEM_LENGTH 8
#define LED_PIN PB_5
#define SENSOR_PIN PA_0

//actual touch sensor
//#define TOUCH_SENSOR_PIN PB_0
//using the user button cause my button is not connecting ffs
unsigned int PanicCounter = 0;
unsigned int LastDigit=0,SecondLastDigit=0;
Pin BUTTON_PIN = PC_13; //change this to our touch button
Queue Rx_Queue;
uint8_t Temperature, Humidity;
unsigned int Tick = 0;
unsigned int TouchCount = 0;
unsigned int LastDigitSum=0;
unsigned int TickInterval = 5;
char SensorBuffer[64];
uint32_t Current_Index = 0;
uint8_t Sensor_Data[5];
bool InMode=false;
bool ModeA=true;
unsigned int times=0;
bool trigger=false;
typedef enum {    //modes that we will be using
	Show_Both = 0,
	Show_Temperature,
	Show_Humidity
} DisplayMode;

DisplayMode display_mode = Show_Both; //set our starting display mode


void Timer_ISR()
{ 
	Tick++;          //counter for interrupt
}


void Handle_Menu_Command(uint8_t cmd)   
{
	switch (cmd) {                       //depending on the Rx_char we will have as an input
		case 'a':
			if (TickInterval > 2)          //if user presses the first option the frequence of the print is faster {freq increases} also we can NOT go lower than 2seconds
				TickInterval--;                   
			uart_print("\r\n[INFO] Reading interval decreased.\r\n");  //debugging messages
			break;
		case 'b':
			if (TickInterval < 10)         //if user presses the second option  the frequence of the print is slower {freq decreases} also we can NOT go higher than 10seconds
				TickInterval++;
			uart_print("\r\n[INFO] Reading interval increased.\r\n");  
			break;
		case 'c':
			display_mode = (display_mode + 1) % 3;   //we have up to 3 modes
			switch (display_mode) {            //mode switching depending on our display_mode
				case Show_Both: uart_print("\r\n[INFO] Display: Temperature & Humidity\r\n"); break;  //each time we press the third option we get a different mode,this case is for the case 0				
				case Show_Temperature: uart_print("\r\n[INFO] Display: Temperature only\r\n"); break; // case 1
				case Show_Humidity: uart_print("\r\n[INFO] Display: Humidity only\r\n"); break; //case 2
			}
			break;
		case 'd':{  //shows status whenever the 4th option is selected
			char status[64];
			sprintf(status, "Current TickInterval: %u seconds\r\n", TickInterval);  //TickIntervals for debugging
			uart_print(status);
			sprintf(status,"Times the profile has changed: %u \r\n",TouchCount); //How many times we pressed the button
			uart_print(status);
			if(ModeA==true){                //boolean to check if our mode is on alert mode or not
				uart_print("Mode A\r\n");
			}
			else if(ModeA==false){
				uart_print("Mode B\r\n");     //when ModeA=flase then we have the ModeB active
			}
			switch (display_mode) {        //displays the mode depending on WHICH one is selected {case of Display mode}
				case Show_Both:
					uart_print("Display Mode: Temperature & Humidity\r\n");
					break;
				case Show_Temperature:
					uart_print("Display Mode: Temperature only\r\n");
					break;
				case Show_Humidity:
					uart_print("Display Mode: Humidity only\r\n");
					break;
			}
			break;
		}
		
	}
}


void Handle_Full_Command(char *cmd)
{
    if (strcmp(cmd, "status") == 0) {
        char buffer[128];
        uart_print("\r\n[STATUS]\r\n");
        // Mode A or B
        if (ModeA)
            uart_print("Current Mode: Mode A\r\n");
        else
            uart_print("Current Mode: Mode B\r\n");
        // Last measurements
        sprintf(buffer, "Last Temperature: %d°C\r\n", Temperature);
        uart_print(buffer);
        sprintf(buffer, "Last Humidity: %d%%\r\n", Humidity);
        uart_print(buffer);

        // Profile changes count
        sprintf(buffer, "Profile changes (TouchCount): %u\r\n", TouchCount);
        uart_print(buffer);
    } else if (strlen(cmd) == 1) {
        // For single char commands (a,b,c,d) //if we press a,b,c,d we use the regular menu options on the switch case above
        Handle_Menu_Command(cmd[0]);
    } else {
        uart_print("\r\n[ERROR] Unknown command.\r\n");  //error
    }
}


void UART_Rx_ISR(uint8_t Rx)
{
	// Check If The Received Character Is A Printable ASCII Character   
	if ((Rx >= 0x0) && (Rx <= 0x7F))
	{
		// Store The Received Character
		queue_enqueue(&Rx_Queue, Rx);
		
	}
}



void Menu_SetUp()  //Menu Visuals
{
	uart_print("======Menu=======\r\n");
	uart_print("Options:\r\n");
	uart_print("A.Press a for increase of reading and printing frequency by 1s\r\n");
	uart_print("B.Press b for decrease of reading and printing frequency by 1s\r\n");
	uart_print("C.Press c for change between Temperature/Humidity/or display of both\r\n");
	uart_print("D.Press d for print of the latest values and status\r\n");
	uart_print("Insert your desired option: ");
	uart_print("\n\r");
}


 //Sensor set/up 
void Sensor_Start_Signal() 
{
	gpio_set_mode(SENSOR_PIN, Output); // we need to set it as an output {opws k se ena ledaki}
  gpio_set(SENSOR_PIN, 1); // pull high initially
  delay_ms(1);        
  gpio_set(SENSOR_PIN, 0); // pull low for at least 18ms
  delay_ms(20);           // evala delay gia testing/debugging
  gpio_set(SENSOR_PIN, 1); // pull high for 20-40us
  delay_us(30);
  gpio_set_mode(SENSOR_PIN, Input);
}
 //checks if the sensor is responsive
bool Sensor_Check_Response()
{
	delay_us(40);
  if (gpio_get(SENSOR_PIN) == 0)
	{
		delay_us(80);
    if (gpio_get(SENSOR_PIN) == 1)
		{
			delay_us(50);
      return true;
    }
   }
   return false;
}

//reads sensor byte by byte
uint8_t Sensor_Read_Byte()
{
	uint8_t Byte = 0;
  for (uint8_t i = 0; i < 8; i++)
	{
		while (!gpio_get(SENSOR_PIN)); // wait for low
    delay_us(30);                  // wait to read bit value
    if (gpio_get(SENSOR_PIN))
			Byte |= (1 << (7 - i));
    while (gpio_get(SENSOR_PIN));      // wait for high to finish
  }
  return Byte;
}
 

bool Sensor_Read(uint8_t *Humidity, uint8_t *Temperature)
{

		Sensor_Start_Signal(); //starts the signal of the sensor
		if (!Sensor_Check_Response()) //if the sensor is NOT responsive returns false
			return false;
		for (unsigned int i = 0; i < 5; i++) //if sensor is responsive we continue 
			Sensor_Data[i] = Sensor_Read_Byte();
		uint8_t Checksum = Sensor_Data[0] + Sensor_Data[1] + Sensor_Data[2] + Sensor_Data[3];
		if (Checksum != Sensor_Data[4]) 
			return false;
		*Humidity = Sensor_Data[0]; 
		*Temperature = Sensor_Data[2];
		return true;

}
//Our Sensor Reading and Printing depending on the mode and display
void ReadSensorAndPrint() { 
	if ((Tick % TickInterval) == 0) {   //our starting frequency is every 5seconds
		if (Sensor_Read(&Humidity, &Temperature)) {
			char SensorBuffer[64];   //we need a Buffer to be able to print the values we get from the sensor

			// === PANIC RESET CHECK ===  //3 serial values over the limits result into a reset
			if ((Temperature > 35) || (Humidity > 80)) {
				PanicCounter++;   //counts the values over the limit
			} else {
				PanicCounter = 0;  //resets them to 0
			}

			if (PanicCounter >= 3) {
				uart_print("\r\n[PANIC RESET] System exceeded safe limits 3 times. Performing software reset...\r\n");
				NVIC_SystemReset();  // This will perform a soft reset
			}

			// === NORMAL DISPLAY ===
			switch (display_mode) {
				case Show_Both:
					sprintf(SensorBuffer, "Temperature: %d°C, Humidity: %d%%\r\n", Temperature, Humidity);
					break;
				case Show_Temperature:
					sprintf(SensorBuffer, "Temperature: %d°C\r\n", Temperature);
					break;
				case Show_Humidity:
					sprintf(SensorBuffer, "Humidity: %d%%\r\n", Humidity);
					break;
			}
			uart_print(SensorBuffer);

			if (!queue_is_empty(&Rx_Queue)) { //can have an input of the menu any time
				InMode = false;
				return;
			}
		}
		__WFI();	
	}
}

	void Button_ISR()  //button interrupt
{
    TouchCount++;
    uart_print("Button Pressed\r\n");  //debugging message
        times = 0;
    // Toggle modes
    ModeA = !ModeA;

   
    if ((TouchCount % 3) == 0) {  //every 3rd press we get the TickInterval=1stlastaemdigit +2ndlastaemdigit
         TickInterval = LastDigitSum;   
        Tick = 0;  // Reset Tick so we can synchronise again
        uart_print("Tick Interval changed\r\n");
    }
}
//mode B Alert mode
		void ModeBInitialization() {
    ReadSensorAndPrint();

    if ((Temperature > 30) || (Humidity > 60)) {
		   	times=0;
        gpio_set(LED_PIN,1);
			  gpio_set(LED_PIN,0);
			  trigger=true; //we need a boolean to know when we enter in these conditions so that our led can keep blinking for 5 reads AFTER entering here
			 
    }else if(((Temperature < 30) || (Humidity < 60)) && (trigger==false)){  //we read without our led blinking cause we never entered above due to trigger being false
      gpio_set(LED_PIN,0);
    }
		else if(((Temperature < 30) || (Humidity < 60)) && (trigger==true))  
		{ if(times<5){
		  	times++;
			 gpio_set(LED_PIN,1);   //delays for synchronisation
		  	delay_ms(500);  
			 gpio_set(LED_PIN,0);
			 delay_ms(500);
		}
		 else if(times==5){
			 gpio_set(LED_PIN,0);
			 trigger=false;  //we reset the trigger for future uses of alert mode B again
	}
}
		}



int main(void)
{   char cmd_buffer[32];  //for the status
    uint8_t cmd_index = 0; //same
    uart_init(BOUD_RATE);
    uart_enable();
    uint8_t Rx_Char = 0;
    char Buff[BUFF_SIZE];
    uint32_t Buff_Index;

    queue_init(&Rx_Queue, 128);
    uart_set_rx_callback(UART_Rx_ISR);

    leds_init();
    leds_set(0, 0, 0);

    __enable_irq();
    gpio_set_mode(LED_PIN, Output);
	  gpio_set_mode(BUTTON_PIN, Input); // Set GPIO Mode of BTN2 (User Button) As The Input
  	gpio_set_mode(BUTTON_PIN, PullUp); // Set GPIO Mode of BTN2 (User Button) As The Input
  	gpio_set_callback(BUTTON_PIN, Button_ISR); // Set Button ISR Callback Function
	  gpio_set_trigger(BUTTON_PIN, Rising); // Set Button Trigger Mode To Positive Edge
	

    // === Password Entry ===
    bool password_correct = false;
do {
        uart_print("Please Enter The Password: ");
        Buff_Index = 0;

        do {
            while (!queue_dequeue(&Rx_Queue, &Rx_Char)) __WFI();

            if (Rx_Char == 0x7F) {
                if (Buff_Index > 0) {
                    Buff_Index--;
                    uart_tx('\b'); uart_tx(' '); uart_tx('\b');
                }
            } else {
                if (Buff_Index < BUFF_SIZE - 1) {
                    Buff[Buff_Index++] = (char)Rx_Char;
                    uart_tx(Rx_Char);
                }
            }
        } while ((Rx_Char != '\r') && (Buff_Index < BUFF_SIZE));

        if (Buff_Index > 0 && (Buff[Buff_Index - 1] == '\r' || Buff[Buff_Index - 1] == '\n')) {
            Buff[--Buff_Index] = '\0';
        } else {
            Buff[Buff_Index] = '\0';
        }

        uart_print("\r\n");
        if (strcmp(Buff, PASSWORD) == 0) {
            password_correct = true;
        }
    } while (!password_correct);   //loops until the password is correct

    // === AEM Entry ===
    uart_print("Enter your AEM: ");
    Buff_Index = 0;
 do {
        while (!queue_dequeue(&Rx_Queue, &Rx_Char)) __WFI();

        if (Rx_Char == 0x7F) {
            if (Buff_Index > 0) {
                Buff_Index--;
                uart_tx('\b'); uart_tx(' '); uart_tx('\b');
            }
        } else if (Rx_Char != '\r') {
            if (Buff_Index < BUFF_SIZE - 1) {
                Buff[Buff_Index++] = (char)Rx_Char;
                uart_tx(Rx_Char);
            }
        }
    } while ((Rx_Char != '\r') && (Buff_Index < AEM_LENGTH)); //reads and saves aem
		 
		
    Buff[Buff_Index] = '\0'; 
		int length = strlen(Buff);   //get the lenght of the buff
		LastDigit=Buff[length - 1] - '0'; //get the last digit
    SecondLastDigit=Buff[length - 2] - '0'; //get the second last
		LastDigitSum=LastDigit+SecondLastDigit; //sum
		
    uart_print("\r\nAEM Received: "); //print the aem for debugging reasons
    uart_print(Buff);
    uart_print("\r\n");
   
    Menu_SetUp();  //our menu display
    Handle_Menu_Command(Rx_Char);  //read and printing
		
while(1) { 
    timer_init(CLK_FREQ/10);  //timer set up ever 1s
    timer_set_callback(Timer_ISR);
    timer_enable(); //enable

    while (true) {
        if (queue_dequeue(&Rx_Queue, &Rx_Char)) {  //uart interrupt
            if (Rx_Char == '\r' || Rx_Char == '\n') {  //
                if (cmd_index > 0) {  //string needs to be !=0 
                    cmd_buffer[cmd_index] = '\0'; // null-terminate string
                    Handle_Full_Command(cmd_buffer); //status function that accepts only char *cmd
                    cmd_index = 0; // reset buffer index for next command
                    uart_print("Insert your desired option: \r\n");
                }
            } else if (Rx_Char == 0x7F) { // backspace
                if (cmd_index > 0) {
                    cmd_index--;
                    uart_tx('\b');
                    uart_tx(' ');
                    uart_tx('\b');
                }
            } else { //normal input
                if (cmd_index < sizeof(cmd_buffer) - 1) {
                    cmd_buffer[cmd_index++] = (char)Rx_Char;
                    uart_tx(Rx_Char); // echo
                }
            }
        } else {
            if (ModeA) {
                ReadSensorAndPrint();
            } else {
                ModeBInitialization();
            }
        }
    }
}
}