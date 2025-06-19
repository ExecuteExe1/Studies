
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

#define BUFF_SIZE 128 // Read Buffer Length

// Additional Define Instructions
unsigned int press=0;
Queue Rx_Queue; // Queue For Storing Received Characters
Pin LED_PIN = PA_5; // LED Pin
Pin BUTTON_PIN = PC_13; // Button pin
unsigned int ButtonCount = 0;
unsigned int Tick = 0;
uint32_t Current_Index = 0;
bool isLEDFrozen = false;

// Interrupt Service Routine For UART Receive
void UART_Rx_ISR(uint8_t Rx)
{
	// Check If The Received Character Is A Printable ASCII Character
	if ((Rx >= 0x0) && (Rx <= 0x7F))
	{
		// Store The Received Character
		queue_enqueue(&Rx_Queue, Rx);
	}
}

void Timer_ISR()
{
	Tick++;
}

void LED_Blink(char* Buff, uint32_t Buff_Index)
{
	char Buff2[BUFF_SIZE];
	sprintf(Buff2, "[TIMER_ISR] Analyzing Buffer <%s> ...\r\n", Buff);
	uart_print(Buff2);
	
		for (Current_Index = 0; Current_Index < (Buff_Index - 1); Current_Index++)
		{Tick=0;
			 if (!queue_is_empty(&Rx_Queue))
		 {
			uart_print("[INTERRUPT] New Input Detected. Aborting Analysis...\r\n");
			return;
	  	}
			
			if (((int)(Buff[Current_Index]) <48) || ((int)(Buff[Current_Index]) > 57))
				continue;
			else if (((int)(Buff[Current_Index]) % 2) == 0)
			{ 
				sprintf(Buff2, "[TIMER_ISR] Current Digit <%c> Is EVEN ...\r\n", Buff[Current_Index]);
				uart_print(Buff2);
		while (Tick <= 5)
			{
				if (!isLEDFrozen)
				{
					if (Tick == 0 || Tick == 2 )  // Question  for lab || Tick==4)
						gpio_toggle(LED_PIN);
				}
				if (!queue_is_empty(&Rx_Queue))
				{
					uart_print("[INTERRUPT] New Input Detected During EVEN Blink. Aborting...\r\n");
					return;
				}
				__WFI();
			}
		}
			
			else if ((int)(Buff[Current_Index]) == 45)
			{
				Current_Index = 0;
			}
			else
			{
				sprintf(Buff2, "[TIMER_ISR] Current Digit <%c> Is ODD ...\r\n", Buff[Current_Index]);
				uart_print(Buff2);
				while (Tick <= 5)
			{
				if (!isLEDFrozen && Tick == 0)
				{
					gpio_toggle(LED_PIN);
				}
				if (!queue_is_empty(&Rx_Queue))
				{
					uart_print("[INTERRUPT] New Input Detected During ODD Blink. Aborting...\r\n");
					return;
				}
				__WFI();
			}
		}
	}
}

void Button_ISR()
{
	press++;
	 if((press%2)==0){
		isLEDFrozen=false;
	 }
	 else{
		 isLEDFrozen=true;
		 }
	char Buff2[BUFF_SIZE];
	sprintf(Buff2, "\nInterrupt: Button Pressed. LED Locked. Count = %u\r\n", ++ButtonCount);
	uart_print(Buff2);
}

int main(void)
{
		
	// Variables To Help With UART Read
	uint8_t Rx_Char = 0;
	char Buff[BUFF_SIZE]; // The UART Read String Will Be Stored Here
	uint32_t Buff_Index;
	
	// Initialize The Receive Queue And UART
	queue_init(&Rx_Queue, 128);
	uart_init(115200);
	uart_set_rx_callback(UART_Rx_ISR); // Set The UART Receive Callback Function
	uart_enable(); // Enable UART Module
	
	leds_init(); // Initialize LEDs
	leds_set(0, 0, 0); // Set LEDs (RGB)
	
	// __enable_irq(); // Enable Interrupts
	
	uart_print("\r\n"); // Print A New Line
	
	// Timer & GPIO Initiation
	
	gpio_set_mode(LED_PIN, Output); // Set GPIO Mode To LD2 (User LED) As The Output
	gpio_set_mode(BUTTON_PIN, Input); // Set GPIO Mode of BTN2 (User Button) As The Input
	gpio_set_mode(BUTTON_PIN, PullUp); // Set GPIO Mode of BTN2 (User Button) As The Input
	gpio_set_callback(BUTTON_PIN, Button_ISR); // Set Button ISR Callback Function
	gpio_set_trigger(BUTTON_PIN, Rising); // Set Button Trigger Mode To Positive Edge
	
	__enable_irq(); // Enable Interrupts
	
	while (1)
	{

		// Prompt The User To Enter Their Designated Numeric Value
		uart_print("Enter Your Designated Numeric Value: ");
		Buff_Index = 0; // Reset Buffer Index
		
		do
		{
			// Wait Until A Character Is Received In The Queue
			while (!queue_dequeue(&Rx_Queue, &Rx_Char))
				__WFI(); // Wait For Interrupt (Sleep)

			// Handle Backspace Character
			if (Rx_Char == 0x7F)
			{
				if (Buff_Index > 0)
				{
					Buff_Index--; // Move Buffer Index Back
					uart_tx(Rx_Char); // Send Backspace Character To Erase On Terminal
				}
			}
			else
			{
				// Store And Echo The Received Character Back
				Buff[Buff_Index++] = (char)Rx_Char; // Store Character In Buffer
				uart_tx(Rx_Char); // Echo Character Back To The Terminal
			}
		} while ((Rx_Char != '\r') && (Buff_Index < BUFF_SIZE)); // Continue Until Enter Key Is Pressed, Or The Buffer Is Full
		
		// Replace The Last Character With Null Terminator To Make It A Valid C String
		Buff[(Buff_Index - 1)] = '\0';
		uart_print("\r\n"); // Print A New Line
		
		// Check If A Buffer Overflow Has Occurred
		if (Buff_Index >= BUFF_SIZE)
			uart_print("[ERROR] Buffer Overload!\r\n");
		else
{
	bool isLoop = (Buff[Buff_Index - 2] == '-'); // Check if second-to-last char is '-'
 
	timer_init(CLK_FREQ / 100); // Set Tick Speed To 100 ms
	timer_set_callback(Timer_ISR); // Assign Timer Interrupt Service Routine
	timer_enable();

	do {
		LED_Blink(Buff, Buff_Index);

		// Check if new character has been typed
		if (!queue_is_empty(&Rx_Queue))
		{
			uart_print("[LOOP_EXIT] New Input Detected. Restarting...\r\n");
			break;
	   	}
	  } while (isLoop);

	  timer_disable();
  }
  }
}