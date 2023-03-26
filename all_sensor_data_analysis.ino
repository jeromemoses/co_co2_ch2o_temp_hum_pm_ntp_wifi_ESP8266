#include<SoftwareSerial.h>
#include "DHT.h"

void read_ch2o(float*);
void read_co(float*);
void read_co2(int*);
void read_ps_PM(int*,int*,int*);

DHT dht(10, DHT22);

//ch2o commands
const byte ch2o_start_QAmode_cmd[] = {0xFF,0x01,0x78,0x41,0x00,0x00,0x00,0x00,0x46};
const byte ch2o_read_cmd[] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
const byte ch20_return_cmd[] = {0xFF,0x86,0x00,0x2A,0x00,0x00,0x00,0x20,0x30};

//carbon monoxide commands - CO
const byte co_start_cmd[] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};

//Pollution sensor commands
const byte ps_start_QAmode_cmd[] = {0XFF,0x01,0x78,0x41,0x00,0x00,0x00,0x00,0x46};
const byte ps_read_cmd[] = {0XFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};

//corbon-dioxide commands (CO2)
const byte co2_start_cmd[] = {0XFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};

byte ch2o_received_bytes[9];
byte co_received_bytes[9];
byte ps_received_byte[24];
byte co2_received_bytes[9];

SoftwareSerial ch2o_serial(D1,D2);
SoftwareSerial co_serial(D3,D4);
SoftwareSerial ps_serial(D5,D6);
SoftwareSerial co2_serial(D7,D8);

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
co_serial.begin(9600);
co2_serial.begin(9600);
ch2o_serial.begin(9600);
ps_serial.begin(9600);
Serial.print("CLEAR DATA");
delay(1000);

//formaldehyde sensor startup code for Q&A mode
for(int i =0;i<9;i++)
{
  ch2o_serial.write(ch2o_start_QAmode_cmd[i]);
}

//pollution sesnor startup code for Q&A mode
for(int i = 0;i<9;i++)
{
  ps_serial.write(ps_start_QAmode_cmd[i]);
}
dht.begin();
}

void loop() {
//co
Serial.println();
float co;
Serial.print("\t");
read_co(&co);
Serial.print(co);
Serial.print("\t");
delay(3000);
  
//co2
int co2;
read_co2(&co2);
Serial.print(co2);
Serial.print("\t");
delay(3000);
 
//ch2o
float ch2o;
read_ch2o(&ch2o);
Serial.print(ch2o);
Serial.print("\t");
delay(3000);
  
//pollution 
int pm1_0;
int pm2_5;
int pm10;
  
read_ps_PM(&pm1_0 , &pm2_5 , &pm10);
Serial.print(pm1_0);
Serial.print("\t");
 
Serial.print (pm2_5);
Serial.print("\t");
 
Serial.print (pm10);
Serial.print("\t");
delay(3000);

//temperature and humidity 
float h = dht.readHumidity();
Serial.print(h);
Serial.print("\t");
float t = dht.readTemperature();
Serial.print(t);
delay(3000);
}

void read_ch2o(float *CH2O)
{
  
  ch2o_serial.write(ch2o_read_cmd,sizeof(ch2o_read_cmd));
  if(ch2o_serial.write(ch20_return_cmd,sizeof(ch20_return_cmd)) == 9)
  {
    for(byte i =0;i<9;i++)
    {
      ch2o_received_bytes[i] = ch2o_serial.read();
    }

//debug code
//    for(int j = 0;j<9;j++)
//    {
//      Serial.println(ch2o_received_bytes[j]);
//    }
//    Serial.println();
    //Gas concentration value=High byte of concentration *256+ Low byte of concentration
    *CH2O = (ch2o_received_bytes[2] * 256) + ch2o_received_bytes[3];
    *CH2O = *CH2O/1000;
  }
}

void read_co(float *co4and5)
{ 
  if(co_serial.write(co_start_cmd,sizeof(co_start_cmd))==9){
    for(int i=0;i<9;i++)
    {
      co_received_bytes[i] = co_serial.read();
    }
    
//   for(int j = 0;j<9;j++)
//   {
//    Serial.println( co_received_bytes[j]);
//   }
    
    int input_dec = co_received_bytes[2]; //assigns high byte to variable named input_dec
    int num = 0;              //variable to store binary digits in decimal form
    int base = 1;             //base of binary (8-4-2-1)
      

    //below code will creats binary number for high byte that is assign to variale named "input_dec" in decimal form 
    for(int j = 4;j>=0;j--)
    {
      //creats binary in decimal form
      if(input_dec >> j & 1 == 1)
      {
        if (j == 0)
        {
          num+=1;
        } 
        else if(j == 1)
        {
          num+=10;
        }
        else if(j == 2)
        {
          num+=100;
        }
        else if(j == 3)
        {
          num+=1000;  
        }
        else if(j == 4)
        {
          num+=10000;
        } 
      }
    }

//  Serial.print("binary number in decimal form: ");
//  Serial.println(num);
  
  int rem = 0; //variable to store last digit of input number(num) 
  int five_bit_decimal = 0;
  
  while (num > 0)  
    {  
        rem = num%10;                                   //returns the last digit of "num"
        five_bit_decimal = five_bit_decimal + (rem * base); //converts binary to decimal 
        num = num / 10;                                   //removes the last digit of "num" by dividing by 10 
        base = base * 2;                                  //binary form 8421 ,here consider it as a "base",
                                                         //the function converts binary from last digit one by one so base value starts from 1 then multiplies by 2 (1-2-4-8-16) = 5 bit's base  
    }
     
//    Serial.print("five bit to decimal: ");
//    Serial.println(five_bit_decimal);

    //Gas concentration value = (The low 5 bit of High Byte*256+Low Byte)*0.1.
    *co4and5 = ((five_bit_decimal * 256) + co_received_bytes[3]) * 0.1;
  }
}

void read_co2(int *CO2)
{
  if(co2_serial.write(co2_start_cmd,sizeof(co2_start_cmd))==9)
  {
        
    for(byte i =0;i<9;i++)
    {
      co2_received_bytes[i] = co2_serial.read();
    }
    
//    debug code  
//    for(int j = 0;j<9;j++)
//    {
//      Serial.println(co2_received_bytes[j]);
//    }
//    Serial.println();

  if((co2_received_bytes[0] == 255) && (co2_received_bytes[8] != 0))
   {
     *CO2 = (co2_received_bytes[2] * 255) + co2_received_bytes[3];  
   }
   else if(co2_received_bytes[8] == 0) 
   {
     *CO2 = (co2_received_bytes[3] * 255) + co2_received_bytes[4];
   }
  }
}

void read_ps_PM(int* res1,int* res2,int *res3)
{
  int pm = 0;
   if(ps_serial.write(ps_read_cmd,sizeof(ps_read_cmd)) == 9)
   {
    for(byte i = 0;i<9;i++)
    {
      ps_received_byte[i] = ps_serial.read();
    }

//    debug code
//    for(byte j = 0;j<9;j++)
//    {
//      Serial.println(ps_received_byte[j]);
//    }

      //pm1_0
      *res1 = (0x00 * 256) + ps_received_byte[7]; 
      //*res1 = *res1/1000;  //converts ug/m3 to ppm   
      
      //pm2_5
      *res2 = (0x00 * 256) + ps_received_byte[3];
      //*res2 = *res2/1000;  //converts ug/m3 to ppm  
      
     
      //pm10
      *res3 = (0x00 * 256) + ps_received_byte[5];
      //*res3 = *res3/1000;  //converts ug/m3 to ppm  
   }
}
