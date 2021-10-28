# INF1316-T1-Escalonamento

## Alunos
> Rafael Ribeiro de Carvalho (2011104)

> Pedro Piquet Fernandes de Sousa (2011040)

## Apresentação
Neste arquivo vai ser documentado a arquitetura dos programas feitos para essa tarefa, além um exemplo de execução prática.

Ambos os escalonadores e interpretadores possuem uma estrutura similar, com características e funcionamento geral em comum.

## IPC
A comunicação entre os processos interpretador e escalonador é feito usando de memória compartilhada, nomeada por uma string constante, **SHM_NAME**, e um semáforo nomeado, também por uma constante, **SEM_NAME**.

As constantes descritas acima são definidas com valores iguais nos pares de programa que funcionam juntos.

Em ambos os pares de programas, o primeiro byte da memória compartilhada é reservado para uma flag que informa se existe um processo esperando no resto da memória para ser iniciado pelo escalonador. O interpretador espera a flag ser `0` para guardar um novo processo na memória e então definir a flag para `1`. O escalonador lê a memória ao checar que a flag é `1` e então reinicia a flag para `0`.

O restante da memória compartilhada é usada para guardar informações do processo, que variam de acordo com o algorítimo de escalonamento.

A mudança da flag da memória compartilhada e, consequentemente, a leitura e escrita de informações de um novo processo, são protegidas pelo semáforo **SEM_NAME**.

O escalonador é responsável pela criação e liberação da memória compartilhada para permitir que um processo escalonador consiga processar os dados vindos de inúmero processos interpretadores diferentes, mesmo que um seja inciado muito depois do escalonador.

Ambos o interpretador e escalonador podem criar o semáforo, porém o escalonador é responsável por liberá-lo pela mesma justificativa acima.

O escalonador usa de um pipe na função de criação de processos para verificar se, após um fork, o exec do processo teve sucesso. O processo filho fecha o pipe caso o exec não tenha erros e, caso o contrário, escreve no pipe o código de erro. O processo pai checa se conseguiu ler algo do pipe, se sim, houve um erro, se não, o processo foi inicalizado com sucesso.

## Interpretador
Ambos interpretadores funcionam de forma bem simples, lendo o arquivo *exec.txt*. O arquivo é lido linha por linha, que são "quebradas" usando o fscanf.

Os interpretadores não criam a memória compartilhada e caso esta já não tenha sido criada pelo escalonador, os processos esperam.

Ao ler uma linha o interpretador organiza as informações no formato correto e a escreve na memória compartilhada, então mudando a flag para 1.

Ao chegar no fim do arquivo o processo encerra.

## Escalonador
Ambos os escalonadores têm estruturas gerais similares, porém diferem no algoritimo e consequentemente estruturas de dados.

### Preparação
Os escalonadores funcionam em um loop a principio infinito, permanecendo em execução esperando por e tratando de processos até o usuário forçar a sua terminação com um **SIGINT** ou **SIGTERM**, enviado por um Ctrl+C por exemplo. Portanto, para finalizar o processo corretamente, o escalonador começa definindo tratamentos para ambos os sinais, que ao serem disparados mudam o estado do loop para finalizar.

Os escalonadores são responsáveis pela criação da memória compartilhada e pela liberação da mesma e do semaforo, por isso são definidas duas funções `void cleanup_sem(void)` e `void cleanup_shm(void)` que têm como objetivo finalizar e liberar, respetivamente, o semáforo e memória compartilhada. Ambas as funções são marcadas, usando da função da biblioteca C `atexit()`, para serem executadas na finalização do programa.

### Estruturas de dados importantes
No geral os escalonadores possuem structs para representar processos e também informações quanto ao estado de I/O dos mesmos.

No loop principal existe uma lista ordenada (para dar prioridade a processos mais novos em caso de empate) para guardar processos bloqueados em I/O, uma variável que mentém as informações do atual processo em execução e estruturas de tempo.

