#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define SDI_GPIO 4      // SDI is assigned to GPIO 4, pin 4
#define SCL_GPIO 5      // SCL is assigned to GPIO 5, pin 5
#define CS_GPIO 6       // CS is assigned to GPIO 6, pin 6
#define DC_GPIO 7       // DC is assigned to GPIO 7, pin 7
#define RES_GPIO 3      // RES is assigned to GPIO 3, pin 3
//BS0 and BS1 tied to ground tells display to operate in 4-wire SPI

int steps;
int heartRate;

unsigned char Ascii_1[97][5] = {     // Refer to "Times New Roman" Font Database...
                        //   Basic Characters
    // each character is basically an 8x5 array where the rows are represented by the bits being stored below
    // this is far more elegant than the 3D array I came up with on my own
    {0x00,0x00,0x00,0x00,0x00},     //   (  0)    - 0x0000 Empty set
    {0x00,0x00,0x4F,0x00,0x00},     //   (  1)  ! - 0x0021 Exclamation Mark
    {0x00,0x07,0x00,0x07,0x00},     //   (  2)  " - 0x0022 Quotation Mark
    {0x14,0x7F,0x14,0x7F,0x14},     //   (  3)  # - 0x0023 Number Sign
    {0x24,0x2A,0x7F,0x2A,0x12},     //   (  4)  $ - 0x0024 Dollar Sign
    {0x23,0x13,0x08,0x64,0x62},     //   (  5)  % - 0x0025 Percent Sign
    {0x36,0x49,0x55,0x22,0x50},     //   (  6)  & - 0x0026 Ampersand
    {0x00,0x05,0x03,0x00,0x00},     //   (  7)  ' - 0x0027 Apostrophe
    {0x00,0x1C,0x22,0x41,0x00},     //   (  8)  ( - 0x0028 Left Parenthesis
    {0x00,0x41,0x22,0x1C,0x00},     //   (  9)  ) - 0x0029 Right Parenthesis
    {0x14,0x08,0x3E,0x08,0x14},     //   ( 10)  * - 0x002A Asterisk
    {0x08,0x08,0x3E,0x08,0x08},     //   ( 11)  + - 0x002B Plus Sign
    {0x00,0x50,0x30,0x00,0x00},     //   ( 12)  , - 0x002C Comma
    {0x08,0x08,0x08,0x08,0x08},     //   ( 13)  - - 0x002D Hyphen-Minus
    {0x00,0x60,0x60,0x00,0x00},     //   ( 14)  . - 0x002E Full Stop
    {0x20,0x10,0x08,0x04,0x02},     //   ( 15)  / - 0x002F Solidus
    {0x3E,0x51,0x49,0x45,0x3E},     //   ( 16)  0 - 0x0030 Digit Zero
    {0x00,0x42,0x7F,0x40,0x00},     //   ( 17)  1 - 0x0031 Digit One
    {0x42,0x61,0x51,0x49,0x46},     //   ( 18)  2 - 0x0032 Digit Two
    {0x21,0x41,0x45,0x4B,0x31},     //   ( 19)  3 - 0x0033 Digit Three
    {0x18,0x14,0x12,0x7F,0x10},     //   ( 20)  4 - 0x0034 Digit Four
    {0x27,0x45,0x45,0x45,0x39},     //   ( 21)  5 - 0x0035 Digit Five
    {0x3C,0x4A,0x49,0x49,0x30},     //   ( 22)  6 - 0x0036 Digit Six
    {0x01,0x71,0x09,0x05,0x03},     //   ( 23)  7 - 0x0037 Digit Seven
    {0x36,0x49,0x49,0x49,0x36},     //   ( 24)  8 - 0x0038 Digit Eight
    {0x06,0x49,0x49,0x29,0x1E},     //   ( 25)  9 - 0x0039 Dight Nine
    {0x00,0x36,0x36,0x00,0x00},     //   ( 26)  : - 0x003A Colon
    {0x00,0x56,0x36,0x00,0x00},     //   ( 27)  ; - 0x003B Semicolon
    {0x08,0x14,0x22,0x41,0x00},     //   ( 28)  < - 0x003C Less-Than Sign
    {0x14,0x14,0x14,0x14,0x14},     //   ( 29)  = - 0x003D Equals Sign
    {0x00,0x41,0x22,0x14,0x08},     //   ( 30)  > - 0x003E Greater-Than Sign
    {0x02,0x01,0x51,0x09,0x06},     //   ( 31)  ? - 0x003F Question Mark
    {0x32,0x49,0x79,0x41,0x3E},     //   ( 32)  @ - 0x0040 Commercial At
    {0x7E,0x11,0x11,0x11,0x7E},     //   ( 33)  A - 0x0041 Latin Capital Letter A
    {0x7F,0x49,0x49,0x49,0x36},     //   ( 34)  B - 0x0042 Latin Capital Letter B
    {0x3E,0x41,0x41,0x41,0x22},     //   ( 35)  C - 0x0043 Latin Capital Letter C
    {0x7F,0x41,0x41,0x22,0x1C},     //   ( 36)  D - 0x0044 Latin Capital Letter D
    {0x7F,0x49,0x49,0x49,0x41},     //   ( 37)  E - 0x0045 Latin Capital Letter E
    {0x7F,0x09,0x09,0x09,0x01},     //   ( 38)  F - 0x0046 Latin Capital Letter F
    {0x3E,0x41,0x49,0x49,0x7A},     //   ( 39)  G - 0x0047 Latin Capital Letter G
    {0x7F,0x08,0x08,0x08,0x7F},     //   ( 40)  H - 0x0048 Latin Capital Letter H
    {0x00,0x41,0x7F,0x41,0x00},     //   ( 41)  I - 0x0049 Latin Capital Letter I
    {0x20,0x40,0x41,0x3F,0x01},     //   ( 42)  J - 0x004A Latin Capital Letter J
    {0x7F,0x08,0x14,0x22,0x41},     //   ( 43)  K - 0x004B Latin Capital Letter K
    {0x7F,0x40,0x40,0x40,0x40},     //   ( 44)  L - 0x004C Latin Capital Letter L
    {0x7F,0x02,0x0C,0x02,0x7F},     //   ( 45)  M - 0x004D Latin Capital Letter M
    {0x7F,0x04,0x08,0x10,0x7F},     //   ( 46)  N - 0x004E Latin Capital Letter N
    {0x3E,0x41,0x41,0x41,0x3E},     //   ( 47)  O - 0x004F Latin Capital Letter O
    {0x7F,0x09,0x09,0x09,0x06},     //   ( 48)  P - 0x0050 Latin Capital Letter P
    {0x3E,0x41,0x51,0x21,0x5E},     //   ( 49)  Q - 0x0051 Latin Capital Letter Q
    {0x7F,0x09,0x19,0x29,0x46},     //   ( 50)  R - 0x0052 Latin Capital Letter R
    {0x46,0x49,0x49,0x49,0x31},     //   ( 51)  S - 0x0053 Latin Capital Letter S
    {0x01,0x01,0x7F,0x01,0x01},     //   ( 52)  T - 0x0054 Latin Capital Letter T
    {0x3F,0x40,0x40,0x40,0x3F},     //   ( 53)  U - 0x0055 Latin Capital Letter U
    {0x1F,0x20,0x40,0x20,0x1F},     //   ( 54)  V - 0x0056 Latin Capital Letter V
    {0x3F,0x40,0x38,0x40,0x3F},     //   ( 55)  W - 0x0057 Latin Capital Letter W
    {0x63,0x14,0x08,0x14,0x63},     //   ( 56)  X - 0x0058 Latin Capital Letter X
    {0x07,0x08,0x70,0x08,0x07},     //   ( 57)  Y - 0x0059 Latin Capital Letter Y
    {0x61,0x51,0x49,0x45,0x43},     //   ( 58)  Z - 0x005A Latin Capital Letter Z
    {0x00,0x7F,0x41,0x41,0x00},     //   ( 59)  [ - 0x005B Left Square Bracket
    {0x02,0x04,0x08,0x10,0x20},     //   ( 60)  \ - 0x005C Reverse Solidus
    {0x00,0x41,0x41,0x7F,0x00},     //   ( 61)  ] - 0x005D Right Square Bracket
    {0x04,0x02,0x01,0x02,0x04},     //   ( 62)  ^ - 0x005E Circumflex Accent
    {0x40,0x40,0x40,0x40,0x40},     //   ( 63)  _ - 0x005F Low Line
    {0x01,0x02,0x04,0x00,0x00},     //   ( 64)  ` - 0x0060 Grave Accent
    {0x20,0x54,0x54,0x54,0x78},     //   ( 65)  a - 0x0061 Latin Small Letter A
    {0x7F,0x48,0x44,0x44,0x38},     //   ( 66)  b - 0x0062 Latin Small Letter B
    {0x38,0x44,0x44,0x44,0x20},     //   ( 67)  c - 0x0063 Latin Small Letter C
    {0x38,0x44,0x44,0x48,0x7F},     //   ( 68)  d - 0x0064 Latin Small Letter D
    {0x38,0x54,0x54,0x54,0x18},     //   ( 69)  e - 0x0065 Latin Small Letter E
    {0x08,0x7E,0x09,0x01,0x02},     //   ( 70)  f - 0x0066 Latin Small Letter F
    {0x06,0x49,0x49,0x49,0x3F},     //   ( 71)  g - 0x0067 Latin Small Letter G
    {0x7F,0x08,0x04,0x04,0x78},     //   ( 72)  h - 0x0068 Latin Small Letter H
    {0x00,0x44,0x7D,0x40,0x00},     //   ( 73)  i - 0x0069 Latin Small Letter I
    {0x20,0x40,0x44,0x3D,0x00},     //   ( 74)  j - 0x006A Latin Small Letter J
    {0x7F,0x10,0x28,0x44,0x00},     //   ( 75)  k - 0x006B Latin Small Letter K
    {0x00,0x41,0x7F,0x40,0x00},     //   ( 76)  l - 0x006C Latin Small Letter L
    {0x7C,0x04,0x18,0x04,0x7C},     //   ( 77)  m - 0x006D Latin Small Letter M
    {0x7C,0x08,0x04,0x04,0x78},     //   ( 78)  n - 0x006E Latin Small Letter N
    {0x38,0x44,0x44,0x44,0x38},     //   ( 79)  o - 0x006F Latin Small Letter O
    {0x7C,0x14,0x14,0x14,0x08},     //   ( 80)  p - 0x0070 Latin Small Letter P
    {0x08,0x14,0x14,0x18,0x7C},     //   ( 81)  q - 0x0071 Latin Small Letter Q
    {0x7C,0x08,0x04,0x04,0x08},     //   ( 82)  r - 0x0072 Latin Small Letter R
    {0x48,0x54,0x54,0x54,0x20},     //   ( 83)  s - 0x0073 Latin Small Letter S
    {0x04,0x3F,0x44,0x40,0x20},     //   ( 84)  t - 0x0074 Latin Small Letter T
    {0x3C,0x40,0x40,0x20,0x7C},     //   ( 85)  u - 0x0075 Latin Small Letter U
    {0x1C,0x20,0x40,0x20,0x1C},     //   ( 86)  v - 0x0076 Latin Small Letter V
    {0x3C,0x40,0x30,0x40,0x3C},     //   ( 87)  w - 0x0077 Latin Small Letter W
    {0x44,0x28,0x10,0x28,0x44},     //   ( 88)  x - 0x0078 Latin Small Letter X
    {0x0C,0x50,0x50,0x50,0x3C},     //   ( 89)  y - 0x0079 Latin Small Letter Y
    {0x44,0x64,0x54,0x4C,0x44},     //   ( 90)  z - 0x007A Latin Small Letter Z
    {0x00,0x08,0x36,0x41,0x00},     //   ( 91)  { - 0x007B Left Curly Bracket
    {0x00,0x00,0x7F,0x00,0x00},     //   ( 92)  | - 0x007C Vertical Line
    {0x00,0x41,0x36,0x08,0x00},     //   ( 93)  } - 0x007D Right Curly Bracket
    {0x02,0x01,0x02,0x04,0x02},     //   ( 94)  ~ - 0x007E Tilde
    {0x08,0x0C,0x0E,0x0C,0x08},     //   ( 95)  upward facing triangle/arrow
    {0x08,0x18,0x38,0x18,0x08},     //   ( 96)  downward facing triangle/arrow
};

