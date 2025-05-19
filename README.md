# Arduino-to-DAQ ver2 Serial_base_v20250202

Arduino program for a serial port DAQ.  
## Command list
-DI: Read the digital channel (not designed yet)
-DO: Write the digital channel
 DO1H or DO1L
 returns R if the commands successfully processed.
-DP: Rapidly modulate the digital channel High and then Low as
DP1,10,10,2,10
DP(channel num),delay time (ms), duration (ms),channel number of sensor, threshold of the sensor read to stop
-AI: Read the analogue channel
AI1:3
from channel 1 to channel 3
AI1,3
channel 1 and channel 3
-AO: Write the analogue channel
AO2,1v
analogue channel 2, number 0-255 corresponding to 0-5V, unit
-II: Read the I2C channel
-F: Start the feedback control
-B: Stop the feedback control
