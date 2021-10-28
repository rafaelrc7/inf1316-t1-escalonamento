# INF1316-T1-Lab

## Alunos
> Rafael Ribeiro de Carvalho (2011104)

> Pedro Piquet Fernandes de Sousa (2011040)

## Execução do Round Robin

exec.txt abaixo:
```
Run ./P1
Run ./P2
Run ./P3
```

Os processos são P1 = Processo 0, P2 = Processo 1, P3 = Processo 2

```
curr_proc: -1   ready_queue: [ 0 ]          Processo 0 criado e adicionado ao fim da fila.
curr_proc: 0    ready_queue: [ ]            Processo 0 entra em execução.
curr_proc: 0    ready_queue: [ 1 ]          Processo 1 criado e adicionado ao fim da fila.
curr_proc: -1   ready_queue: [ 1 0 ]        Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 1    ready_queue: [ 0 ]          Processo 1 entra em execução.
curr_proc: 1    ready_queue: [ 0 2 ]        Processo 2 criado e adicionado ao fim da fila.
curr_proc: -1   ready_queue: [ 0 2 1 ]      Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0    ready_queue: [ 2 1 ]        Processo 0 entra em execução.
curr_proc: -1   ready_queue: [ 2 1 0 ]      Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 2    ready_queue: [ 1 0 ]        Processo 2 entra em execução.
curr_proc: -1   ready_queue: [ 1 0 2 ]      Processo 2 sofre preempção e vai para o fim da fila.
curr_proc: 1    ready_queue: [ 0 2 ]        Processo 1 entra em execução.
curr_proc: -1   ready_queue: [ 0 2 1 ]      Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0    ready_queue: [ 2 1 ]        Processo 0 entra em execução.
curr_proc: -1   ready_queue: [ 2 1 0 ]      Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 2    ready_queue: [ 1 0 ]        Processo 2 entra em execução.
curr_proc: 2    ready_queue: [ 1 0 ]        Processo 2 é interrompido por I/O e é bloqueado.
curr_proc: 1    ready_queue: [ 0 ]          Processo 1 entra em execução.
curr_proc: -1   ready_queue: [ 0 1 ]        Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0    ready_queue: [ 1 ]          Processo 0 entra em execução.
curr_proc: -1   ready_queue: [ 1 0 ]        Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 1    ready_queue: [ 0 ]          Processo 1 entra em execução.
curr_proc: 1    ready_queue: [ 0 2 ]        Processo 2 finaliza I/O, é desbloqueado e vai para o
fim da fila.
curr_proc: -1   ready_queue: [ 0 2 1 ]      Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0    ready_queue: [ 2 1 ]        Processo 0 entra em execução.
curr_proc: -1   ready_queue: [ 2 1 0 ]      Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 2    ready_queue: [ 1 0 ]        Processo 2 entra em execução.
curr_proc: -1   ready_queue: [ 1 0 2 ]      Processo 2 sofre preempção e vai para o fim da fila.
curr_proc: 1    ready_queue: [ 0 2 ]        Processo 1 entra em execução.
curr_proc: -1   ready_queue: [ 0 2 1 ]      Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0    ready_queue: [ 2 1 ]        Processo 0 entra em execução.
curr_proc: -1   ready_queue: [ 2 1 0 ]      Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 2    ready_queue: [ 1 0 ]        Processo 2 entra em execução.
curr_proc: 2    ready_queue: [ 1 0 ]        Processo 2 é interrompido por I/O e é bloqueado.
curr_proc: 1    ready_queue: [ 0 ]          Processo 1 entra em execução.
curr_proc: -1   ready_queue: [ 0 1 ]        Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0    ready_queue: [ 1 ]          Processo 0 entra em execução.
curr_proc: -1   ready_queue: [ 1 0 ]        Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 1    ready_queue: [ 0 ]          Processo 1 entra em execução.
curr_proc: 1    ready_queue: [ 0 2 ]        Processo 2 finaliza I/O, é desbloqueado e vai para o
fim da fila.
curr_proc: -1   ready_queue: [ 0 2 1 ]      Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0    ready_queue: [ 2 1 ]        Processo 0 entra em execução.
curr_proc: -1   ready_queue: [ 2 1 0 ]      Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 2    ready_queue: [ 1 0 ]        Processo 2 entra em execução.
curr_proc: -1   ready_queue: [ 1 0 2 ]      Processo 2 sofre preempção e vai para o fim da fila.
```

## Execução do Round Robin

exec.txt abaixo:
```
Run ./P4 I=5 D=10
Run ./P5 I=20 D=30
Run ./P6 I=40 D=20
```

Os processos são P4 = Processo 0, P5 = Processo 1, P6 = Processo 2 (entra em conflito e não executa)


```
Processo 0 criado e adicionado à agenda.
TS: 0   Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: x | 21: x | 22: x | 23: x | 24: x | 25: x | 26:
x | 27: x | 28: x | 29: x | 30: x | 31: x | 32: x | 33: x | 34: x | 35: x | 36:
x | 37: x | 38: x | 39: x | 40: x | 41: x | 42: x | 43: x | 44: x | 45: x | 46:
x | 47: x | 48: x | 49: x | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 criado e adicionado à agenda.
TS: 0   Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

[ERRO] Novo Processo conflita com outro processo escalonado. Ignorando novo processo.
Processo 0 entra em execução.
TS: 5   Current Proc: 0 schedule: | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 0 sofre preempção.
TS: 15  Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 entra em execução.
TS: 20  Current Proc: 1 schedule: | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 sofre preempção.
TS: 50  Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 0 entra em execução.
TS: 5   Current Proc: 0 schedule: | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 0 sofre preempção.
TS: 15  Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 entra em execução.
TS: 20  Current Proc: 1 schedule: | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 sofre preempção.
TS: 50  Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 0 entra em execução.
TS: 5   Current Proc: 0 schedule: | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 0 sofre preempção.
TS: 15  Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 entra em execução.
TS: 20  Current Proc: 1 schedule: | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 sofre preempção.
TS: 50  Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 0 entra em execução.
TS: 5   Current Proc: 0 schedule: | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 0 sofre preempção.
TS: 15  Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 entra em execução.
TS: 20  Current Proc: 1 schedule: | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 sofre preempção.
TS: 50  Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 0 entra em execução.
TS: 5   Current Proc: 0 schedule: | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 0 sofre preempção.
TS: 15  Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 entra em execução.
TS: 20  Current Proc: 1 schedule: | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |

Processo 1 sofre preempção.
TS: 50  Current Proc: x schedule : | 0: x | 1: x | 2: x | 3: x | 4: x | 5: 0 |
6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 0 | 11: 0 | 12: 0 | 13: 0 | 14: 0 | 15: x | 16:
x | 17: x | 18: x | 19: x | 20: 1 | 21: 1 | 22: 1 | 23: 1 | 24: 1 | 25: 1 | 26:
1 | 27: 1 | 28: 1 | 29: 1 | 30: 1 | 31: 1 | 32: 1 | 33: 1 | 34: 1 | 35: 1 | 36:
1 | 37: 1 | 38: 1 | 39: 1 | 40: 1 | 41: 1 | 42: 1 | 43: 1 | 44: 1 | 45: 1 | 46:
1 | 47: 1 | 48: 1 | 49: 1 | 50: x | 51: x | 52: x | 53: x | 54: x | 55: x | 56:
x | 57: x | 58: x | 59: x |
```
