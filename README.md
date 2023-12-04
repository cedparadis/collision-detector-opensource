# Third Eye Vision - IoT Collision Detector

## Overview
The collision detector system is an embedded system used for visually impaired individuals to walk around and detect obstacles to then warn the user with an increasing warning sound about the distance to the object. This project is a mix of hardware and software integration.

## Devices

### TTGO ESP32
This microcontroller is the brain of this project. It controls the ultrasonic sensor, output sound to the buzzer, creates a web server to display data, and communicate to an API server.

#### Features:
- **WiFi Connectivity**: Connects to the network for remote communication.
- **Ultrasonic sensor Control**: Ping object with a sound wave to return the distance
- **Buzzer** : Output warning sounds that reflect the distance

## TTGO ESP32 Web Server
 A web server that receives data from the device and enables user interaction through a web interface. (To be use by the person helping the one using the device)

#### Features:
- **Real time distance monitoring** : Allows user to see the distance to an object as well as the warning for it in real time through WebSocket technology.
- **Collision History** : The user to see the number of collisions since the usage of the device
- **Collision Data Analytics**: Displays newest analysis from data in a database such as location of last collision, average time between collision, user's data, total number of collisions, etc.

## API server (Flask)
An API server was built using Flask to store collision data to a database (MySql) and retrieve data from the database to the web page for statistical analytics

### Features:
- **POST request** : Sends data about collisions to the database
- **GET request**: Retrieves latest data on collisions as well as the user's data
- **Collision detection Notification**: Sends an email notification to someone helping the user that a collision happened.

## Installation 

## Hardware Setup
1. **Connect the TTGO Board**
2. **Connect the ultrasonic sensor**: Connect it to the TTGO and calibrate the distance
3. **Connect the buzzer**"  Connect the buzzer to the TTGO and adjust the frequency of the sound

## Software Configuration
1.1. **Upload to the TTGO**: Upload the provided code to the TTGO ESP32 board
2. **Environment Configuration**: Set up WiFi credentials and server URLs in the TTGO code.
3. **Server Setup**: Deploy the Flask server on a suitable hosting platform or locally. 
4. **Email Notifications Setup**: Configure smtlib for email notifications. This involves setting up your Gmail account to work with Flask. You'll need to enable App password in your Gmail settings.

## Credits and Acknowledgments
This project leverages several open-source libraries and platforms, including:
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32) for programming the TTGO ESP32 module.
- [NewPing](https://github.com/eliteio/Arduino_New_Ping) for converting ultrasonic sensor ping to distance.
- [ArduinoJson](https://arduinojson.org/) for sending http request
- [WebSocketsServer](https://github.com/Links2004/arduinoWebSockets/tree/master) for sending real time data
- [Flask](https://flask.palletsprojects.com/) for api web server
- [MySql](https://www.mysql.com/) for the database

## License
This project is licensed under the MIT License - see the `LICENSE` file for details.
