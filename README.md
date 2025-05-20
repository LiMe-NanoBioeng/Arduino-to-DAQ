# Arduino-to-DAQ ver Serial_base_v20250202<br/>

Arduino program for a serial port DAQ.  <br/>
## Command list<br/>
- DI: Read the digital channel (not designed yet)<br/>
- DO: Write the digital channel<br/>
```
 DO1H　　
 DO1L
```
 returns R if the commands successfully processed.<br/>

```
     # Control Valve    
    def ArduinoDO(channel,flag):
        if flag:
            Dout = 'DO' + str(channel) + 'H\n'   
        else:
            Dout = 'DO' + str(channel) + 'L\n'
        ser.write(Dout.encode('utf-8'))
        ser_bytes = ser.readline().decode('utf-8')
        return(ser_bytes.strip())

```
        
- PP: Rapidly modulate the digital channel High and then Low as<br/>
```
PP1:2,10,10,2,10
```
PP(start ch #):(end ch #),delay time (ms), duration (ms),ch # of sensor, threshold of the sensor read to stop<br/>


```
    def ArduinoDigitalPulse(ch1,ch2,delay,width,threshold):
        # text = 'PP'+str(ch1)+':'+str(ch2)+','+str(int(delay))+','+str(width)+'\n'
        text = 'PP'+str(ch1)+','+str(ch2)+','+str(int(delay))+','+str(width)+','+'8'+','+str(int(threshold))+'\n' #use for two valves in pulse
        ser.write(text.encode('utf-8'))
        #time.sleep(delay+width)
```

- AI: Read the analogue channel<br/>
```
AI1:3
```
from ch 1 to ch 3<br/>
```
AI1,3
```
ch 1 and ch 3<br/>

An example code to call this function is

```
    def ArduinoAI():
        ser.write(b'AI7,8\n')
        time.sleep(0.01)
        ser_bytes = ser.readline()
        ser_bytes = ser_bytes.decode('utf-8').strip()
        decoded_bytes = ser_bytes
 
```       
- AO: Write the analogue channel<br/>
```
AO2,1v
```
analogue channel 2, number 0-255 corresponding to 0-5V, unit<br/>
An example Python code is 
```
    def ArduinoAO(channel,flag,values):
        if flag == True:
            AO6out = 'AO'+str(channel)+'v'+ str(values) + '\n'
        else:
            AO6out = 'AO'+str(channel)+'v'+'0\n'
        ser.write(b'B')
        time.sleep(0.1)
        ser.write(AO6out.encode('utf-8'))

```
- II: Read the I2C channel<br/>
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
- FB: Start the feedback control<br/>
```
FB1,19,1,2,3
```
FB analogue channel number, setpoint, proportional, integral, differential control parameters<br/>
An example Python code to call this function is
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
- B: Stop the feedback control<br/>
