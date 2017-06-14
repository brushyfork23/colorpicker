/* For use with the colorview Arduino example sketch 
   Update the Serial() new call to match your serial port
   e.g. COM4, /dev/usbserial, etc!
*/


import processing.serial.*;
import java.awt.datatransfer.*;
import java.awt.Toolkit;

Serial port;
 
void setup(){
 // set the window size:
  size(400, 300);
 port = new Serial(this, "/dev/tty.SLAB_USBtoUART", 9600); //remember to replace COM20 with the appropriate serial port on your computer
 background(0);
}
 
 
String buff = "";

int wRed, wGreen, wBlue,
prevRed, prevGreen, prevBlue;
 
int xPos = 1;         // horizontal position of the graph

void draw(){
 // check for serial, and process
 while (port.available() > 0) {
   serialEvent(port.read());
 }
}
 
void serialEvent(int serial) {
 if(serial != '\n') {
   buff += char(serial);
 } else {
   println(buff);
   
   int cRed = buff.indexOf("R");
   int cGreen = buff.indexOf("G");
   int cBlue = buff.indexOf("B");
   
   if(cRed >=0){
     String val = buff.substring(cRed+3);
     val = val.split("\t")[0]; 
     wRed = Integer.parseInt(val.trim());
   } else { return; }
   
   if(cGreen >=0) {
     String val = buff.substring(cGreen+3);
     val = val.split("\t")[0]; 
     wGreen = Integer.parseInt(val.trim());
   } else { return; }
   
   if(cBlue >=0) {
     String val = buff.substring(cBlue+3);
     val = val.split("\t")[0]; 
     wBlue = Integer.parseInt(val.trim());
   } else { return; }
   
   print("Red: "); print(wRed);
   print("\tGrn: "); print(wGreen);
   print("\tBlue: "); println(wBlue);


  // draw red
  stroke(255, 0, 0);
  line(xPos, height - prevRed, xPos, height - wRed);
  
  // draw green
  stroke(0, 255, 0);
  line(xPos, height - prevGreen, xPos, height - wGreen);
  
  // draw blue
  stroke(0, 0, 255);
  line(xPos, height - prevBlue, xPos, height - wBlue);
  /*
  // draw red
  stroke(255, 0, 0);
  line(xPos, 800, xPos, 800 - wRed);
  
  // draw green
  stroke(0, 255, 0);
  line(xPos, 520, xPos, 520 - wGreen);
  
  // draw blue
  stroke(0, 0, 255);
  line(xPos, 260, xPos, 260 - wBlue);
*/
  // at the edge of the screen, go back to the beginning:
  if (xPos >= width) {
    xPos = 0;
    background(0);
  } else {
    // increment the horizontal position:
    xPos++;
  }
  
  prevRed = wRed;
  prevGreen = wGreen;
  prevBlue = wBlue;
   buff = "";
 }
}