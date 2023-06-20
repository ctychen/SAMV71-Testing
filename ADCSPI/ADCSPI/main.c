#include <atmel_start.h>
//#include <asf.h>
#include "variables.h"
#include <stdint.h>
#include "atca_basic.h"
#include "atcac_base64.h"



// Commands
// 0 send bin
// 1 request for time
// 2 send event-by-event

volatile uint8_t data_mode = 1;

#define MESSAGE_LENGTH 13

#define MESSAGE_INTERVAL_MS 30



// Struct for Timer Task
static struct timer_task task;
uint32_t milliCounter = 0;
uint32_t secondCounter = 0;

#define CS_PIN_DEVICE1 PIO_PA28_IDX // replace with your actual CS pins
#define CS_PIN_DEVICE2 PIO_PA29_IDX
#define CS_PIN_DEVICE3 PIO_PA30_IDX
#define CS_PIN_DEVICE4 PIO_PA31_IDX

int calibrated_time_ms = 0;
int time_datum = 0; 



// buffer for data passing down
volatile uint8_t detector_data[NUM_OF_DETECTOR][DATA_LENGTH];

// create a ring buffer for an efficient rolling buffer
int end_index = 0;
int start_index = 0;


// Serial set up 
// Serial receiving & Complete Flags
volatile uint8_t serial_receiving = 0;
volatile uint8_t serial_complete = 0;
volatile uint8_t send_data_flag = 0;
uint32_t messageCounter = 0;



// Bytes Received Counters
volatile uint8_t serial_received_bytes_counter = 0;
volatile uint8_t total_bytes = 0;

// Size of receive buffer
#define SERIAL_BUFFER_SIZE 200

// Receive and transmit buffers
volatile uint8_t rx_buffer[SERIAL_BUFFER_SIZE] = { 0x00 }; // make sure we don't have anything left in the memory
volatile uint8_t tx_buffer[SERIAL_BUFFER_SIZE + 14] = "Your message: ";
volatile uint8_t indicator_buffer[3] = "YES";




/**
 * Callback for Timer Task
 *
 */
static void timer_task_cb(const struct timer_task *const timer_task)
{
	// Toggle LED
	milliCounter++;
	
	messageCounter++;

	// Check if it's time to send a message
	if (messageCounter >= MESSAGE_INTERVAL_MS) {
		// Send serial message
		send_data_flag = 1;
		// Reset message counter
		messageCounter = 0;
	}

	// Reset millisecond counter every second
	if (milliCounter >= 1000) {
		milliCounter = 0;
		secondCounter++;
	}
	
}




// SPI functions

void read_SPI_data(void)
{
	struct io_descriptor *io;
	spi_m_sync_get_io_descriptor(&SPI_0, &io);
	
	uint8_t read_data; // Array to hold the read data

	// List of CS pins for all devices
	uint8_t cs_pins[4] = {CS_PIN_DEVICE1, CS_PIN_DEVICE2, CS_PIN_DEVICE3, CS_PIN_DEVICE4};

	for (int i = 0; i < 4; i++) {
		

		gpio_set_pin_level(cs_pins[i], false); // set the pin low (select the device)
		io_read(io, &read_data, 1); // Read 1 bytes of data
		gpio_set_pin_level(cs_pins[i], true); // set the pin high (deselect the device)
		add_to_buffer(read_data, i);
		

		//data_device = (read_data[0]<<8) | read_data[1]; // Assume the data is in big-endian byte order
	}
}


// buffer functions
void add_to_buffer(uint8_t new_entry, uint8_t detector_id) {
	detector_data[detector_id][end_index] = new_entry;
	end_index = (end_index + 1) % DATA_LENGTH;
	
	// If the buffer is full, increment the start index as well
	if (end_index == start_index) {
		start_index = (start_index + 1) % DATA_LENGTH;
	}
}

uint8_t get_from_buffer(uint8_t detector_id) {
	if (start_index == end_index) {
		// Buffer is empty, return an error value
		return 0xFF;
	}
	
	uint8_t entry = detector_data[detector_id][start_index];
	start_index = (start_index + 1) % DATA_LENGTH;
	return entry;
}

// USART functions
// Virtual COM port transmit callback function
static void serial_tx_cb(const struct usart_async_descriptor *const io_descr) {
	
	// Do nothing so far
}