### Execução
O loop principal de ambos os escalonadores pode ser dividido em três fazes em comum:
1. Checar por processos novos e executa-los.
2. Checar se processos bloqueados finalizaram o I/O.
3. Checar se precisa retomar algum processo caso nenhum esteja em execução.
4. Checar se o processo atual em execução entrou em I/O ou finalizou.
5. Checar se o processo atual precisa sofrer preempção.

#### Checar por processos novos e executa-los
O processo de checar por processos novos funciona lendo suas informações da memória compartilhada e interpretando essas informações. A criação de processos é a única funcionalidade do loop principal tratada por uma função separada, `Process *create_process(...)`, seus parâmetros dependem do algorítimo do escalonador. A função possui um contador estático `local_pid` que serve como um pid local do escalonador (não confundir com o pid do sistema, que também é usado para controlar os processos) e tem como função principal o tratamento de prioridade para processos mais novos. A função aloca espaço para as informações do processo, executa o `fork()` e `exec()` e, caso o processo tenha sido iniciado com sucesso, o adiciona para a fila. A função usa de um `pipe()` para checar se o `exec()` teve sucesso. Caso a inicialização do processo falhe, a função retorna `NULL` e não modifica a lista de processos do escalonador. Após o `exec()` o processo recebe um `SIGSTOP` para ser pausado e então depois ser tratado pelo escalonador. Em sucesso a função retorna um ponteiro para o processo alocado.

#### Checar se processos bloqueados finalizaram o I/O
Ambos escalonadores usam a mesma estrutura de dados `SList` para guardar processos bloqueado, a lista é ordenada de acordo com uma função que é passada para a lista durante sua criação. A lista é ordenada pelo `local_pid` do processo, para dar prioridade a processos mais novos. A lista também tem um iterador que é então usado para navegar pela lista e checar se um processo finalizou seu I/O, sendo então removido da lista de I/O e podendo ser executado novamente pelo escalonador.


#### Checar se precisa retomar algum processo caso nenhum esteja em execução.
O escalonador usa a variável `curr_proc` para guardar uma referência ao processo em execução no momento. Caso esta variável seja `NULL` não existe processo em execução, e portanto o escalonador checa se existe algum processo para ser executado, e então, se sim, o faz.

#### Checar se o processo atual em execução entrou em I/O ou finalizou.
Os escalonadores usam a função `waitpid()` para checar se o processo em execução, `curr_proc` entrou em I/O ou finalizou. A função recebe duas flags, `WNOHANG` que faz com que a função não bloqueie o programa, já que é esperado que o processo não necessariamente finalize antes do tempo de preempção ou que necessariamente entre em I/O, e a flag `WUNTRACED` que faz com que a função também avise caso o processo tenha recebido o sinal `SIGSTOP`, que é usado para sinalizar I/O.

Caso a `waitpid()` detecte algo, o status do programa é guardado na variável `stat_loc` que é analisada pelos macros `WIFEXITED` e `WIFSTOPPED` que, respectivamente, checam se o processo finalizou ou entrou em I/O. As situações então são tratadas de acordo com o processo sendo removido caso finalizado ou bloqueado caso entre em I/O.

#### Checar se o processo atual precisa sofrer preempção.
Dependendo do algorítimo o escalonador checa se o processo atual precisa sofrer preempção e, caso sim, pausa-o com um `SIGSTOP` e `curr_proc` passa a ser `NULL` preparando-se pra tentar executar outro processo.

## Peculiariedades

### Round Robin

#### Interpretador
O interpretador do algorítimo RR passa como dados do processo, por memória compartilhada, apenas o nome do programa a ser executado.

#### Escalonador
A estrutura de dados usada para organizar a execução dos processos é uma Fila, definida no módulo `queue`. Caso um processo esteja bloqueado (em I/O) ele é removido dessa fila e é adicionado no final ao poder ser executado novamente.

### Real Time

