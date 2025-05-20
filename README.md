# Arduino-to-DAQ ver2 Serial_base_v20250202<br/>

Arduino program for a serial port DAQ.  <br/>
## Command list<br/>
-DI: Read the digital channel (not designed yet)<br/>
-DO: Write the digital channel<br/>
```
 DO1H　　
 DO1L
```
 returns R if the commands successfully processed.<br/>
-DP: Rapidly modulate the digital channel High and then Low as<br/>
```
DP1,10,10,2,10
```
DP(channel num),delay time (ms), duration (ms),channel number of sensor, threshold of the sensor read to stop<br/>
-AI: Read the analogue channel<br/>
```
AI1:3
```
from channel 1 to channel 3<br/>
```
AI1,3
```
channel 1 and channel 3<br/>

An example code to call this function is

```
    def ArduinoAI():
        ser.write(b'AI7,8\n')
        time.sleep(0.01)
        ser_bytes = ser.readline()
        ser_bytes = ser_bytes.decode('utf-8').strip()
        decoded_bytes = ser_bytes
 
```       
-AO: Write the analogue channel<br/>
```
AO2,1v
```
analogue channel 2, number 0-255 corresponding to 0-5V, unit<br/>
-II: Read the I2C channel<br/>
```
II
```
An example Python code call this function

```
    def ArduinoI2C():
        ser.write(b'II\n')
        time.sleep(0.01)
        ser_bytes = ser.readline()
        ser_bytes=ser_bytes.decode('utf-8').rstrip()
        return(float(ser_bytes))

```
-F: Start the feedback control<br/>
an example python code to call this function is

```
    def ArduinoFB(value,vNumA,setpoint,Kp,Ki,Kd):
        if value==True:
            text='FB' + str(vNumA) + ',' + str(setpoint) + ',' + str(Kp) + ',' + str(Ki) + ',' + str(Kd) +'\n'
            ser.write(text.encode('utf-8'))
            time.sleep(0.1)
        else:
            ser.write(b'B')
            time.sleep(0.1)
            
```
-B: Stop the feedback control<br/>