void serial_send_data() {
	
	// TURN EVERYTHING INTO UINT16!!!!
	//volatile uint8_t serial_message[MESSAGE_LENGTH+1];

	//uint8_t newline[] = "\n";
	//memcpy(&serial_message[MESSAGE_LENGTH], newline, sizeof(newline));
	
	//serial_message[0] = data_mode;
//

	
	//// Extract milliseconds
	//serial_message[1] = (uint8_t)milliCounter; // Casts to 8 bit integer, only lower 8 bits are kept
	//serial_message[2] = (uint8_t)(milliCounter >> 8);  // Shift right by 8 bits and cast to uint8_t
	//serial_message[3] = (uint8_t)(milliCounter >> 16);  // Shift right by 16 bits and cast to uint8_t
	//serial_message[4] = (uint8_t)(milliCounter >> 24); // Shift right by 24 bits and cast to uint8_t
	//
	//
	//
	//// Extract seconds
	//serial_message[5] = (uint8_t)secondCounter; // Casts to 8 bit integer, only lower 8 bits are kept
	//serial_message[6] = (uint8_t)(secondCounter >> 8);  // Shift right by 8 bits and cast to uint8_t
	//serial_message[7] = (uint8_t)(secondCounter >> 16);  // Shift right by 16 bits and cast to uint8_t
	//serial_message[8] = (uint8_t)(secondCounter >> 24); // Shift right by 24 bits and cast to uint8_t
	
	//for (int i = 0; i < NUM_OF_DETECTOR; i++) {
		//serial_message[i+9] = get_from_buffer(i);
	//}
	
	
	//uint8_t txbuffer [MESSAGE_LENGTH+1];
	//memcpy(&txbuffer[0],&serial_message[0],MESSAGE_LENGTH+1);
	
	
	//io_write(&USART_0.io, &data_mode, 1);
	//io_write(&USART_0.io, &secondCounter, sizeof(secondCounter));
	//io_write(&USART_0.io, &milliCounter, sizeof(milliCounter));
	//for (int i = 0; i < NUM_OF_DETECTOR; i++) {
		//uint8_t detector = get_from_buffer(i);
		//io_write(&USART_0.io, &detector,1);
		//memset(&detector,0x00,1);
//
	//}
	
	// create a buffer big enough to hold all the data
uint8_t buffer[1 + sizeof(secondCounter) + sizeof(milliCounter) + NUM_OF_DETECTOR] = {0x00};
//uint8_t buffer[5] = {0x00};
int index = 0;
volatile int result = 0;

// add data_mode to the buffer

buffer[index++] = data_mode;

//// add secondCounter to the buffer
//memcpy(&buffer[index], &secondCounter, sizeof(secondCounter));
//index += sizeof(secondCounter);
//
//// add milliCounter to the buffer
//memcpy(&buffer[index], &milliCounter, sizeof(milliCounter));
//index += sizeof(milliCounter);

// add detector data to the buffer
for (int i = 0; i < NUM_OF_DETECTOR; i++) {
	uint8_t detector = get_from_buffer(i);
	buffer[index++] = detector;
}

// write the entire buffer
result = io_write(&USART_0.io, &buffer, sizeof(buffer));

memset(&buffer,0x00,sizeof(buffer));

	
// 	io_write(&USART_0.io, txbuffer, MESSAGE_LENGTH+1);
// 	
// 	//io_write(&USART_0.io, newline, sizeof(newline));
// 	//io_write(&USART_0.io, '\n', 2);
// 	memset(&serial_message,0x00,MESSAGE_LENGTH+1);
// 	memset(&txbuffer,0x00,MESSAGE_LENGTH+1);
	//memset(&newline,0x00,sizeof(newline));
	

}