#### Interpretador
No caso do RT, o interpretador precisa passar muito mais informações sobre o processo (seu tempo de início e duração do burst) e, portanto, o par de escalonador e interpretador RT usam de uma estrutura comum `Msg` para enviar e interpretar dados pela memória compartilhada. Além disso o interpretador define se o tempo de início passado é um tempo "absoluto" ou se é o nome de um outro processo, indicando que é relativo a ele.

#### Escalonador
Por ter um agendamento mais "fixo", a estrutura de dados usada para armazenar a ordem dos processos é um simples array, onde cada indíce representa uma unidade de time slice de execução. Portanto se um processo tem `I=5` ele será posto no índice `5` do array e nos próximos `D` índices, indicando que ele também é executado nesses períodos. Como os processos apenas são removidos do array ao finalizarem, o estado de bloqueio é guardado em uma boleana.

## Módulos extras implementados

### Node
Estrutura simples de um nó encadeado, possui dois campos:

- `void *ptr`: ponteiro para armazernar-se informação.
- `Node *next`: ponteiro para  outro Node, para encadeamento.

### Queue
Implementação simples de uma fila (FIFO) usando a estrutura Node. A API fornecida é de 5 funções:


#### `Queue *queue_create();`
Cria uma estrutura de fila nova, e retorna um ponteiro para ela.

#### `void queue_destroy(Queue *queue);`
Recebe um ponteiro para uma queue e a libera.

#### `void queue_enqueue(Queue *queue, void *item);`
Adiciona um item (precisa ser dinamicamente alocado) ao fim da fila.

#### `void *queue_dequeue(Queue *queue);`
Remove e retorna o primeiro item da fila.

#### `int queue_is_empty(Queue *queue);`
Retorna se uma fila está vazia ou não.

### SList
Estrutura que implementa uma lista encadeada ordenada. Em sua criação é fornecida uma função que define a relação de ordenação entre dois valores e ao inserir um novo item, colca-o em uma posição que mantenha a lista ordenada.

A lista também possui um mecanismo de iteração de forma que possa-se navegar pela lista e checar os valores de cada item.

A API fornece as seguintes 11 funções:

#### `SList *slist_create(int (*fun_ordering)(void *, void *));`
Cria uma estrutura de lista nova e retorna um ponteiro para ela. A lista criada usará a função passada como parametro para sua ordenação.

A função deve receber dois ponteiros, que apontaram para o item a ser inserido e um outro item da lista. A função deverá retornar `-1` se o primeiro for menor que o segundo, `0` se forem iguais e `1` se o primeiro for maior que o outro.

#### `void slist_destroy(SList *slist);`
Recebe um ponteiro para uma lista e a libera.

#### `void slist_insert(SList *slist, void *ptr);`
Insere um item na lista, usando da função `int fun_ordering(void *, void *)` de sua estrutura para manter a lista ordenada.

#### `void *slist_remove(SList *slist);`
Remove e retorna o primeiro item da lista.

#### `void *slist_peek(SList *slist);`
Retorna, mas não remove, o primeiro item da lista.

#### `void *slist_remove_index(SList *slist, unsigned long int index);`
Remove e retorna o item da lista na posição dada por index.

#### `unsigned long int slist_size(SList *slist);`
Retorna o número de elementos armazenados na lista.

#### `unsigned int slist_is_empty(SList *slist);`
Retorna se a lista está vazia ou não.

#### `void *slist_iterator_head(SList *slist);`
Inicia o processo de iteração, começando pelo primeiro item da lista, retornando seu valor.

#### `void *slist_iterator_next(SList *slist);`
Avança o iterador da lista, retornando o próximo item. retorna NULL se a iteração chegou ao fim.

#### `void *slist_iterator_remove(SList *slist);`
Remove o item atualmente apontado pelo iterador da lista e o retorna.

