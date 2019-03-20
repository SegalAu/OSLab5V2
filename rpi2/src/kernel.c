
/*
*
*	The Kernel
*
*/

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "kernel.h"
#include "hal/hal.h"
#include "string.h"

#include "drivers/stdio/emb-stdio.h"			// Needed for printf
#include "drivers/sdcard/SDCard.h"

void kernel_init(void);
void input_output_init(void);
void sys_info( uint8_t* );
void sd_card_fs_demo();
void catText(char* file_name);
void openBin(char* file_name);
void printFile(char* file_name, int file_size);

/*
 *		Kernel's entry point
**/
void main(uint32_t r0, uint32_t r1, uint32_t atags){

  //Init
  kernel_init();
  input_output_init();

  sd_card_fs_demo();   //<<-- Uncomment this to show File System/SD Card demo
  
  openBin("app.bin");

  //Welcome Msg Video
  hal_io_video_puts( "\n\r\n\rWelcome to MiniOS Pi Zero\n\r", 3, VIDEO_COLOR_GREEN );
  hal_io_serial_puts( SerialA, "\n\r\n\rWelcome to MiniOS Pi Zero\n\r" );
  hal_io_video_puts( "\n\r$ ", 2, VIDEO_COLOR_GREEN );
  hal_io_serial_puts( SerialA, "\n\r$ " );

  uint8_t c;

	while (1){
    c = hal_io_serial_getc( SerialA );

    printf_video( "%c", c );  //<<--- We also have printfs
    printf_serial( "%c", c );
  }

}

/*
* Initializes the kernel
*/
void kernel_init(void){

  hal_io_init();
  //console_init();
  //system_calls_init();
  //scheduler_init();
  //faults_init();

}

/*
* Initializes All IO
*/
void input_output_init(void){
  uint32_t video_init_res = HAL_FAILED;

#ifdef VIDEO_PRESENT
  video_init_res = hal_io_video_init();
#endif

#ifdef SERIAL_PRESENT
  hal_io_serial_init();
#endif
//NOTE: PAST THIS POINT WE CAN USE printf
//      (printf needs both serial and video inits to be called first)

  if ( video_init_res == HAL_SUCCESS )
    sys_info( "Video Initialized\n\r" );
  else
    sys_info( "Video Init Failed [x]\n\r" );

    sys_info( "Serial Initialized\n\r" );
}

void sys_info( uint8_t* msg ){
  printf_video( msg );
  printf_serial( msg );
}

/////////////////////////////////////////////////////////////////
////////////////    D E M O    C O D E    ///////////////////////
/////////////////////////////////////////////////////////////////

char buffer[500];
char bin_buffer[8192];
void DisplayDirectory(const char*);


void sd_card_fs_demo(){
  printf_serial("\n\n");
  sdInitCard (&printf_serial, &printf_serial, true);

  /* Display root directory */
  printf_serial("\n\nDirectory (/): \n");
  DisplayDirectory("\\*.*");

  printf_serial("\n");
  printf_serial("Opening Alice.txt \n");
  catText("Alice.txt");

/*   HANDLE fHandle = sdCreateFile("Alice.txt", GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (fHandle != 0) {
    uint32_t bytesRead;

    if ((sdReadFile(fHandle, &buffer[0], 500, &bytesRead, 0) == true))  {
        buffer[bytesRead-1] = '\0';  ///insert null char
        printf_serial("File Contents: %s", &buffer[0]);
    }
    else{
      printf_serial("Failed to read" );
    }

    // Close the file
    sdCloseHandle(fHandle);

  } */

}

void catText(char* file_name) {
	char text_buffer[2];
	HANDLE fHandle = sdCreateFile(file_name, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (fHandle != 0) {
		uint32_t bytesRead;

    // Fake directory bullshit:global current directory
    char currentDir[256];
    memset(currentDir, '\0', 256);
		
		while (1) {
			if (sdReadFile(fHandle, text_buffer, 1, &bytesRead, 0) != true) {
				printf_serial("Failed to read");
				break;
			}
			if (text_buffer[0] == -1) {
				break;
			}
			text_buffer[bytesRead] = 0;
			printf_serial("%s", text_buffer);
		}
	}
	sdCloseHandle(fHandle);
}

void openBin(char* file_name) {
	char text_buffer;
	int counter = 0;
	
	HANDLE fHandle = sdCreateFile(file_name, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (fHandle != 0) {
		uint32_t bytesRead;
	
		while (1) {
			if (sdReadFile(fHandle, &text_buffer, 1, &bytesRead, 0) != true) {
				printf_serial("\nFinished reader\n");
				break;
			}
			bin_buffer[counter] = text_buffer;
			//printf_serial("%c", bin_buffer[counter]);
			++counter;
		}
	}
	printFile(bin_buffer, counter);
}

void printFile(char* file_name, int file_size) {
	for (int i = 0; i < file_size; ++i) {
		printf_serial("%c", file_name[i]);
	}		
}

void DisplayDirectory(const char* dirName) {
	HANDLE fh;
	FIND_DATA find;
	char* month[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	fh = sdFindFirstFile(dirName, &find);							// Find first file
	do {
		if (find.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			printf_serial("%s <DIR>\n", find.cFileName);
		else printf_serial("%c%c%c%c%c%c%c%c.%c%c%c Size: %9lu bytes, %2d/%s/%4d, LFN: %s\n",
			find.cAlternateFileName[0], find.cAlternateFileName[1],
			find.cAlternateFileName[2], find.cAlternateFileName[3],
			find.cAlternateFileName[4], find.cAlternateFileName[5],
			find.cAlternateFileName[6], find.cAlternateFileName[7],
			find.cAlternateFileName[8], find.cAlternateFileName[9],
			find.cAlternateFileName[10],
			(unsigned long)find.nFileSizeLow,
			find.CreateDT.tm_mday, month[find.CreateDT.tm_mon],
			find.CreateDT.tm_year + 1900,
			find.cFileName);										// Display each entry
	} while (sdFindNextFile(fh, &find) != 0);						// Loop finding next file
	sdFindClose(fh);												// Close the serach handle
}

/*
  Need global currentDir string to change, put it into old_dir
  User input will be thrown into old_dir to update it
*/
void ChangeDirectory(char * old_dir, char* new_dir) {
  strcpy(old_dir, new_dir);
  printf_serial( "filepath %s \n\r", old_dir);
}

