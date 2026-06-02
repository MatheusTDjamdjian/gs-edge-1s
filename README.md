# 🛰️ Estação Edge de Monitoramento Climático e de Queimadas

**Global Solution 2026 – Edge Computing & Computer Systems (FIAP – 1º ano)**
**Tema: _Space Connect_ – tecnologia espacial a serviço dos desafios da Terra**

---

## 👥 Integrantes do grupo

| Nome completo | RM |
|---|---|
| Matheus Tasso Djamdjian | 57076 |
| Daniel Silva Boccia | 569617 |
| Matheus Augusto da Silva | 572976 |
| Kaik Sales de Amorim | 571558 |

---

## 📌 Descrição do projeto

Estação terrestre autônoma (*ground station*) que atua como **complemento da constelação de satélites** de observação da Terra para o monitoramento de **clima e queimadas**.

Satélites cobrem grandes áreas, mas têm **alta latência** e **baixa frequência de revisita** — não enxergam, em tempo real e no nível do solo, o **início** de uma queimada ou de uma anomalia térmica. Além disso, no campo (florestas, áreas rurais, reservas) **nem sempre há internet/nuvem disponível**.

A solução resolve isso aplicando **Edge Computing**: o dispositivo coleta, filtra, funde e **decide localmente** (na borda), acionando alarmes imediatamente, e só envia ao satélite a informação **resumida e relevante**, economizando banda e energia do enlace.

## 🎯 Objetivo da solução

- Detectar **precocemente** risco de queimada/anomalia climática a partir de **fusão de sensores** (calor + fumaça + clarão).
- Tomar decisões em **tempo real e baixa latência**, sem depender de conexão com a nuvem.
- Reduzir o tráfego para o satélite usando **transmissão orientada a evento** (telemetria resumida em vez de dados brutos contínuos).
- Alinhar-se aos ODS da ONU: **13** (Ação Climática), **11** (Cidades Sustentáveis), **9** (Inovação/Infraestrutura) e **15** (Vida Terrestre).

## 🧩 Componentes utilizados

| Qtd | Componente | Função |
|----|-------------|--------|
| 1 | Arduino Uno R3 | Unidade de processamento na borda (*edge node*) |
| 1 | Sensor de temperatura **TMP36** | Mede calor (°C) |
| 1 | **Fotoresistor (LDR)** + resistor 10 kΩ | Luminosidade / clarão de chama (dia–noite) |
| 1 | **Sensor de gás (MQ)** + resistor 10 kΩ | Fumaça / gases de combustão |
| 1 | **LCD 16x2** (paralelo, HD44780) | Exibe leituras e estado de risco |
| 1 | Potenciômetro 10 kΩ | Ajuste de contraste do LCD |
| 3 | LEDs (verde, amarelo, vermelho) + 3× 220 Ω | Sinalização visual do estado |
| 1 | Buzzer piezo | Alarme sonoro |
| 1 | Protoboard + jumpers | Montagem |

> 💡 **Nota de simulação:** o sensor de gás MQ do Tinkercad possui um *slider* de concentração, ótimo para a demonstração. Caso prefira, ele pode ser substituído por um **potenciômetro em A2** — o código lê o pino como um valor analógico 0–1023, **sem necessidade de alterar nada**.

## ⚙️ Explicação do funcionamento

O firmware aplica, em ciclo, os princípios de **Edge Computing**:

1. **Coleta multi-sensor na borda** — 2 leituras/segundo de temperatura, fumaça e luz.
2. **Pré-processamento local** — **média móvel** (janela de 8 amostras) para filtrar ruído.
3. **Fusão de sensores → Índice de Risco (0–100)** calculado *localmente*:
   `risco = 0,50·fumaça + 0,40·calor + 0,10·clarão`
   *(o CRÍTICO exige a fusão de fumaça **e** calor, reduzindo falso-positivo)*