## Tratamento de erros
Não existe classificação de tipos de erro no código em si. Porém os programas tratam erros de duas maneiras, erros "fatais" e não fatais. Falhas em funções do sistema como `malloc()` e `calloc()`, dentre outras, são tratados como erro fatal e o programa é encerrado. Porém alguns erros, que são esperados e trataveis, como, por exemplo, tentar executar um programa que não existe (não encontrado pela `exec()`) apenas gerará uma mensagem de erro e o processo será ignorado.

## Testes Práticos
### Real Time

O programa executado foi o `exec.txt` abaixo
```
Run ./prog_cpu I=0 D=10
Run ./prog_cpu I=10 D=5
Run ./prog_cpu I=30 D=5
Run ./prog_io I=15 D=2
Run ./prog_cpu I=./prog_io D=5
```

Como explicado acima, em *Escalonador*, I = inicio da execução e D = duração.

Os interpretadores enviam novos processos aos escalonadores a cada 1s, portanto os processos foram criados, respectivamente, nos momentos 1s, 2s, 3s, 4s e 5s.

A ordem de execução foi o primeiro programa em 0s, que executado até 10s. Em 10s executa-se o segundo programa, que é executado até 15s. Em 15s executa-se o quarto programa, que é executado até 17s. Da mesma maneira, o quinto programa é executado quando o time slice do quarto acaba (17s), sendo executado de 17s a 22s. De 22s a 30s a CPU fica ociosa, e em 30s o terceiro programa é iniciado, sendo executado até 35s. CPU fica ociosa de 35s a 59s. O período então acaba e volta para a time slice 0s.

Abaixo está cada passo da execução com a "agenda" de processos atualizada nas mudanças. 'x' significa que não tem um processo alocado naquele momento ou q não tem um processo em execução.

