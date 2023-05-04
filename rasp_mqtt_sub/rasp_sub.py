import paho.mqtt.client as mqtt
import time
from datetime import datetime
import mysql.connector

# funkcja - nawiązanie połączenia z serwerem MQTT
def on_connect(client, userdata, flags, rc):
   global flag_connected
   flag_connected = 1               # flaga połączenia = 1
   client_subscriptions(client)     # ustawienie subskrybcji
   print("Connected to MQTT server")

# funkcja - przerwanie połączenia z serwerem MQTT
def on_disconnect(client, userdata, rc):
   global flag_connected
   flag_connected = 0               # flaga połączenia = 0
   print("Disconnected from MQTT server")
   
# dane do połączenia się z bazą danych MySql
mydb = mysql.connector.connect(
  host="localhost",
  user="mysql_user",
  password="123123",
  database="projekt"
)

# kursor służący do łączenia z bazą danych
mycursor = mydb.cursor()

# funkcja skalująca odczytaną wartość z przetwornika ADC 10-bitowego
# na wartości 0-10 bardziej intuicyjne
def light_scale(light):     # określa natężenie światła
    scale = int((light-100)/30)
    if(scale < 0): scale = 0
    return scale

# callback - wykonany gry wystąpi odbiór danych z ESP nr 1
def callback_esp32_sensor1(client, userdata, msg):
    print('ESP sensor1 data: ', msg.payload.decode('utf-8'))    # informacja wyświetlona w konsoli

    # utworzenie zmiennych daty, czasu oraz temperatury, czasu odebranego z ESP
    sql = "INSERT INTO esp1 VALUES (%s, %s, %s, %s)"
    now = datetime.now()
    actual_time = now.strftime('%H:%M:%S')
    actual_date = now.strftime('%Y-%m-%d')
    split_words = msg.payload.decode('utf-8').split("#")
    val = (actual_date, actual_time, split_words[0], light_scale(int(split_words[1])))

    mycursor.execute(sql, val)  # wykonanie zapytania do bazy danych MySql
    mydb.commit()

# callback - wykonany gry wystąpi odbiór danych z ESP nr 2
def callback_esp32_sensor2(client, userdata, msg):
    print('ESP sensor2 data: ', msg.payload.decode('utf-8'))    # informacja wyświetlona w konsoli

    # utworzenie zmiennych daty, czasu oraz temperatury, czasu odebranego z ESP
    sql = "INSERT INTO esp2 VALUES (%s, %s, %s, %s)"
    now = datetime.now()
    actual_time = now.strftime('%H:%M:%S')
    actual_date = now.strftime('%Y-%m-%d')
    split_words = msg.payload.decode('utf-8').split("#")
    val = (actual_date, actual_time, split_words[0], light_scale(int(split_words[1])))

    mycursor.execute(sql, val)   # wykonanie zapytania do bazy danych MySql
    mydb.commit()

# callback - wykonany gry wystąpi odbiór danych z Raspberry Pi
def callback_rpi_broadcast(client, userdata, msg):
    print('RPi Broadcast message:  ', str(msg.payload.decode('utf-8')))

# definiowanie subskrypcji klientów MQTT do określonych tematów
def client_subscriptions(client):
    client.subscribe("esp32/#")         # odbiór wiadomości z każdego tematu "esp32/..."
    client.subscribe("rpi/broadcast")   # odbiór wiadomości z każdego tematu "rpi/broadcast"

client = mqtt.Client("rpi_client1")     # utwórz nowego klienta MQTT o nazwie "rpi_client1"
flag_connected = 0

# przypisanie funkcji do zmiennych w bibliotece MQTT
client.on_connect = on_connect
client.on_disconnect = on_disconnect

# wyczyść tabele esp1 i esp2 w bazie danych
sql = "DELETE FROM esp1"
mycursor.execute(sql)
sql = "DELETE FROM esp2"
mycursor.execute(sql)

# przypisanie tematów do callbacków
client.message_callback_add('esp32/sensor1', callback_esp32_sensor1)
client.message_callback_add('esp32/sensor2', callback_esp32_sensor2)
client.message_callback_add('rpi/broadcast', callback_rpi_broadcast)
client.connect('192.168.160.125',1883)  # adres IP serwera / localhost

# utwórz nowy wątek
client.loop_start()             # uruchomienie w tle pętli sieciowej klienta MQTT
client_subscriptions(client)    # start subskrybcji
print("......client setup complete............")


while True:
    time.sleep(4)       # informowanie gdy uracono połączenie z serwerem
    if (flag_connected != 1):
        print("trying to connect MQTT server..")
        
