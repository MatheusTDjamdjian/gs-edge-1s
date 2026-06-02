# 🔌 Estrutura do circuito

## Diagrama de blocos (fluxo Edge Computing)

```
   SENSORES (borda)            ARDUINO UNO (edge node)              ATUADORES (local)
 ┌──────────────────┐      ┌───────────────────────────┐      ┌──────────────────────┐
 │ TMP36   (calor)  │─A0──▶│ 1. coleta multi-sensor     │─D6──▶│ LED verde   (NORMAL) │
 │ LDR     (clarão) │─A1──▶│ 2. média móvel (filtro)    │─D7──▶│ LED amarelo (ALERTA) │
 │ Gás MQ  (fumaça) │─A2──▶│ 3. fusão → Índice de Risco │─D8──▶│ LED vermelho(CRÍTICO)│
 └──────────────────┘      │ 4. estados c/ histerese    │─D9──▶│ Buzzer (alarme)      │
                           │ 5. uplink por evento ──────┼──────▶ LCD 16x2 (D2-D5,11,12)│
                           └─────────────┬─────────────┘      └──────────────────────┘
                                         │ Serial (uplink resumido)
                                         ▼
                              📡  REDE DE SATÉLITES / NUVEM
                              (só recebe telemetria por evento)
```

## Tabela completa de ligações

### Arduino → Componentes
| Pino Arduino | Componente | Observação |
|---|---|---|
| 5V | Trilho + da protoboard | Alimentação |
| GND | Trilho – da protoboard | Referência |
| A0 | TMP36 — Vout | Temperatura |
| A1 | LDR + resistor 10 kΩ (divisor) | Luminosidade |
| A2 | Sensor de gás (saída) + 10 kΩ | Fumaça |
| D2 | LCD pino 14 (D7) | Dados LCD |
| D3 | LCD pino 13 (D6) | Dados LCD |
| D4 | LCD pino 12 (D5) | Dados LCD |
| D5 | LCD pino 11 (D4) | Dados LCD |
| D11 | LCD pino 6 (E) | Enable LCD |
| D12 | LCD pino 4 (RS) | Register Select |
| D6 | LED verde (ânodo via 220 Ω) | NORMAL |
| D7 | LED amarelo (ânodo via 220 Ω) | ALERTA |
| D8 | LED vermelho (ânodo via 220 Ω) | CRÍTICO |
| D9 | Buzzer piezo (+) | Alarme |

### LCD 16x2 (HD44780) — 16 pinos
| Pino | Sinal | Liga em |
|---|---|---|
| 1 | VSS | GND |
| 2 | VDD | 5V |
| 3 | VO | Wiper do pot. de contraste (10 kΩ) |
| 4 | RS | D12 |
| 5 | RW | GND |
| 6 | E | D11 |
| 7–10 | D0–D3 | (não usados — modo 4 bits) |
| 11 | D4 | D5 |
| 12 | D5 | D4 |
| 13 | D6 | D3 |
| 14 | D7 | D2 |
| 15 | A | 5V (back-light) |
| 16 | K | GND (back-light) |

## Lógica de decisão (resumo)

```
risco = 0,50·fumaça + 0,40·calor + 0,10·clarão        (0–100, calculado na borda)

         risco↑ ≥40                 risco↑ ≥70
NORMAL ───────────────▶ ALERTA ───────────────▶ CRÍTICO
   ◀───────────────         ◀───────────────
         risco↓ <30                 risco↓ <60
        (histerese — evita oscilação no limiar)
```