// send a command to the display
// refer to command table in display MCU datasheet
void command(unsigned char c)               
{  
    unsigned char i;
    unsigned char mask = 0x80;                  // mask = 10000000
    gpio_set_level(CS_GPIO, 0);              
    gpio_set_level(DC_GPIO, 0);                 // DC set to 0 to tell MCU this is a command
    for(i=0;i<8;i++)                            // loop through each bit of c passing each bit through SDI
    {
        gpio_set_level(SCL_GPIO, 0);            // SCL falling edge
        if((c & mask) >> 7 == 1)                // checks if the first bit of c matches with the first bit of mask (1)
        {
            gpio_set_level(SDI_GPIO, 1);        // if so, output 1 on SDI
        }
        else
        {
            gpio_set_level(SDI_GPIO, 0);        // if not, output 0 on SDI
        }
        gpio_set_level(SCL_GPIO, 1);            // SCL rising edge
        c = c << 1;                             // pushes each bit of c left 1 (first bit is replaced by second)
    }
    gpio_set_level(CS_GPIO, 1);
} 

// send byte d of data to the display
void data(unsigned char d)                  
{ 
    unsigned char i;
    unsigned char mask = 0x80;                  // mask = 10000000
    gpio_set_level(CS_GPIO, 0);               
    gpio_set_level(DC_GPIO, 1);                 // DC set to 1 to tell MCU this is data
    for(i=0;i<8;i++)                            // loop through each bit of d passing each bit through SDI
    {
        gpio_set_level(SCL_GPIO, 0);            // SCL falling edge
        if((d & mask) >> 7 == 1)                // checks if the first bit of d matches with the first bit of mask (1)
        {
            gpio_set_level(SDI_GPIO, 1);        // if so, output 1 on SDI
        }
        else
        {
            gpio_set_level(SDI_GPIO, 0);        // if not, output 0 on SDI
        }
        gpio_set_level(SCL_GPIO, 1);            // SCL rising edge
        d = d << 1;                             // pushes each bit of d left 1 (first bit is replaced by second)
    }
    gpio_set_level(CS_GPIO, 1); 
}

