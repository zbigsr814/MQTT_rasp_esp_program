# MQTT_rasp_esp_program

Program jest drugą częścią mojej pracy magisterskiej, wraz z django_aplikacją.

Programy dla Raspberry Pi i ESP3266. Programy realizują komunikację przez protokół MQTT między urządzeniami. Dane z czujników temperatury i naświetlenia odbierane są przez ESP. Przez protokół MQTT wysyłane są do Raspberry Pi. RPi wysyła zgromadzone dane do bazy danych MySQL na serwerze HTTP Apache2 na RPi.

URUCHOMIENIE PROJEKTU
Program na ESP3266 przesyłamy przez program Arduino IDE. W programie należy dokonać zmiany adresu IP brokera MQTT, w naszym przypadku jest to IP Raspberry Pi.
Programy na Raspberry Pi należy uruchomić w interprerze pythonowym. Programy najlepiej uruchomić w tle: python rasp_sub.py &
W plikach należy wpisać adres IP brokera Raspberry Pi oraz dane do połączenia się z bazą danych MySql.
