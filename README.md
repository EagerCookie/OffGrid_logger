# OffGrid_logger
This project aimed to provide information from both - Off Grid inverter and BMS.
Project is under development and grows as fast as I have free time for it.

Additional notes:

System builded on ESP8266
-Via RS232 (native) request QPIGS data from inverter (279 cahrs/bytes+checksum)
-Via GPS port of BMS request 109 raw bytes (depends on how many serial batteries in the serial connection)

-Built-in HTML page that using websockets to display data
- Has API endpoints to provide data
- Pushing data to Google sheets take around 1-2s that leads to totally bloking all operations of microcontroller.
So websockets is solution for such type of problem.
- Use hardware UART connection for BMS (115200 baud) and software for inverter (2700 baud)
Software UART has limitation

TODO:
General:
-Upload Google AppsScript script file
-Upload SVG templates for graphical representation
-Upload python application client (OLD - use endpoint API for requesting data)
 
Software:
- Create graphical representation for html page (svg + animation)
- Make initialization procedure(Start SoftAP, input wifi credentials etc.)
- Save configuration into EEPROM
- Make sure that disconnection any of equipment doesn't lead to stucking in the loop waiting for the response
  

Hardware:
- Use GPS power source to supply logger / Use inverter's native RS232 as a power source (ONLY data lines are persist)
- Make wiring schematic and upload

  
