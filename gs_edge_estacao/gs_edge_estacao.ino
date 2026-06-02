/*
  ============================================================================
  GLOBAL SOLUTION 2026  -  EDGE COMPUTING & COMPUTER SYSTEMS  (FIAP - 1o ano)
  ----------------------------------------------------------------------------
  Projeto: ESTACAO EDGE DE MONITORAMENTO CLIMATICO E DE QUEIMADAS

  ODS atendidos: 13 (acao climatica), 11 (cidades sustentaveis),
                  9 (inovacao/infraestrutura), 15 (vida terrestre).

  Integrantes do grupo:
   - Matheus Tasso Djamdjian   RM 57076
   - Daniel Silva Boccia       RM 569617
   - Matheus Augusto da Silva  RM 572976
   - Kaik Sales de Amorim      RM 571558
  ============================================================================
*/

#include <LiquidCrystal.h>
#include <stdio.h>

// ----------------------------- MAPA DE PINOS --------------------------------
const uint8_t PIN_TMP36 = A0;   // Sensor de temperatura TMP36 (calor)
const uint8_t PIN_LDR   = A1;   // Sensor de luminosidade LDR (clarao/dia-noite)
const uint8_t PIN_GAS   = A2;   // Sensor de gas/fumaca MQ (fumaca de queimada)

const uint8_t PIN_LED_VERDE    = 6;   // Estado NORMAL
const uint8_t PIN_LED_AMARELO  = 7;   // Estado ALERTA
const uint8_t PIN_LED_VERMELHO = 8;   // Estado CRITICO
const uint8_t PIN_BUZZER       = 9;   // Alarme sonoro

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const unsigned long INTERVALO_AMOSTRAGEM_MS = 500;    // 2 leituras por segundo
const unsigned long INTERVALO_HEARTBEAT_MS  = 15000;  // uplink resumo a cada 15s
const uint8_t N_AMOSTRAS = 8;                          // janela da media movel

const int RISCO_SOBE_ALERTA   = 40;
const int RISCO_DESCE_ALERTA  = 30;
const int RISCO_SOBE_CRITICO  = 70;
const int RISCO_DESCE_CRITICO = 60;

const float PESO_TEMP   = 0.40;
const float PESO_FUMACA = 0.50;
const float PESO_CHAMA  = 0.10;

// Faixa de temperatura usada para pontuar risco (graus C)
const float TEMP_MIN_RISCO = 30.0;
const float TEMP_MAX_RISCO = 70.0;

// ------------------------------- ESTADOS ------------------------------------
enum Estado { NORMAL, ALERTA, CRITICO };
Estado estadoAtual = NORMAL;
Estado estadoAnterior = NORMAL;

int bufTemp[N_AMOSTRAS];
int bufGas[N_AMOSTRAS];
int bufLum[N_AMOSTRAS];
uint8_t idxBuf = 0;
bool bufCheio = false;

unsigned long tUltimaAmostra = 0;
unsigned long tUltimoHeartbeat = 0;
unsigned long tBuzzer = 0;
bool buzzerLigado = false;

// ------------------------------ PROTOTIPOS ----------------------------------
int   mediaMovel(int *buf);
float lerTemperaturaC(int raw);
int   clamp0a100(float v);
const char* nomeEstado();
void  atualizarEstado(int risco);
void  acionarSaidas();
void  atualizarLCD(float tempC, int fumacaPct, int lumPct, int risco);
void  imprimirLinha(uint8_t linhaIdx, const char *txt);
void  gerenciarBuzzer(unsigned long agora);
void  enviarTelemetriaSatelite(float tempC, int fumacaPct, int lumPct, int risco, bool mudou);

// ================================ SETUP =====================================
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  pinMode(PIN_LED_VERDE,    OUTPUT);
  pinMode(PIN_LED_AMARELO,  OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);
  pinMode(PIN_BUZZER,       OUTPUT);

  lcd.setCursor(0, 0); lcd.print(" ESTACAO  EDGE  ");
  lcd.setCursor(0, 1); lcd.print(" Space  Connect ");

  Serial.println(F("=== Estacao Edge de Monitoramento Climatico - ONLINE ==="));
  Serial.println(F("Edge: coleta -> filtra -> funde -> decide local; uplink por evento."));

  lcd.clear();
}

void loop() {
  unsigned long agora = millis();

  // ---------- 1) AMOSTRAGEM PERIODICA ----------
  if (agora - tUltimaAmostra >= INTERVALO_AMOSTRAGEM_MS) {
    tUltimaAmostra = agora;

    bufTemp[idxBuf] = analogRead(PIN_TMP36);
    bufLum[idxBuf]  = analogRead(PIN_LDR);
    bufGas[idxBuf]  = analogRead(PIN_GAS);
    idxBuf++;
    if (idxBuf >= N_AMOSTRAS) { idxBuf = 0; bufCheio = true; }

    int rawTemp = mediaMovel(bufTemp);
    int rawLum  = mediaMovel(bufLum);
    int rawGas  = mediaMovel(bufGas);

    float tempC     = lerTemperaturaC(rawTemp);
    int   lumPct    = map(rawLum, 0, 1023, 0, 100);
    int   fumacaPct = map(rawGas, 0, 1023, 0, 100);

    float scoreTemp = (tempC - TEMP_MIN_RISCO) * 100.0 / (TEMP_MAX_RISCO - TEMP_MIN_RISCO);
    scoreTemp = clamp0a100(scoreTemp);
    float scoreFumaca = fumacaPct;
    float scoreChama  = (lumPct > 80) ? 100.0 : 0.0;   // clarao intenso (possivel chama)

    int risco = clamp0a100(PESO_TEMP   * scoreTemp +
                           PESO_FUMACA * scoreFumaca +
                           PESO_CHAMA  * scoreChama);

    atualizarEstado(risco);

    acionarSaidas();
    atualizarLCD(tempC, fumacaPct, lumPct, risco);

    bool mudouEstado = (estadoAtual != estadoAnterior);
    bool heartbeat   = (agora - tUltimoHeartbeat >= INTERVALO_HEARTBEAT_MS);
    if (mudouEstado || heartbeat) {
      enviarTelemetriaSatelite(tempC, fumacaPct, lumPct, risco, mudouEstado);
      tUltimoHeartbeat = agora;
      estadoAnterior = estadoAtual;
    }
  }

  gerenciarBuzzer(agora);
}

