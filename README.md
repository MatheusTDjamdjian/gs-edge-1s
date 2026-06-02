# Estação Edge de Monitoramento Climático e de Queimadas

**Global Solution 2026 – Edge Computing & Computer Systems (FIAP – 1º ano)**

---

## Integrantes do grupo

| Nome completo | RM |
| Matheus Tasso Djamdjian | 57076 |
| Daniel Silva Boccia | 569617 |
| Matheus Augusto da Silva | 572976 |
| Kaik Sales de Amorim | 571558 |

---

## Descrição do projeto

Estação terrestre autônoma que atua como complemento da constelação de satélites de observação da Terra para o monitoramento de **clima e queimadas**.

Satélites cobrem grandes áreas, mas têm **alta latência** e **baixa frequência de revisita** — não enxergam, em tempo real e no nível do solo, o início de uma queimada. Além disso, no campo nem sempre há internet/nuvem disponível.

A solução resolve aplicando a matéria: o dispositivo coleta, filtra e decide localmente na borda, acionando alarmes e só envia ao satélite a informação resumida e relevante.

## Objetivo

- Detectar risco de queimada/anomalia climática a partir de fusão de sensores.
- Tomar decisões em tempo real e baixa latência, sem depender de conexão com a nuvem.
- Reduzir o tráfego para o satélite usando transmissão orientada a evento.
- ODS da ONU: **13** (Ação Climática), **11** (Cidades Sustentáveis), **9** (Inovação/Infraestrutura) e **15** (Vida Terrestre).

## Componentes utilizados

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

## Explicação do funcionamento

1. **Coleta multi-sensor na borda** — 2 leituras/segundo de temperatura, fumaça e luz.
2. **Pré-processamento local** — **média móvel** para filtrar ruído.
3. **Fusão de sensores → Índice de Risco (0–100)** calculado *localmente*:
   `risco = 0,50·fumaça + 0,40·calor + 0,10·clarão`
4. **Decisão de baixa latência** — classifica o ambiente e aciona LED + buzzer + LCD **na hora**, sem nuvem:

   | Estado | Faixa de risco | LED | Buzzer |
   |--------|----------------|-----|--------|
   | 🟢 NORMAL | sobe < 40 | Verde | Silêncio |
   | 🟡 ALERTA | ≥ 40 (volta < 30) | Amarelo | Bipe lento |
   | 🔴 CRÍTICO | ≥ 70 (volta < 60) | Vermelho | Bipe rápido |

## Estrutura do circuito

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

## Instruções de execução

### Tinkercad
2. Em **Code → Text**, cole o conteúdo de [`gs_edge_estacao/gs_edge_estacao.ino`](gs_edge_estacao/gs_edge_estacao.ino).
3. Clique em **Start Simulation**.
4. Abra o **Serial Monitor** para ver o *uplink*.

## Estrutura do repositório

```
gs-edge-1s/
├── gs_edge_estacao/
│   └── gs_edge_estacao.ino   # Código-fonte Arduino/C++
├── docs/
│   └── circuito.md           # Diagrama e tabela de ligações
├── TINKERCAD.md              # Passo a passo de montagem no simulador
└── README.md
```
