🦅 Espantalho Inteligente IoT (Smart Scarecrow)
Sistema de monitoramento e dissuasão baseado em ESP32 com controle via Arduino Cloud e registro de dados no InfluxDB.

📖 Sobre o Projeto
Este projeto é um sistema de segurança (ou espantalho eletrônico) que detecta presença através de um sensor PIR. Ao identificar movimento, o sistema executa um ciclo de alerta de 8 segundos que envolve:

  1.Acionamento de LEDs visuais.
  
  2.Disparo de um Buzzer (com controle de volume PWM).
  
  3.Movimentação de varredura (vai e vem) de um Servo Motor.
  
  4.Além da reação física, o sistema envia dados de telemetria em tempo real para o Arduino IoT Cloud (para controle remoto) e grava o histórico de eventos no InfluxDB para análise de dados.

🛠️ Hardware Utilizado
  ESP32	1	Placa de desenvolvimento (DevKit V1)
  Sensor PIR	1	Sensor de movimento infravermelho (HC-SR501)
  Servo Motor	1	Micro Servo (SG90)
  Buzzer	1	Buzzer Passivo (5V)
  LEDs	2	Cores de sua preferência
  Resistores	2	220Ω ou 330Ω (para os LEDs)
  Fonte	1	Cabo USB

💻 Software e Bibliotecas
O projeto foi desenvolvido na Arduino IDE. As seguintes bibliotecas são necessárias:
  
  WiFi.h & HTTPClient.h: Nativas do ESP32.
  
  ArduinoIoTCloud: Para conexão com a dashboard.
  
  ESP32Servo: Para controle preciso do motor no ESP32.
  
  InfluxDB Client for Arduino: Para envio de logs ao banco de dados.

Desenvolvido por Lucas Mineiro, Beatriz Rosa, Cassio Sobreira, Andrei Rehem.