// begins writing data to ram, will continue until command is called.
void startWrite(unsigned char startRow, unsigned char endRow, unsigned char startCol, unsigned char endCol)
{
    command(0x15);      //Row Address command
    data(startRow);
    data(endRow);
    command(0x75);      //Col Address command
    data(startCol);
    data(endCol);
    command(0x5C);      //Begin writing to ram (until command is given)
}

// sends 3 byte color data one byte at a time
void sendColor(unsigned long color)
{
    data(color << 16);  // send 1st byte
    data(color << 8);   // send 2nd byte
    data(color);        // send 3rd byte
}

// takes a char letter and prints it to display at (x,y)
// default size of character is 8 rows by 5 columns
void text(unsigned char x, unsigned char y, unsigned char letter, unsigned long textColor, unsigned long bgColor, int size)
{
    int row;
    int col;
    unsigned char mask = 0x80;
    
    for(row=0;row<8*size;row++)
    {
        startWrite(x, x+(8*size), y, y+(5*size));
        for (col=0;col<5*size;col++)    
        {
            if((Ascii_1[letter][col/size] & mask) == mask)
                sendColor(textColor);
            else
                sendColor(bgColor);
        }
        y++;
        if((row%size)==0)
        {
          mask = mask >> 1;                                 // pushes each bit of mask right one
        }
   }
}

