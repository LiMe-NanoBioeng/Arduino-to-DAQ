# Arduino-to-DAQ ver2 Serial_base_v20250202

Arduino program for a serial port DAQ.  
Command list  

#AO[int channel][v/b][int duty]  
-Analogue output  
-For Arduino micro, analogue output pins are 5,6,9-11.  
-v: milli volt 0-5000, b: byte 0-255  
-example:"AO6v5000"  

#AI[int channel :int channel]  
-Analogue input  
-exmaple:"AI1:3"  

#DO[int channel][H/L]  
-Digital output  
-example "DO1H"  

#DI  
-Not surpported yet.  
