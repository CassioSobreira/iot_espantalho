#include <HTTPClient.h>
#include "thingProperties.h"
#include <ESP32Servo.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// URL do servidor 
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com" 
// Token
#define INFLUXDB_TOKEN "vb-iIWSmI8Lm95_CMmCwgLzWn5w9bQA9jV69gVG0oAlzjghJTAyuvuLFtIFpwUSFaPpdbugltA-kjeaCGnAw7w=="
// Organização 
#define INFLUXDB_ORG "Dev"
// Nome do Bucket
#define INFLUXDB_BUCKET "Espantalho"

// Fuso Horário para o InfluxDB saber a hora certa (Brasil -3h)
#define TZ_INFO "<-03>3"

// --- Objetos do InfluxDB ---
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensor("alarme_log"); 

// --- Definição dos Pinos ---
const int pinoPIR = 13;
const int pinoLED1 = 12;
const int pinoLED2 = 14;
const int pinoServo = 26;
const int pinoBuzzer = 27;

// --- Configuração Servo e Buzzer ---
Servo meuServo;
const int minPulse = 500;
const int maxPulse = 2400; 
const int frequencia = 2000;
const int resolucao = 8; 

// --- Variáveis de Controle ---
int posAtualServo = 0;
int incrementoServo = 5;
unsigned long tempoUltimoPasso = 0;
const int VELOCIDADE_SERVO = 15; 

// --- Timer Geral ---
unsigned long tempoInicioAlarme = 0;
const long DURACAO_ALARME = 8000; 
bool alarmeAtivo = false;

void setup() {
  Serial.begin(9600);
  delay(1500);

  // Configuração Hardware
  pinMode(pinoPIR, INPUT);
  pinMode(pinoLED1, OUTPUT);
  pinMode(pinoLED2, OUTPUT);

  meuServo.setPeriodHertz(50);
  meuServo.attach(pinoServo, minPulse, maxPulse);
  meuServo.write(0); 

  ledcAttach(pinoBuzzer, frequencia, resolucao);

  // --- Inicialização Arduino Cloud ---
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  
  // --- Inicialização InfluxDB e Hora ---
  Serial.println("Sincronizando relogio com a internet...");
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov"); // Pega a hora certa da internet
  
  // Verifica conexão com InfluxDB
  if (client.validateConnection()) {
    Serial.print("Conectado ao InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("Erro InfluxDB: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  ArduinoCloud.update();

  if (digitalRead(pinoPIR) == HIGH) {
    movimento_detectado = true;
  } else {
    movimento_detectado = false;
  }

  // --- GATILHO DO ALARME ---
  if (movimento_detectado && !alarmeAtivo) {
    Serial.println("ALARME INICIADO!");
    alarmeAtivo = true;
    tempoInicioAlarme = millis();
    
    estado_leds = true;     
    estado_buzzer = true;   
    estado_servo = true; 
    
    if (volume_buzzer <= 10) volume_buzzer = 200; 
    
    // --- ENVIA PARA O INFLUXDB ---
    enviarDadosInflux();
    
    atualizarEstaticos(); 
  }

  // --- DURANTE O ALARME (8s) ---
  if (alarmeAtivo) {
    // Servo vai e volta
    if (millis() - tempoUltimoPasso >= VELOCIDADE_SERVO) {
      tempoUltimoPasso = millis(); 
      posAtualServo += incrementoServo;
      if (posAtualServo >= 180) { posAtualServo = 180; incrementoServo = -5; }
      if (posAtualServo <= 0) { posAtualServo = 0; incrementoServo = 5; }
      meuServo.write(posAtualServo);
    }
    
    ledcWrite(pinoBuzzer, volume_buzzer);

    if (millis() - tempoInicioAlarme >= DURACAO_ALARME) {
      alarmeAtivo = false;
      estado_leds = false;
      estado_buzzer = false; 
      estado_servo = false; 
      
      posAtualServo = 0;
      meuServo.write(0);
      ledcWrite(pinoBuzzer, 0);
      digitalWrite(pinoLED1, LOW);
      digitalWrite(pinoLED2, LOW);
    }
  } else {
    atualizarEstaticos();
  }
}

void atualizarEstaticos() {
  if (estado_leds) { digitalWrite(pinoLED1, HIGH); digitalWrite(pinoLED2, HIGH); }
  else { digitalWrite(pinoLED1, LOW); digitalWrite(pinoLED2, LOW); }

  if (!alarmeAtivo) {
    if (estado_buzzer) ledcWrite(pinoBuzzer, volume_buzzer);
    else ledcWrite(pinoBuzzer, 0);
    
     if (estado_servo) meuServo.write(180);
     else meuServo.write(0);
  }
}

// --- FUNÇÃO DE ENVIO DEFINITIVA ---
void enviarDadosInflux() {
  sensor.clearTags();   
  sensor.clearFields(); 
  
  sensor.addTag("dispositivo", "esp32_espantalho");
  sensor.addTag("local", "jardim"); 

  sensor.addField("ativado", 1); 
  sensor.addField("volume_som", volume_buzzer);
  
  Serial.print("Enviando dados: ");
  Serial.println(client.pointToLineProtocol(sensor));

  if (!client.writePoint(sensor)) {
    Serial.print("Falha ao enviar InfluxDB: ");
    Serial.println(client.getLastErrorMessage());
  } else {
    Serial.println("Dados salvos no InfluxDB com sucesso!");
  }
}

void onEstadoLedsChange()   { atualizarEstaticos(); }
void onEstadoBuzzerChange() { atualizarEstaticos(); }
void onEstadoServoChange()  { atualizarEstaticos(); }
void onVolumeBuzzerChange() { atualizarEstaticos(); }
