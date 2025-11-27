#include "thingProperties.h"
#include <ESP32Servo.h>

// --- Definição dos Pinos ---
const int pinoPIR = 13;
const int pinoLED1 = 12;
const int pinoLED2 = 14;
const int pinoServo = 26;
const int pinoBuzzer = 27;

// --- Configuração Servo ---
Servo meuServo;
const int minPulse = 500;   // Ajuste para ESP32
const int maxPulse = 2400;  // Ajuste para ESP32

// --- Variáveis de Controle de Movimento do Servo ---
int posAtualServo = 0;      // Posição atual em graus
int incrementoServo = 5;    // Quantos graus move por vez (aumente para ser mais rápido)
unsigned long tempoUltimoPasso = 0;
const int VELOCIDADE_SERVO = 15; // Tempo em ms entre cada movimento (menor = mais rápido)

// --- Configuração Buzzer ---
const int frequencia = 2000;
const int resolucao = 8; 

// --- Timer Geral do Alarme ---
unsigned long tempoInicioAlarme = 0;
const long DURACAO_ALARME = 8000; // 8 Segundos
bool alarmeAtivo = false;

void setup() {
  Serial.begin(9600);
  delay(1500);

  // Hardware init
  pinMode(pinoPIR, INPUT);
  pinMode(pinoLED1, OUTPUT);
  pinMode(pinoLED2, OUTPUT);

  // Servo init
  meuServo.setPeriodHertz(50);
  meuServo.attach(pinoServo, minPulse, maxPulse);
  meuServo.write(0); // Inicia fechado

  // Buzzer init
  ledcAttach(pinoBuzzer, frequencia, resolucao);

  // Cloud init
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();

  // 1. Leitura do Sensor
  if (digitalRead(pinoPIR) == HIGH) {
    movimento_detectado = true;
  } else {
    movimento_detectado = false;
  }

  // 2. Início do Disparo (Gatilho)
  if (movimento_detectado && !alarmeAtivo) {
    Serial.println("ALARME: Movimento! Iniciando ciclo de 8s...");
    alarmeAtivo = true;
    tempoInicioAlarme = millis();
    
    // Atualiza Cloud para mostrar que está tudo ligado
    estado_leds = true;     
    estado_buzzer = true;   
    estado_servo = true; 
    
    // Volume de segurança
    if (volume_buzzer <= 10) volume_buzzer = 200; 
    
    // Liga imediatamente LEDs e Som
    atualizarEstaticos(); 
  }

  // 3. Durante o Alarme (Os 8 Segundos)
  if (alarmeAtivo) {
    
    // A) Lógica de Movimento do Servo (Vai e Vem)
    if (millis() - tempoUltimoPasso >= VELOCIDADE_SERVO) {
      tempoUltimoPasso = millis(); // Reseta cronômetro do passo
      
      // Move a posição
      posAtualServo += incrementoServo;

      // Se bateu nos limites (0 ou 180), inverte a direção
      if (posAtualServo >= 180) {
        posAtualServo = 180;
        incrementoServo = -5; // Começa a descer
      }
      if (posAtualServo <= 0) {
        posAtualServo = 0;
        incrementoServo = 5;  // Começa a subir
      }

      // Aplica ao motor
      meuServo.write(posAtualServo);
    }
    
    // B) Mantém o som tocando (caso mude o volume na dashboard)
    ledcWrite(pinoBuzzer, volume_buzzer);

    // C) Verifica se acabou o tempo (8s)
    if (millis() - tempoInicioAlarme >= DURACAO_ALARME) {
      Serial.println("FIM DO CICLO.");
      alarmeAtivo = false;
      
      // Reseta variáveis da nuvem
      estado_leds = false;
      estado_buzzer = false; 
      estado_servo = false; 
      
      // Reseta Hardware para posição de descanso
      posAtualServo = 0;
      meuServo.write(0);
      ledcWrite(pinoBuzzer, 0);
      digitalWrite(pinoLED1, LOW);
      digitalWrite(pinoLED2, LOW);
    }
  } else {
    // Se o alarme NÃO está ativo, obedece aos comandos manuais da Dashboard
    // Exemplo: Se você clicar no botão do LED no celular, ele acende
    atualizarEstaticos();
  }
}

// Função para controlar coisas que não ficam mudando toda hora (LEDs e Buzzer ON/OFF)
void atualizarEstaticos() {
  // LEDs
  if (estado_leds) {
    digitalWrite(pinoLED1, HIGH);
    digitalWrite(pinoLED2, HIGH);
  } else {
    digitalWrite(pinoLED1, LOW);
    digitalWrite(pinoLED2, LOW);
  }

  // Buzzer Manual (se alarme não estiver rodando)
  if (!alarmeAtivo) {
    if (estado_buzzer) {
      ledcWrite(pinoBuzzer, volume_buzzer);
    } else {
      ledcWrite(pinoBuzzer, 0);
    }
  }
  
  // Se quiser controlar o servo manualmente quando NÃO tem ladrão
  if (!alarmeAtivo) {
     if (estado_servo) meuServo.write(180);
     else meuServo.write(0);
  }
}

// Callbacks (Gatilhos da Nuvem)
void onEstadoLedsChange()   { atualizarEstaticos(); }
void onEstadoBuzzerChange() { atualizarEstaticos(); }
void onEstadoServoChange()  { atualizarEstaticos(); }
void onVolumeBuzzerChange() { atualizarEstaticos(); }