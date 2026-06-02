# Estrutura do circuito

### Arduino → Componentes
| Pino Arduino | Componente | Observação |
| 5V | Trilho + da protoboard | Alimentação |
| GND| Trilho – da protoboard | Referência |
| A0 | TMP36 — Vout | Temperatura |
| A1 | LDR + resistor 10 kΩ (divisor) | Luminosidade |
| A2 | Sensor de gás (saída) + 10 kΩ | Fumaça |
| D2 | LCD pino 14 (D7) | Dados LCD |
| D3 | LCD pino 13 (D6) | Dados LCD |
| D4 | LCD pino 12 (D5) | Dados LCD |
| D5 | LCD pino 11 (D4) | Dados LCD |
| D11| LCD pino 6 (E) | Enable LCD |
| D12| LCD pino 4 (RS) | Register Select |
| D6 | LED verde (ânodo via 220 Ω) | NORMAL |
| D7 | LED amarelo (ânodo via 220 Ω) | ALERTA |
| D8 | LED vermelho (ânodo via 220 Ω) | CRÍTICO |
| D9 | Buzzer piezo (+) | Alarme |

## Lógica de decisão (resumo)

```
risco = 0,50·fumaça + 0,40·calor + 0,10·clarão        (0–100, calculado na borda)

         risco↑ ≥40                 risco↑ ≥70
NORMAL ───────────────▶ ALERTA ───────────────▶ CRÍTICO
   ◀───────────────         ◀───────────────
         risco↓ <30                 risco↓ <60
        (histerese — evita oscilação no limiar)
```