```
Processo 0 criado e adicionado à agenda.
TS: 0	Current Proc: 0	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: x |
11: x | 12: x | 13: x | 14: x | 15: x | 16: x | 17: x | 18: x | 19: x | 20: x |
21: x | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: x |
31: x | 32: x | 33: x | 34: x | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 0 entra em execução.
TS: 0	Current Proc: 0	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: x |
11: x | 12: x | 13: x | 14: x | 15: x | 16: x | 17: x | 18: x | 19: x | 20: x |
21: x | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: x |
31: x | 32: x | 33: x | 34: x | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 1 criado e adicionado à agenda.
TS: 0	Current Proc: 0	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: x | 16: x | 17: x | 18: x | 19: x | 20: x |
21: x | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: x |
31: x | 32: x | 33: x | 34: x | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 2 criado e adicionado à agenda.
TS: 1	Current Proc: 0	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: x | 16: x | 17: x | 18: x | 19: x | 20: x |
21: x | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 3 criado e adicionado à agenda.
TS: 2	Current Proc: 0	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: x | 18: x | 19: x | 20: x |
21: x | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 4 criado e adicionado à agenda.
TS: 3	Current Proc: 0	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 0 sofre preempção.
TS: 10	Current Proc: 1	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 1 entra em execução.
TS: 10	Current Proc: 1	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 1 sofre preempção.
TS: 15	Current Proc: 3	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 3 entra em execução.
TS: 15	Current Proc: 3	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 3 é interrompido por I/O e é bloqueado.

Processo 4 entra em execução.
TS: 17	Current Proc: 4	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 3 finaliza I/O e é desbloqueado.

Processo 4 sofre preempção.
TS: 22	Current Proc: x	schedule :
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 2 entra em execução.
TS: 30	Current Proc: 2	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 2 sofre preempção.
TS: 35	Current Proc: x	schedule :
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 0 entra em execução.
TS: 0	Current Proc: 0	schedule:
| 0: 0 | 1: 0 | 2: 0 | 3: 0 | 4: 0 | 5: 0 | 6: 0 | 7: 0 | 8: 0 | 9: 0 | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

*Processo 0 finalizou.
TS: 0	Current Proc: x	schedule :
| 0: x | 1: x | 2: x | 3: x | 4: x | 5: x | 6: x | 7: x | 8: x | 9: x | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 1 entra em execução.
TS: 10	Current Proc: 1	schedule:
| 0: x | 1: x | 2: x | 3: x | 4: x | 5: x | 6: x | 7: x | 8: x | 9: x | 10: 1 |
11: 1 | 12: 1 | 13: 1 | 14: 1 | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

*Processo 1 finalizou.
TS: 11	Current Proc: x	schedule :
| 0: x | 1: x | 2: x | 3: x | 4: x | 5: x | 6: x | 7: x | 8: x | 9: x | 10: x |
11: x | 12: x | 13: x | 14: x | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 3 entra em execução.
TS: 15	Current Proc: 3	schedule:
| 0: x | 1: x | 2: x | 3: x | 4: x | 5: x | 6: x | 7: x | 8: x | 9: x | 10: x |
11: x | 12: x | 13: x | 14: x | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 3 é interrompido por I/O e é bloqueado.

Processo 4 entra em execução.
TS: 17	Current Proc: 4	schedule:
| 0: x | 1: x | 2: x | 3: x | 4: x | 5: x | 6: x | 7: x | 8: x | 9: x | 10: x |
11: x | 12: x | 13: x | 14: x | 15: 3 | 16: 3 | 17: 4 | 18: 4 | 19: 4 | 20: 4 |
21: 4 | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 3 finaliza I/O e é desbloqueado.

*Processo 4 finalizou.
TS: 21	Current Proc: x	schedule :
| 0: x | 1: x | 2: x | 3: x | 4: x | 5: x | 6: x | 7: x | 8: x | 9: x | 10: x |
11: x | 12: x | 13: x | 14: x | 15: 3 | 16: 3 | 17: x | 18: x | 19: x | 20: x |
21: x | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

Processo 2 entra em execução.
TS: 30	Current Proc: 2	schedule:
| 0: x | 1: x | 2: x | 3: x | 4: x | 5: x | 6: x | 7: x | 8: x | 9: x | 10: x |
11: x | 12: x | 13: x | 14: x | 15: 3 | 16: 3 | 17: x | 18: x | 19: x | 20: x |
21: x | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: 2 |
31: 2 | 32: 2 | 33: 2 | 34: 2 | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |

*Processo 2 finalizou.
TS: 31	Current Proc: x	schedule :
| 0: x | 1: x | 2: x | 3: x | 4: x | 5: x | 6: x | 7: x | 8: x | 9: x | 10: x |
11: x | 12: x | 13: x | 14: x | 15: 3 | 16: 3 | 17: x | 18: x | 19: x | 20: x |
21: x | 22: x | 23: x | 24: x | 25: x | 26: x | 27: x | 28: x | 29: x | 30: x |
31: x | 32: x | 33: x | 34: x | 35: x | 36: x | 37: x | 38: x | 39: x | 40: x |
41: x | 42: x | 43: x | 44: x | 45: x | 46: x | 47: x | 48: x | 49: x | 50: x |
51: x | 52: x | 53: x | 54: x | 55: x | 56: x | 57: x | 58: x | 59: x |
```

### Round Robin


O programa executado foi o `exec.txt` abaixo

```
Run ./prog_cpu
Run ./prog_cpu
Run ./prog_cpu
Run ./prog_io
Run ./prog_cpu
```

Como os interpretadores enviam os novos processos a cada 1s, os processos foram criados, respectivamente, no momento 1s, 2s, 3s, 4s, 5s. Os processos são executados um após o outro, numa fila FIFO, porém em caso de I/O o processo ficara fora da fila até ser desbloqueado.

O *quantum* é de 1s e o I/O dura 3s. Os estados da fila de prontos do programa foram:

