#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <String>
#include <stdio.h>
#include <IRremote.hpp>

#define DELAY_AFTER_SEND 2000
#define LEDA 13
#define LEDV 26
#define LEDVERM 25
#define WIFI_SSID "Connection Error"
#define WIFI_PASSWORD "31313131"
#define API_KEY "AIzaSyAyut07XIb2UhfdMEim0V1hMRAW7B43umc"
#define FIREBASE_PROJECT_ID "urban-voice-5bab7"
#define USER_EMAIL "tiopaulo@gmail.com"
#define USER_PASSWORD "123456"

int vetor[100] = {} ;

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

String documentPath;

struct Dados {
  String mensagem;
  int valor;
};

int stringToInt(const String& str) {
    int result = 0;

    // Loop através dos caracteres da string
    for (int i = 0; i < str.length(); ++i) {
        char c = str.charAt(i);
        // Multiplica o resultado atual por 10 e adiciona o valor do dígito atual
        result = result * 10 + (c - '0');
    }

    return result;
}


void write(float temperatura) {

  String documentPath = "Votos1/Logs";
  FirebaseJson content;
  if (temperatura != 0) {
    content.set("fields/log/stringValue", String(temperatura));
  } else {
    //content.set("fields/log/stringValue", "Empatado");
  }

  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "")) {
    Serial.println("Documento criado com sucesso!");
    digitalWrite(LEDV, HIGH) ;
    digitalWrite(LEDA, HIGH) ;
    delay(1500) ;
    digitalWrite(LEDV, LOW) ;
    digitalWrite(LEDA, LOW) ;
    
  } else {
    Serial.println("Falha ao criar documento:");
    digitalWrite(LEDVERM, HIGH) ;
    delay(1500) ;
    digitalWrite(LEDVERM, LOW) ;
    Serial.println(fbdo.errorReason());
  }

}

int encontrarModa(int vetor[], int tamanho) {
    int frequencia[101] = {0}; // Array para armazenar as frequências de cada elemento
    int i, moda, maxFrequencia, contadorModas;

    // Calculando as frequências de cada elemento
    for (i = 0; i < tamanho; i++) {
        frequencia[vetor[i]]++;
    }

    // Encontrando o elemento com maior frequência (moda)
    maxFrequencia = 0;
    contadorModas = 0;
    for (i = 1; i <= 100; i++) { // começa em 1 para evitar 0, a menos que seja especificado que 0 está no vetor
        if (frequencia[i] > maxFrequencia) {
            maxFrequencia = frequencia[i];
            moda = i;
            contadorModas = 1; // Reinicia o contador de modas
        } else if (frequencia[i] == maxFrequencia) {
            contadorModas++;
        }
    }

    if (contadorModas > 1) {
        write(0) ;
        return 0 ;
    } else {
        write(moda);
        return moda ;
    }
}

int readAndChecagem() {

  String documentPath = "Votos1/Votos";
  int verificador = 0 ;
  int counter = 0 ;
  int vetorChecagem[100] = {} ;
  int i;

  for (i = 1 ; verificador != 1 ; i++) {
    if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str())) {
    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK && fbdo.payload().length() > 0){
      digitalWrite(LEDV, HIGH) ;
      delay(1500) ;
      digitalWrite(LEDV, LOW) ;

      // Processa os dados lidos
      Dados dados;
      FirebaseJson json;
      json.setJsonData(fbdo.payload());

      // Extrai os campos do documento
      FirebaseJsonData jsonData;

      if (json.get(jsonData, "fields/" + String(i) + "/stringValue") && jsonData.type == "string") {
        dados.mensagem = jsonData.stringValue;
      } 
      
      String str = dados.mensagem ;
      if (str != "0") {
      vetorChecagem[i-1] = stringToInt(str) ;// Colocando os dados em um vetor
      //Serial.println(vetorChecagem[i]) ;
      } else {
        verificador = 1 ;
      }

    } else { // Caso a leitura de errado
      Serial.println("Deu errado") ;
      digitalWrite(LEDVERM, HIGH) ;
      delay(1500) ;
      digitalWrite(LEDVERM, LOW) ;
    }
  } else { // Caso a leitura de errado
    Serial.println("Deu errado dnv") ;
    digitalWrite(LEDVERM, HIGH) ;
    delay(1500) ;
    digitalWrite(LEDVERM, LOW) ;
  }

  }
  // Checar se tem pelo menos dez votos
  vetorChecagem[i+1] = 0 ;

  for (int j=0; vetorChecagem[j] != 0; j++){
    counter = counter + 1 ;
  }

  return counter ;

}

