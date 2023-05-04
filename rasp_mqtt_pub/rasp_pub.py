import time
import paho.mqtt.client as mqtt

# informacja w terminalu o publikowanym temacie
def on_publish(client, userdata, mid):
    print("message published")

client = mqtt.Client("rpi_client2")     # utwórz nowego klienta MQTT o nazwie "rpi_client2"
client.on_publish = on_publish          # przypisanie funkcji do zmiennych w bibliotece MQTT
client.connect('192.168.160.125',1883)  # adres IP serwera / localhost

client.loop_start()                     # uruchomienie w tle pętli sieciowej klienta MQTT

k=0
# publikowanie wiadomości na temat "rpi/broadcast"
# w wiadomości wysyłana wartość zmiennej k inkrementowana od 1 do 20 co 2 sekundy
while True:
    k=k+1
    if(k>20):
        k=1 
        
    try:
        msg =str(k)
        pubMsg = client.publish(
            topic='rpi/broadcast',
            payload=msg.encode('utf-8'),
            qos=0,
        )
        pubMsg.wait_for_publish()
        print(pubMsg.is_published())
    
    except Exception as e:
        print(e)
        
    time.sleep(2)
