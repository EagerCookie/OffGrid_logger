import requests
import tkinter as tk
from tkinter import Label, font


# Function to send a GET request to the specified URL
def send_get_request():
    url = "http://192.168.1.66/data"
    try:
        response = requests.get(url)
    
    except ValueError:
        print(f"Error while making request {ValueError}")
        pass
    
    
    if response.status_code == 200:
        data = response.text
        process_data(data)
    # Schedule the next GET request after a delay (e.g., 2 seconds)
    window.after(100, send_get_request)

# Function to process the received data
def process_data(data):
    values = data.split()
    if len(values) >= 20:
                # Convert values to integers and remove leading zeros
        Power_VA = str(int(values[4]))
        Power_W = str(int(values[5]))
        OutP_percentage = str(int(values[6]))
        BusVoltage = str(int(values[7]))
        BatteryVoltage = str(float(values[8]))
        BattCharge = str(int(values[9]))
        BattCapacity = str(int(values[10]))
        Temp = str(int(values[11]))
        PVBattCharge = str(int(values[12]))
        PV_Voltage = str(float(values[13]))
        SCC_Bat = str(float(values[14]))
        BattDischarge = str(int(values[15]))
        Solar_W = str(int(values[19]))
        
        # Display values number 5, 6, 12, and 20
        display_values(Power_VA, Power_W, Temp, Solar_W, OutP_percentage, BusVoltage, BatteryVoltage, BattCharge, PVBattCharge, PV_Voltage, SCC_Bat, BattDischarge, BattCapacity)

# Function to update the GUI with the specified values
def display_values(value1, value2, value3, value4, value5, value6, value7, value8, value9, value10, value11, value12, value13):
    label_value1.config(text=f"Power : {value1} VA")
    label_value2.config(text=f"Power : {value2} W")
    label_value3.config(text=f"Temp : {value3}C")
    label_value4.config(text=f"Solar : {value4} W")
    label_value5.config(text=f"MaxPower : {value5} %")
    label_value6.config(text=f"BusVoltage : {value6} V")
    label_value7.config(text=f"BatteryVoltage : {value7} V")
    label_value8.config(text=f"BatteryChargeCurrent : {value8} A")
    label_value9.config(text=f"PVBatteryCharge : {value9} A")
    label_value10.config(text=f"PV_Voltage : {value10} V")
    label_value11.config(text=f"SCC_Bat : {value11} V")
    label_value12.config(text=f"BatteryDischarge : {value12} A")
    label_value13.config(text=f"BatteryCapacity : {value13} %")
# Create a GUI window
window = tk.Tk()
window.title("Data Display")

label_font = font.Font(size=24)

# Create labels to display values
label_value1 = Label(window, text="Value 5:", font=label_font)
label_value2 = Label(window, text="Value 6:", font=label_font)
label_value3 = Label(window, text="Value 12:", font=label_font)
label_value4 = Label(window, text="Value 20:", font=label_font)
label_value5 = Label(window, text="Value 20:", font=label_font)
label_value6 = Label(window, text="Value 20:", font=label_font)
label_value7 = Label(window, text="Value 20:", font=label_font)
label_value8 = Label(window, text="Value 20:", font=label_font)
label_value9 = Label(window, text="Value 20:", font=label_font)
label_value10 = Label(window, text="Value 20:", font=label_font)
label_value11 = Label(window, text="Value 20:", font=label_font)
label_value12 = Label(window, text="Value 20:", font=label_font)
label_value13 = Label(window, text="Value 20:", font=label_font)



label_value1.pack()
label_value2.pack()
label_value3.pack()
label_value4.pack()
label_value5.pack()
label_value6.pack()
label_value7.pack()
label_value8.pack()
label_value9.pack()
label_value10.pack()
label_value11.pack()
label_value12.pack()
label_value13.pack()

# Create a thread to periodically send GET requests
import threading
interval = 2  # Interval in seconds
thread = threading.Thread(target=lambda: send_get_request())
thread.daemon = True
thread.start()

# Start the GUI main loop
window.mainloop()