void readAndLogic(int vetor[]) {

  String documentPath = "Votos1/Votos";
  int verificador = 0 ;
  int counter = 0 ;
  int i;

  for (i = 1 ; verificador != 1 ; i++) {
    if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str())) {
    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK && fbdo.payload().length() > 0) {
      digitalWrite(LEDV, HIGH) ;
      delay(1500) ;
      digitalWrite(LEDV, LOW) ;

      // Processa os dados lidos
      Dados dados;
      FirebaseJson json;
      json.setJsonData(fbdo.payload());

      // Extrai os campos do documento
      FirebaseJsonData jsonData;

      if (json.get(jsonData, "fields/" + String(i) + "/stringValue") && jsonData.type == "string") {
        dados.mensagem = jsonData.stringValue;
      } 
      
      String str = dados.mensagem ;
      if (str != "0") {
      vetor[i-1] = stringToInt(str) ;// Colocando os dados em um vetor
      Serial.println(vetor[i-1]) ;
      counter = counter + 1 ;
      } else {
        verificador = 1 ;
      }

    } else { // Caso a leitura de errado
        Serial.println("Deu errado") ;
        digitalWrite(LEDVERM, HIGH) ;
        delay(1500) ;
        digitalWrite(LEDVERM, LOW) ;
    }
  } else { // Caso a leitura de errado
      Serial.println("Deu errado dnv") ;
      digitalWrite(LEDVERM, HIGH) ;
      delay(1500) ;
      digitalWrite(LEDVERM, LOW) ;
  }

  }
  
  int moda = encontrarModa(vetor, counter) ;
  Serial.println(moda) ;
  
  setTemp(moda-18) ; // subtraindo 18 pois o 18 é a base
}

int zerarDb(int vetor[]) {
  String documentPath = "Votos1/Votos";
  FirebaseJson content;

  for (int i = 1 ; i < 30 ; i++) {
    vetor[i] = 0 ; // zerando o vetor tbm

    content.set("fields/"+ String(i) + "/stringValue", "0");

    if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "")) {
    Serial.println("Documento criado com sucesso!");
    digitalWrite(LEDA, HIGH) ;
    delay(500);
    digitalWrite(LEDA, LOW) ;

  } else {
    Serial.println("Falha ao criar documento:");
    Serial.println(fbdo.errorReason());
  }

  }
  delay(1000) ;

}

void setTemp(int sCommand) {
    IrSender.sendNEC(69, sCommand, 0);
    delay(DELAY_AFTER_SEND);
}

void setup() {

  Serial.begin(115200);

  WiFi.setSleep(false) ;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }


  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;  

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);


    uint8_t tSendPin = 27;
    IrSender.begin(tSendPin, false, false); 

    Serial.print(F("Send IR signals at pin "));
    Serial.println(tSendPin);

#if !defined(SEND_PWM_BY_TIMER)
    /*
     * Print internal software PWM signal generation info
     */
    IrSender.enableIROut(38); // Call it with 38 kHz just to initialize the values printed below
    Serial.print(F("Send signal mark duration is "));
    Serial.print(IrSender.periodOnTimeMicros);
    Serial.print(F(" us, pulse narrowing correction is "));
    Serial.print(IrSender.getPulseCorrectionNanos());
    Serial.print(F(" ns, total period is "));
    Serial.print(IrSender.periodTimeMicros);
    Serial.println(F(" us"));
#endif

  pinMode(LEDVERM, OUTPUT) ;
  pinMode(LEDA, OUTPUT) ;
  pinMode(LEDV, OUTPUT) ;
  
}

void loop() {

  int contagemVotosChecagem = readAndChecagem() ;
  if (contagemVotosChecagem > 9) {
    readAndLogic(vetor) ;
    zerarDb(vetor) ;
    Serial.println("Database zerada, a sessão irá começar em breve...") ;
  } else {
    Serial.println("Esperando por número mínimo de votos") ;
    delay(10000) ;
  }
  
}


