#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

String ssid = "Mariano_2G";
String password = "20061977";
String id = "0";
String idRetorno = "";
String idIot = "0";
String nameIot = "ControlePilotoPradoVelho";
String tipoIot = "IOT";
String nomeConector = "ControlePilotoPradoVelho";
String atualEstado = "OFF";
//String server = "34.69.67.128";
//String server = "2001:1284:f013:b957:e911:cd0d:2220:24b0";
String server = "192.168.0.103";
String jSon = "";
int porta = 27015;
String usuario = "Matinhos";
String senha = "M@r0403";
int status = WL_IDLE_STATUS;
int hora = 0;
int minuto = 0;
int segundo = 0;
int dia = 0;
int mes = 0;
int ano = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", -3 * 3600, 60000);

String mostradorRelogio = "";
StaticJsonDocument<1000> doc;
StaticJsonDocument<1000> com;


void setup()
{
  delay(5000);
  pinMode(D3, OUTPUT);
  digitalWrite(D3, HIGH);
  Serial.begin(115200);
  Serial.println();

  Serial.print("Connecting to " + ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  timeClient.begin();
}


void loop()
{
  WiFiClient client;
  client.setTimeout(10000);

  Serial.print("\n" + retornoHoraData() + " [Connecting to " + server + " ... ");



  if (client.connect(server, porta))
  {
    Serial.println("connected]");
    log("Logando.");
    client.print(login());

    while (client.connected())
    {
      if (WiFi.status() != WL_CONNECTED) {
        log("Perda conexão Wifi. Tentando recuperar");
        client.stop();
        WiFi.disconnect();
        status = WL_IDLE_STATUS;
        while ( status != WL_CONNECTED) {
          status = WiFi.begin(ssid, password);
          delay(10000);
        }
        return;
      }


      if (client.available())
      {
        log("Aguardando dados");
        String line = client.readStringUntil('\n');
        if (line == "")
        {
          log("Retorno vazio");
          continue;
        }
        //log(line);
        DeserializationError error = deserializeJson(doc, line);
        if (error)
        {
          Serial.print(F("deserializeJson() failed: "));
          log(error.c_str());
        }
        else
        {




          String status = doc["status"];
          Serial.print(retornoHoraData()+" Evento - ");
          Serial.println(status);



          if (status == "FAIL")
          {
            log("Conexão falhou...");
            String erro = doc["erro"];
            log(erro);
            delay(5000);
            client.stop();
            return;
          }
          if (status == "CONECTADO")
          {
            String nome = doc["nome"];
            String getId = doc["id"];
            id = getId;
            Serial.print(retornoHoraData()+" "+ nome + " - ID:");
            Serial.println(id);
            //client.print(alive());

          }
          if (status == "CONTROLLERCOMMAND")
          {
            String getIdIot = doc["iot"]["id"];
            idIot =  getIdIot;
            String jSonRetorno = "";
            String getIdRetorno = doc["id"];
            idRetorno = getIdRetorno;
            log("Executando comando");
            jSonRetorno = executaComando();
            client.print(retorno(jSonRetorno));
          }
        }
      }
      else
      {
        //log("Cliente indisponivel");
        //return ;
        
      }
    }
    client.stop();
  }
  else
  {
    Serial.println("connection failed!]");
    client.stop();
  }
  delay(5000);
}

String login()
{
  String retorno = "";
  doc["id"] = id;
  doc["nome"] = nomeConector;
  doc["usuario"] = usuario;
  doc["senha"]   =  senha;
  doc["iot"]["id"] = 0;
  doc["iot"]["name"] = nameIot;
  doc["status"] = "LOGIN";
  serializeJson(doc, retorno);
  //Serial.print("login json: ");
  //Serial.print(retorno);
  //log("...");
  return retorno + "\r\n";

}

String executaComando()
{
  String json = doc["iot"]["jSon"];

  log(json);

  if (json == "null")
  { log("sem comando");
    return "";
  }

  DeserializationError error = deserializeJson(com, json);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    log(error.c_str());
  }

  else {


    int buttonID = com[0]["buttonID"];
    String tecla    = com[0]["tecla"];
    String statusB  = com[0]["status"];

    Serial.printf("%d - ", buttonID);
    Serial.print(tecla + " -  ");
    log(statusB);

    if (statusB == "READ") {
      String sRet = "";
      StaticJsonDocument<1000> retRead;
      retRead["buttonID"] = 1;
      retRead["status"] = atualEstado;
      retRead["tecla"] = 1;
      serializeJson(retRead, sRet);
      return sRet;

    }

    if (buttonID == 1)
    {
      if (statusB == "OUT")
      {

        if (tecla == "ON")
        {
          log("Ligando...");
          digitalWrite(D3, LOW);
          atualEstado = "ON";
        }
        else if (tecla == "OFF")
        {
          log("Desligando...");
          digitalWrite(D3, HIGH);
          atualEstado = "OFF";
        }

      }
    }
  }
  return "";
}

String retorno(String jSon)
{
  StaticJsonDocument<1000> retJson;
  String retorno = "";
  retJson["id"] = idRetorno;
  retJson["usuario"] = usuario;
  retJson["nome"] = nomeConector;
  retJson["tipo"] = tipoIot;
  retJson["senha"]   =  senha;
  retJson["iot"]["id"] = idIot;
  if (jSon != "")
    retJson["iot"]["jSon"] = jSon;
  retJson["iot"]["name"] = nameIot;
  retJson["status"] = "RETORNO";
  serializeJson(retJson, retorno);
  return retorno + "\n";

}

void log(String texto) {
  Serial.println(retornoHoraData() + " " + texto);
}

String retornoHoraData() {
  timeClient.update();
  return timeClient.getFormattedTime();
}

String alive()
{
  String retorno = "";
  doc["id"] = id;
  doc["usuario"] = usuario;
  doc["nome"] = nomeConector;
  doc["tipo"] = tipoIot;
  doc["senha"]   =  senha;
  doc["iot"]["id"] = idIot;
  doc["iot"]["name"] = nameIot;
  doc["status"] = "ALIVE";
  serializeJson(doc, retorno);
  return retorno + "\r\n";

}