// takes an integer num and prints it to display at (x,y)
void number(int num, unsigned char x, unsigned char y, unsigned long textColor, unsigned long bgColor, int size)
{
    static int power10LU[5] = {1,10,100,1000,10000};                // look up table for powers of 10
    int length = 0;
    for(int pow10=4;pow10>-1;pow10--)
    {
        unsigned char placeVal = (num / power10LU[pow10])%10;       // finds place value at current power
        if(placeVal > 0 || length > 0)
        {
            text(x,y+length,16 + placeVal,textColor,bgColor,size);  // prints place value
            length = length + 5*size;
        }
    }
}

// fills screen with given color
void fillScreen(unsigned long color)
{
    int row;
    int col;
    startWrite(0,127,0,127);
    for(row=0;row<128;row++)
    {
        for(col=0;col<128;col++)
        {
            sendColor(color);
        }
    }
}

/* not being used

manually write image from 2D array of color data
int writeToDisplay(unsigned long displayData[][], unsigned char x, unsigned char y, int rows, int cols)
{
    int currRow;
    int currCol;
    int endRow = x+rows;
    int endCol = y+cols;
    if((endRow > 128) || (endCol > 128))    // should include same protections in other methods in future
    {
        return 0;
    }
    startWrite(x,endRow,y,endCol);
    for(currRow=x;currRow<endRow;currRow++) // loops through displayData and sends color data to display
    {
        for(currCol=y;currCol<endCol;currCol++)
        {
            sendColor(displayData[currRow][currCol]);
        }
    }
    return 1;
}*/

// gets step count from accelerometer and assigns it to global variable steps
void getSteps()
{
    //get stepcount from accelerometer
    steps++; //for testing purposes, step count is just incremented
}