```
curr_proc: -1	ready_queue: [ 0 ]	Processo 0 criado e adicionado ao fim da fila.
curr_proc: 0	ready_queue: [ ]	Processo 0 entra em execução.
curr_proc: 0	ready_queue: [ 1 ]	Processo 1 criado e adicionado ao fim da fila.
curr_proc: -1	ready_queue: [ 1 0 ]	Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 1	ready_queue: [ 0 ]	Processo 1 entra em execução.
curr_proc: 1	ready_queue: [ 0 2 ]	Processo 2 criado e adicionado ao fim da fila.
curr_proc: -1	ready_queue: [ 0 2 1 ]	Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0	ready_queue: [ 2 1 ]	Processo 0 entra em execução.
curr_proc: 0	ready_queue: [ 2 1 3 ]	Processo 3 criado e adicionado ao fim da fila.
curr_proc: -1	ready_queue: [ 2 1 3 0 ]	Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 2	ready_queue: [ 1 3 0 ]	Processo 2 entra em execução.
curr_proc: 2	ready_queue: [ 1 3 0 4 ]	Processo 4 criado e adicionado ao fim da fila.
curr_proc: -1	ready_queue: [ 1 3 0 4 2 ]	Processo 2 sofre preempção e vai para o fim da fila.
curr_proc: 1	ready_queue: [ 3 0 4 2 ]	Processo 1 entra em execução.
curr_proc: -1	ready_queue: [ 3 0 4 2 1 ]	Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 3	ready_queue: [ 0 4 2 1 ]	Processo 3 entra em execução.
curr_proc: 3	ready_queue: [ 0 4 2 1 ]	Processo 3 é interrompido por I/O e é bloqueado.
curr_proc: 0	ready_queue: [ 4 2 1 ]	Processo 0 entra em execução.
curr_proc: -1	ready_queue: [ 4 2 1 0 ]	Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 4	ready_queue: [ 2 1 0 ]	Processo 4 entra em execução.
curr_proc: -1	ready_queue: [ 2 1 0 4 ]	Processo 4 sofre preempção e vai para o fim da fila.
curr_proc: 2	ready_queue: [ 1 0 4 ]	Processo 2 entra em execução.
curr_proc: 2	ready_queue: [ 1 0 4 3 ]	Processo 3 finaliza I/O, é desbloqueado e vai para o fim
da fila.
curr_proc: -1	ready_queue: [ 1 0 4 3 2 ]	Processo 2 sofre preempção e vai para o fim da fila.
curr_proc: 1	ready_queue: [ 0 4 3 2 ]	Processo 1 entra em execução.
curr_proc: -1	ready_queue: [ 0 4 3 2 1 ]	Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0	ready_queue: [ 4 3 2 1 ]	Processo 0 entra em execução.
curr_proc: -1	ready_queue: [ 4 3 2 1 0 ]	Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 4	ready_queue: [ 3 2 1 0 ]	Processo 4 entra em execução.
curr_proc: -1	ready_queue: [ 3 2 1 0 4 ]	Processo 4 sofre preempção e vai para o fim da fila.
curr_proc: 3	ready_queue: [ 2 1 0 4 ]	Processo 3 entra em execução.
curr_proc: 3	ready_queue: [ 2 1 0 4 ]	Processo 3 é interrompido por I/O e é bloqueado.
curr_proc: 2	ready_queue: [ 1 0 4 ]	Processo 2 entra em execução.
curr_proc: -1	ready_queue: [ 1 0 4 2 ]	Processo 2 sofre preempção e vai para o fim da fila.
curr_proc: 1	ready_queue: [ 0 4 2 ]	Processo 1 entra em execução.
curr_proc: -1	ready_queue: [ 0 4 2 1 ]	Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0	ready_queue: [ 4 2 1 ]	Processo 0 entra em execução.
curr_proc: 0	ready_queue: [ 4 2 1 3 ]	Processo 3 finaliza I/O, é desbloqueado e vai para o fim
da fila.
curr_proc: -1	ready_queue: [ 4 2 1 3 0 ]	Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 4	ready_queue: [ 2 1 3 0 ]	Processo 4 entra em execução.
curr_proc: -1	ready_queue: [ 2 1 3 0 4 ]	Processo 4 sofre preempção e vai para o fim da fila.
curr_proc: 2	ready_queue: [ 1 3 0 4 ]	Processo 2 entra em execução.
curr_proc: -1	ready_queue: [ 1 3 0 4 2 ]	Processo 2 sofre preempção e vai para o fim da fila.
curr_proc: 1	ready_queue: [ 3 0 4 2 ]	Processo 1 entra em execução.
curr_proc: -1	ready_queue: [ 3 0 4 2 1 ]	Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 3	ready_queue: [ 0 4 2 1 ]	Processo 3 entra em execução.
curr_proc: 3	ready_queue: [ 0 4 2 1 ]	Processo 3 é interrompido por I/O e é bloqueado.
curr_proc: 0	ready_queue: [ 4 2 1 ]	Processo 0 entra em execução.
curr_proc: -1	ready_queue: [ 4 2 1 0 ]	Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 4	ready_queue: [ 2 1 0 ]	Processo 4 entra em execução.
curr_proc: -1	ready_queue: [ 2 1 0 4 ]	Processo 4 sofre preempção e vai para o fim da fila.
curr_proc: 2	ready_queue: [ 1 0 4 ]	Processo 2 entra em execução.
curr_proc: 2	ready_queue: [ 1 0 4 3 ]	Processo 3 finaliza I/O, é desbloqueado e vai para o fim
da fila.
curr_proc: -1	ready_queue: [ 1 0 4 3 2 ]	Processo 2 sofre preempção e vai para o fim da fila.
curr_proc: 1	ready_queue: [ 0 4 3 2 ]	Processo 1 entra em execução.
curr_proc: -1	ready_queue: [ 0 4 3 2 1 ]	Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0	ready_queue: [ 4 3 2 1 ]	Processo 0 entra em execução.
curr_proc: -1	ready_queue: [ 4 3 2 1 0 ]	Processo 0 sofre preempção e vai para o fim da fila.
curr_proc: 4	ready_queue: [ 3 2 1 0 ]	Processo 4 entra em execução.
curr_proc: -1	ready_queue: [ 3 2 1 0 4 ]	Processo 4 sofre preempção e vai para o fim da fila.
curr_proc: 3	ready_queue: [ 2 1 0 4 ]	Processo 3 entra em execução.
curr_proc: -1	ready_queue: [ 2 1 0 4 ]	*Processo 3 finalizou.
curr_proc: 2	ready_queue: [ 1 0 4 ]	Processo 2 entra em execução.
curr_proc: -1	ready_queue: [ 1 0 4 2 ]	Processo 2 sofre preempção e vai para o fim da fila.
curr_proc: 1	ready_queue: [ 0 4 2 ]	Processo 1 entra em execução.
curr_proc: -1	ready_queue: [ 0 4 2 1 ]	Processo 1 sofre preempção e vai para o fim da fila.
curr_proc: 0	ready_queue: [ 4 2 1 ]	Processo 0 entra em execução.
curr_proc: -1	ready_queue: [ 4 2 1 ]	*Processo 0 finalizou.
curr_proc: 4	ready_queue: [ 2 1 ]	Processo 4 entra em execução.
curr_proc: -1	ready_queue: [ 2 1 4 ]	Processo 4 sofre preempção e vai para o fim da fila.
curr_proc: 2	ready_queue: [ 1 4 ]	Processo 2 entra em execução.
curr_proc: -1	ready_queue: [ 1 4 ]	*Processo 2 finalizou.
curr_proc: 1	ready_queue: [ 4 ]	Processo 1 entra em execução.
curr_proc: -1	ready_queue: [ 4 ]	*Processo 1 finalizou.
curr_proc: 4	ready_queue: [ ]	Processo 4 entra em execução.
curr_proc: -1	ready_queue: [ 4 ]	Processo 4 sofre preempção e vai para o fim da fila.
curr_proc: 4	ready_queue: [ ]	Processo 4 entra em execução.
curr_proc: -1	ready_queue: [ ]	*Processo 4 finalizou.
```