4. **Decisão de baixa latência** — uma **máquina de estados com histerese** classifica o ambiente e aciona LED + buzzer + LCD **na hora**, sem nuvem:

   | Estado | Faixa de risco | LED | Buzzer |
   |--------|----------------|-----|--------|
   | 🟢 NORMAL | sobe < 40 | Verde | Silêncio |
   | 🟡 ALERTA | ≥ 40 (volta < 30) | Amarelo | Bipe lento |
   | 🔴 CRÍTICO | ≥ 70 (volta < 60) | Vermelho | Bipe rápido |

   *A histerese (limiares diferentes para subir e descer) evita que o alarme fique “piscando” quando o risco oscila perto do limite.*
5. **Uplink orientado a evento** — o “envio ao satélite” (Monitor Serial) só ocorre **quando o estado muda** ou a cada **15 s** (heartbeat), em um **pacote JSON compacto** — economizando banda e energia do enlace de satélite.

Exemplo de telemetria enviada:
```
[UPLINK -> SATELITE] (evento: mudanca de estado)
  {"estacao":"GS-EDGE-01","t_ms":18230,"temp_C":58.4,"fumaca_pct":72,"luz_pct":40,"risco":81,"estado":"CRITICO"}
  >> ALERTA CRITICO retransmitido a rede de satelites / Defesa Civil!
```

## 🔌 Estrutura do circuito

**Pinagem do Arduino Uno:**

| Pino | Conexão |
|------|---------|
| A0 | TMP36 (Vout) |
| A1 | LDR (divisor com 10 kΩ p/ GND) |
| A2 | Sensor de gás (saída analógica) |
| D2 | LCD D7 |
| D3 | LCD D6 |
| D4 | LCD D5 |
| D5 | LCD D4 |
| D11 | LCD E (Enable) |
| D12 | LCD RS |
| D6 / D7 / D8 | LED Verde / Amarelo / Vermelho (via 220 Ω) |
| D9 | Buzzer piezo |

Diagrama de ligações detalhado e tabela completa do LCD: **[docs/circuito.md](docs/circuito.md)**
Passo a passo para montar no simulador: **[TINKERCAD.md](TINKERCAD.md)**
Guia do zero (para quem nunca usou Arduino/Tinkercad): **[GUIA_INICIANTE.md](GUIA_INICIANTE.md)**

## ▶️ Instruções de execução

### Opção A — Tinkercad (recomendado para a entrega)
1. Monte o circuito conforme **[TINKERCAD.md](TINKERCAD.md)**.
2. Em **Code → Text**, cole o conteúdo de [`gs_edge_estacao/gs_edge_estacao.ino`](gs_edge_estacao/gs_edge_estacao.ino).
3. Clique em **Start Simulation**.
4. Abra o **Serial Monitor** (ícone no canto inferior do editor de código) para ver o *uplink*.
5. **Demonstração:**
   - **ALERTA (🟡):** clique no sensor de gás e suba o *slider* de fumaça para ~80–100%.
   - **CRÍTICO (🔴):** mantenha a fumaça alta **e** clique no TMP36 elevando a temperatura para ~65 °C (fusão dos dois sinais).
   - Reduza os valores e observe a **histerese** (o estado só desce ao cruzar o limiar inferior).
   - Acompanhe o LCD, os LEDs, o buzzer e os pacotes `[UPLINK -> SATELITE]` no Serial.

### Opção B — Arduino IDE (placa física, opcional)
1. Abra `gs_edge_estacao/gs_edge_estacao.ino` na Arduino IDE.
2. Selecione **Placa: Arduino Uno** e a porta correta.
3. Clique em **Upload**. Abra o **Monitor Serial** em **9600 baud**.

> A biblioteca `LiquidCrystal` já vem incluída na Arduino IDE e no Tinkercad — não é necessário instalar nada.

## 🔗 Links da entrega

- **Simulação Tinkercad:** _<cole aqui o link público do seu projeto após montar>_
- **Repositório GitHub:** _<este repositório>_
- **Vídeo da solução:** _<mesmo vídeo da disciplina de Storytelling>_

## 📄 Estrutura do repositório

```
gs-edge-1s/
├── gs_edge_estacao/
│   └── gs_edge_estacao.ino   # Código-fonte Arduino/C++
├── docs/
│   └── circuito.md           # Diagrama e tabela de ligações
├── TINKERCAD.md              # Passo a passo de montagem no simulador
└── README.md
```