// ============================ FUNCOES AUXILIARES ============================

int mediaMovel(int *buf) {
  uint8_t n = bufCheio ? N_AMOSTRAS : idxBuf;
  if (n == 0) return 0;
  long soma = 0;
  for (uint8_t i = 0; i < n; i++) soma += buf[i];
  return (int)(soma / n);
}

// Converte leitura analogica do TMP36 em graus Celsius
float lerTemperaturaC(int raw) {
  float tensao = raw * (5.0 / 1023.0);
  return (tensao - 0.5) * 100.0;
}

int clamp0a100(float v) {
  if (v < 0)   return 0;
  if (v > 100) return 100;
  return (int)v;
}

const char* nomeEstado() {
  switch (estadoAtual) {
    case NORMAL:  return "NORMAL";
    case ALERTA:  return "ALERTA";
    case CRITICO: return "CRITICO";
  }
  return "?";
}

void atualizarEstado(int risco) {
  switch (estadoAtual) {
    case NORMAL:
      if      (risco >= RISCO_SOBE_CRITICO) estadoAtual = CRITICO;
      else if (risco >= RISCO_SOBE_ALERTA)  estadoAtual = ALERTA;
      break;
    case ALERTA:
      if      (risco >= RISCO_SOBE_CRITICO) estadoAtual = CRITICO;
      else if (risco <  RISCO_DESCE_ALERTA) estadoAtual = NORMAL;
      break;
    case CRITICO:
      if (risco < RISCO_DESCE_CRITICO) estadoAtual = ALERTA;
      break;
  }
}

// Acende apenas o LED do estado atual
void acionarSaidas() {
  digitalWrite(PIN_LED_VERDE,    estadoAtual == NORMAL);
  digitalWrite(PIN_LED_AMARELO,  estadoAtual == ALERTA);
  digitalWrite(PIN_LED_VERMELHO, estadoAtual == CRITICO);
}

// Escreve uma linha de 16 colunas, preenchendo com espacos
void imprimirLinha(uint8_t linhaIdx, const char *txt) {
  char buf[17];
  uint8_t i = 0;
  for (; i < 16 && txt[i] != '\0'; i++) buf[i] = txt[i];
  for (; i < 16; i++) buf[i] = ' ';
  buf[16] = '\0';
  lcd.setCursor(0, linhaIdx);
  lcd.print(buf);
}

void atualizarLCD(float tempC, int fumacaPct, int lumPct, int risco) {
  char l0[24];
  char l1[24];
  snprintf(l0, sizeof(l0), "T%dC F%d%% L%d%%", (int)tempC, fumacaPct, lumPct);
  snprintf(l1, sizeof(l1), "%s R:%d", nomeEstado(), risco);
  imprimirLinha(0, l0);
  imprimirLinha(1, l1);
}

void gerenciarBuzzer(unsigned long agora) {
  unsigned long periodo;
  if      (estadoAtual == CRITICO) periodo = 200;   // bipe rapido
  else if (estadoAtual == ALERTA)  periodo = 800;   // bipe lento
  else { noTone(PIN_BUZZER); buzzerLigado = false; return; }  // NORMAL = silencio

  if (agora - tBuzzer >= periodo) {
    tBuzzer = agora;
    buzzerLigado = !buzzerLigado;
    if (buzzerLigado) tone(PIN_BUZZER, estadoAtual == CRITICO ? 2000 : 1000);
    else              noTone(PIN_BUZZER);
  }
}

// So é chamado por EVENTO (mudanca de estado) ou periódico.
void enviarTelemetriaSatelite(float tempC, int fumacaPct, int lumPct, int risco, bool mudou) {
  Serial.println(F("------------------------------------------------"));
  Serial.print(F("[UPLINK -> SATELITE] "));
  Serial.println(mudou ? F("(evento: mudanca de estado)") : F("(heartbeat periodico)"));
  Serial.print(F("  {\"estacao\":\"GS-EDGE-01\",\"t_ms\":"));
  Serial.print(millis());
  Serial.print(F(",\"temp_C\":"));     Serial.print(tempC, 1);
  Serial.print(F(",\"fumaca_pct\":")); Serial.print(fumacaPct);
  Serial.print(F(",\"luz_pct\":"));    Serial.print(lumPct);
  Serial.print(F(",\"risco\":"));      Serial.print(risco);
  Serial.print(F(",\"estado\":\""));   Serial.print(nomeEstado());
  Serial.println(F("\"}"));
  if (estadoAtual == CRITICO)
    Serial.println(F("  >> ALERTA CRITICO retransmitido a rede de satelites / Defesa Civil!"));
}