int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	
	// Set up Timer Function
	task.interval = 1;
	task.cb = timer_task_cb;
	task.mode = TIMER_TASK_REPEAT;
	
	// Add timer task
	timer_add_task(&TIMER_0, &task);
	timer_start(&TIMER_0);


	// Configure CS pins as output, initial state high
	gpio_set_pin_direction(CS_PIN_DEVICE1, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(CS_PIN_DEVICE2, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(CS_PIN_DEVICE3, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(CS_PIN_DEVICE4, GPIO_DIRECTION_OUT);
	
	// Enable SPI
	spi_m_sync_enable(&SPI_0);
	
	
		usart_async_register_callback(&USART_0, USART_ASYNC_TXC_CB, serial_tx_cb);
	int32_t result = usart_async_enable(&USART_0);
	if (result == ERR_NONE) {
		uint8_t startPrint [12] = "Serial ready";
		uint8_t newline[] = "\n";
		io_write(&USART_0.io, startPrint, 12);
		io_write(&USART_0.io, newline, sizeof(newline));
		memset(startPrint,0x00,12);
		memset(newline,0x00,sizeof(newline));
	}
	

	while (1) {
		read_SPI_data();
		
		if (send_data_flag) {
			
			uint8_t startNum = "s";
			uint8_t buffer[2 + sizeof(startNum) + sizeof(secondCounter) + sizeof(milliCounter) + NUM_OF_DETECTOR] = {0x00};

			int index = 0;
			volatile int result = 0;
			

			// add startNum to the buffer
			memcpy(&buffer[index], &startNum, sizeof(startNum));
			index += sizeof(startNum);

			// add data_mode to the buffer
			buffer[index++] = data_mode;

			// add secondCounter to the buffer
			memcpy(&buffer[index], &secondCounter, sizeof(secondCounter));
			index += sizeof(secondCounter);

			// add milliCounter to the buffer
			memcpy(&buffer[index], &milliCounter, sizeof(milliCounter));
			index += sizeof(milliCounter);

			// add detector data to the buffer
			for (int i = 0; i < NUM_OF_DETECTOR; i++) {
				uint8_t detector = get_from_buffer(i);
				buffer[index++] = detector;
			}



			//// add endNum to the buffer
			//memcpy(&buffer[index], &endNum, sizeof(endNum));
			//index += sizeof(endNum);
			
			size_t input_data_len = sizeof(buffer) / sizeof(uint8_t); // calculate size of your input data

			// estimate size of the output buffer
			size_t output_buffer_len = ((input_data_len + 2) / 3 * 4) + 1; // "+ 1" for null terminator

			// allocate the output buffer
			char* output_buffer = (char*)malloc(output_buffer_len);
			if (output_buffer == NULL) {
				// handle error
			}

			// encode the data

			int ret = atcac_base64_encode(buffer, input_data_len, output_buffer, &output_buffer_len);
			if (ret != ATCA_SUCCESS) {
				// handle error
			}

			// now output_buffer contains the base64-encoded string and output_buffer_len is its actual length

			// write the entire buffer
			result = io_write(&USART_0.io, &buffer, sizeof(buffer));
			
			
			// Don't forget to free the output buffer when you're done with it
			free(output_buffer);

			memset(&buffer,0x00,sizeof(buffer));

			send_data_flag = 0;

		
	}
}


	}
	
	



// encoding
static const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const uint8_t* data, uint16_t input_length, char* output, uint16_t output_length) {
    uint8_t i;
    uint8_t j = 0;
    uint8_t temp[3];
    uint8_t sextet;

    for (i = 0; i < input_length; i++) {
        // Take three bytes of input at a time
        if ((i % 3) == 0) {
            // If there is enough space in the output buffer
            if ((j + 4) < output_length) {
                // Start a new temp array
                temp[0] = temp[1] = temp[2] = 0;
            } else {
                return; // Not enough space in output buffer
            }
        }
        temp[i % 3] = data[i];
        if ((i % 3) == 2) {
            // Output the four encoded bytes
            sextet = (temp[0] >> 2) & 0x3F;
            output[j++] = base64chars[sextet];
            sextet = ((temp[0] & 3) << 4) | ((temp[1] >> 4) & 0x0F);
            output[j++] = base64chars[sextet];
            sextet = ((temp[1] & 0x0F) << 2) | ((temp[2] >> 6) & 0x03);
            output[j++] = base64chars[sextet];
            sextet = temp[2] & 0x3F;
            output[j++] = base64chars[sextet];
        }
    }

    // Deal with the case where the input length is not divisible by 3
    if ((i % 3) == 1) {
        sextet = (temp[0] >> 2) & 0x3F;
        output[j++] = base64chars[sextet];
        sextet = ((temp[0] & 3) << 4);
        output[j++] = base64chars[sextet];
        output[j++] = '='; // Pad with equals signs
        output[j++] = '=';
    } else if ((i % 3) == 2) {
        sextet = (temp[0] >> 2) & 0x3F;
        output[j++] = base64chars[sextet];
        sextet = ((temp[0] & 3) << 4) | ((temp[1] >> 4) & 0x0F);
        output[j++] = base64chars[sextet];
        sextet = ((temp[1] & 0x0F) << 2);
        output[j++] = base64chars[sextet];
        output[j++] = '=';
    }
    output[j] = '\0'; // Null-terminate the output string
}

