
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2 // вывод Data подключен ко 2 порту Ардуино

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

SoftwareSerial GSMSerial(10, 11); // RX, TX
String currStr = "";
boolean isStringMessage = false; // Переменная принимает значение True, если текущая строка является сообщением
boolean alarm_on = true; // переменная отвечающая за режим тревоги, труе=включен, фолс=выключен
int i;
String new_curr_Str;

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  GSMSerial.begin(19200);
  sensors.begin(); //Запуск модуля датчика температуры 
  // Настраиваем приём сообщений с других устройств
  // Между командами даём время на их обработку
// delay(10000); //ждем пока ЖСМ поймает сеть
 delay(20000);
 GSMSerial.print("AT+CNMI=1,2,2,1,0\r"); //вкл. перехват смс
 delay(500);
 GSMSerial.print("AT+CMGF=1\r"); //вкл. текстовый режим работы
 delay(300);
// GSMSerial.print("AT+IFC=1, 1\r");
// delay(300);
 GSMSerial.print("AT+CPBS=\"SM\"\r");
// delay(1500);
// GSMSerial.print("AT+CNMI=1,2,2,1,0\r");

   }

/*
 * Функция отправки SMS-сообщения
 * number - номер мобильного телефона +375297464685
 * message - текстовое сообщение
 */
void send_SMS(String phone, String text, SoftwareSerial interface) 
{
  Serial.println("SMS send started");
  interface.println("AT+CMGS=\"" + phone + "\"");
  delay(1000);
  interface.print(text);
  delay(300);
  interface.print((char)26);
  delay(300);
  interface.println("SMS send finish");
  delay(3000);
}

float get_temperature()
{
  sensors.requestTemperatures(); // Отправляем запрос датчикам температуры
  return sensors.getTempCByIndex(0);
  }

void loop() // run over and over
{
if (alarm_on==true){
if (get_temperature()>30){
  send_SMS("+375291660887", "Alarm, temp="+String( get_temperature()), GSMSerial);
  delay(60000);
  }


}

if (!GSMSerial.available())
        return;
    
    char currSymb = GSMSerial.read(); 
    Serial.write(currSymb);
    if ('\r' == currSymb) {
        if (isStringMessage) {
            //если текущая строка - SMS-сообщение,
            //отреагируем на него соответствующим образом
            if (!currStr.compareTo("?")) {
              if (alarm_on){
              send_SMS("+375291660887", "Alarm = ON, Temp = "+String(get_temperature()), GSMSerial);}
              else{
              send_SMS("+375291660887", "Alarm = OFF, Temp = "+String(get_temperature()), GSMSerial);}
              GSMSerial.print("AT+CMGDA=\"DEL ALL\"\n");// удаляем все смс во избежании переполнения
              }              
             //   Сбрасываем СМС-кой показания датчиков
            else if (!currStr.compareTo("!")) {
            //  GSMSerial.print("AT+CUSD=1,\"*120#\"\n"); // делаем запрос баланса
              if (alarm_on){
               alarm_on=false;} else
             {
               alarm_on=true;
              }
              isStringMessage = false;
            }   
        } else {
            if (currStr.startsWith("+CMT")) {
                //если текущая строка начинается с "+CMT",
                //то следующая строка является сообщением
                isStringMessage = true;
            }
            if (currStr.startsWith("+CUSD: 2")) {
              new_curr_Str="";
              for (int i=10; i <=currStr.length()-3 ; i++){
                new_curr_Str=new_curr_Str+currStr[i];
              }
             // send_SMS("+375297464685", new_curr_Str, GSMSerial);
            }
        }
        currStr = "";
    } else if ('\n' != currSymb) {
        currStr += String(currSymb);
    }
 }
