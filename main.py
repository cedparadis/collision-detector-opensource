# This is a sample Python script.

# Press Maj+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
from flask import Flask, request, jsonify
from flask_cors import CORS
import mysql.connector
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from datetime import datetime

app = Flask(__name__)
CORS(app)
# Connect to your MySQL database
db = mysql.connector.connect(
    host='localhost',
    user='root',
    password='test123',
    database='collision-detector'
)
cursor = db.cursor()

# Create tables if they don't exist
cursor.execute('''
    CREATE TABLE IF NOT EXISTS user_data (
        id VARCHAR(255) PRIMARY KEY,
        name VARCHAR(255),
        phoneNumber VARCHAR(255)
    )
''')
cursor.execute('''
    CREATE TABLE IF NOT EXISTS collision_data (
        numberOfCollisions INT,
        timeOfPreviousCollision DATETIME,
        averageTimeBetweenCollisions FLOAT,
        locationOfLastCollision VARCHAR(255)
    )
''')
db.commit()

def send_email_notification():
    sender_email = "cedparadis99@gmail.com"
    receiver_email = "cedparadis99@gmail.com"
    password = "faxf tfna tbae kyop"

    message = MIMEMultipart()
    message['From'] = sender_email
    message['To'] = receiver_email
    message['Subject'] = "Collision Detected!"

    body = "A collision has been detected. Please take necessary actions."

    message.attach(MIMEText(body, 'plain'))

    try:
        server = smtplib.SMTP('smtp.gmail.com', 587)  # Replace with your SMTP server and port
        server.starttls()
        server.login(sender_email, password)
        server.sendmail(sender_email, receiver_email, message.as_string())
        server.quit()
        print("Email notification sent successfully!")
    except Exception as e:
        print("Error sending email notification:", str(e))


# API endpoint to receive data from embedded system
@app.route('/api/data', methods=['POST'])
def receive_data():
    data = request.json

    collision_data = data.get('collisions', {})

    # Store collision data in the database
    cursor.execute('''
        INSERT INTO collision_data (numberOfCollisions, timeOfPreviousCollision, averageTimeBetweenCollisions, locationOfLastCollision)
        VALUES (%s, %s, %s, %s)
    ''', (
        collision_data.get('numberOfCollisions'),
        collision_data.get('timeOfPreviousCollision'),
        collision_data.get('averageTimeBetweenCollisions'),
        collision_data.get('locationOfLastCollision')
    ))

    db.commit()

    send_email_notification()

    return jsonify({'message': 'Data received and stored successfully'}), 200

# API endpoint to retrieve data for the embedded system
@app.route('/api/data', methods=['GET'])
def get_data():
    try:
        # Fetch data from the database
        cursor.execute("SELECT * FROM user_data")
        user_data = cursor.fetchall()

        # Fetch the last row from collision_data table, ordered by numberOfCollisions in descending order
        cursor.execute("SELECT * FROM collision_data ORDER BY numberOfCollisions DESC LIMIT 1")
        last_collision_data = cursor.fetchone()

        # Format the data as needed before sending
        data = {
            "user": {
                "id": user_data[0][0],  # Assuming the ID is the first column
                "name": user_data[0][1],  # Assuming the name is the second column
                "phoneNumber": user_data[0][2]  # Assuming the phone number is the third column
            },
            "last_collision": {
                "numberOfCollisions": last_collision_data[0],
                "timeOfPreviousCollision": str(last_collision_data[1]),  # Convert to string for Arduino compatibility
                "averageTimeBetweenCollisions": last_collision_data[2],
                # Convert to float for Arduino compatibility
                "locationOfLastCollision": last_collision_data[3]
            }
        }

        return jsonify(data), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    app.run(host='0.0.0.0')

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