// gets heart rate from heart rate monitor and assigns it to global variable heartRate
void getHeartRate()
{
    //get heart rate from heart rate monitor
    heartRate++; //for testing purposes, heartRate is just incremented
}

// Initializes OLED display and gives time for built in MCU to stabilize
// Mostly uses default settings of display
void initDisplay()
{
    gpio_set_level(RES_GPIO, 0);
    vTaskDelay(500);
    gpio_set_level(RES_GPIO, 1);
    vTaskDelay(500);

    command(0xFD);	    // Command lock setting
    data(0x12);		    // unlock
    command(0xFD);	    // Command lock setting
    data(0xB1);		    // unlock
    command(0xAE);
    /*command(0xB3);	// clock & frequency
    data(0xF1);		    // clock=Diviser+1 frequency=fh */
    command(0xCA);	    // Duty
    data(0x7F);		    // OLED _END+1
    command(0xA2);  	// Display offset
    data(0x00);
    command(0xA1);	    // Set display start line
    data(0x00);		    // 0x00 start line
    command(0xA0);	    // Set Re-map, color depth
    data(0xA0);		    // 8-bit 262K
    command(0xB5);	    // set GPIO
    data(0x00);		    // disabled
    command(0xAB);	    // Function Set
    data(0x01);		    // 8-bit interface, internal VDD regulator
    /*command(0xB4);	    // set VSL
    data(0xA0);		    // external VSL
    data(0xB5);
    data(0x55);*/
    command(0xC1);	    // Set contrast current for A,B,C
    data(0x8A);		    // Color A
    data(0x70);		    // Color B
    data(0x8A);		    // Color C
    command(0xC7);	    // Set master contrast
    data(0x0F);		    	
    command(0xB9);	    // use linear grayscale LUT
    command(0xB1);	    // Set pre & dis-charge
    data(0x32);		    // pre=1h, dis=1h  
    command(0xBB);	    // Set precharge voltage of color A,B,C
    data(0x07);		
    command(0xB2);      // display enhancement
    data(0xa4);		
    data(0x00);
    data(0x00);
    command(0xB6);	    // precharge period
    data(0x01);		
    /*command(0xBE);	    // Set VcomH
    data(0x07);*/
    command(0xA6);	    // Normal display
    command(0xAF);	    // Display on
}

//configures GPIO pins and in future will configure I2C
void setup(void)
{
    gpio_set_direction(SDI_GPIO, GPIO_MODE_DEF_OUTPUT);
    gpio_set_direction(SCL_GPIO, GPIO_MODE_DEF_OUTPUT);
    gpio_set_direction(CS_GPIO, GPIO_MODE_DEF_OUTPUT);
    gpio_set_direction(DC_GPIO, GPIO_MODE_DEF_OUTPUT);
    gpio_set_direction(RES_GPIO, GPIO_MODE_DEF_OUTPUT);
    gpio_set_level(CS_GPIO, 1);

    steps = 0;
    heartRate = 0;

    /*int i2c_master_port = 0;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,         // select GPIO specific to your project
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,         // select GPIO specific to your project
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,  // select frequency specific to your project
        // .clk_flags = 0,          !< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here.
    };*/


}

void app_main()
{
    printf("Hello world!\n");

    /* Print chip information 
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());*/

    setup();
    initDisplay();

    //fillScreen(0x000000);
    fillScreen(0xFFFFFF);

    text(0,0,40,0x0000FF,0x000000,2);   //print H
    text(0,10,69,0x0000FF,0x000000,2);  //print e
    text(0,20,76,0x0000FF,0x000000,2);  //print l
    text(0,30,76,0x0000FF,0x000000,2);  //print l
    text(0,40,79,0x0000FF,0x000000,2);  //print o
    text(0,50,0,0x0000FF,0x000000,2);   //print space
    text(0,60,55,0x0000FF,0x000000,2);  //print W
    text(0,70,79,0x0000FF,0x000000,2);  //print o
    text(0,80,82,0x0000FF,0x000000,2);  //print r
    text(0,90,76,0x0000FF,0x000000,2);  //print l
    text(0,100,68,0x0000FF,0x000000,2); //print d
    
    while(1==1)
    {
        getSteps();
        getHeartRate();
        number(steps,0x60,0x00,0x00FF00,0x000000,1);
        number(heartRate,070,0x00,0xFF0000,0x000000,1);
    }
}