#import "resources/report.typ" as report

#show: report.styling.with(
    hasFooter: false
)

#show raw.where(block: true): block.with(
  fill: luma(240),
  inset: 10pt,
  radius: 4pt,
)

#set table(
  fill: (x, y) =>
    if x == 0 or y == 0 {
      luma(240)
    },

  align: (x, y) =>
    if x == 0 or y == 0 {
      center
    } else {
      right
    },
)

= Introdução 

 O objetivo deste relatório é apresentar a implementação de um orquestrador de tarefas. O sistema, desenvolvido em C, é composto por um cliente e um servidor que communicam através de pipes com nome (`FIFOS`). O cliente, que opera através do terminal, envia comandos para o servidor, que os interpreta e executa as tarefas correspondentes, suportado por uma política de escalonamento pré-definida e pela capacidade de poder executar múltiplos processos em simultâneo, informando assim o cliente sobre o identificador da tarefa, cujo resultado é posteriormente guardado em um ficheiro de texto unicamente identificado.

 Este projeto foi desenvolvido com recurso às chamadas de sistema do UNIX, tais como `fork`, `pipe`, `dup2`, `execvp`, `wait`, entre outras, que são cruciais para o desenvolvimento de um sistema de orquestração de tarefas eficiente e responsivo.
 Assim, este relatório apresenta a implementação do sistema, descrevendo a comunicação entre o cliente e o servidor, o modo de execução de tarefas e o escalonamento das mesmas. Adicionalmente, são apresentados os resultados de testes de desempenho realizados ao orquestrador de tarefas, que permitem avaliar a eficiência e o uso de diferentes politicas de escalonamento.

= Comunicação entre Cliente e Servidor

== Servidor

Após a sua inicialização e da verificação dos parâmetros de entrada, o servidor cria um `FIFO` geral para receber os vários pedidos dos clientes. Este `FIFO` serve como um canal de comunicação centralizado através do qual todos os pedidos são encaminhados para o servidor. Assim, após a leitura do pedido enviado pelo cliente, o servidor utiliza a função `command_interpreter`, para interpretar o request. Esta função é responsável por identificar não só o comando enviado do cliente, como também obter o seu `id`. Este `id` é crucial pois vai ser usado posteriormente para identificar o `FIFO` do cliente, com o objetivo de enviar uma resposta em função do pedido feito.

== Cliente

Após o arranque do servidor, o cliente pode começar com os seus pedidos. Inicalmente, o cliente cria um `FIFO` com o nome do seu `PID`, que será utilizado para receber as respostas do servidor. De seguida, o cliente envia um pedido ao servidor através do `FIFO` geral. O envio de toda a informação é assegurado pela função `command_sender`. Esta função encapsula a lógica necessária para formatar e transmitir os comandos de forma eficiente e sem erros, garantindo que o servidor recebe a informação completa.
Este pedido é lido pelo servidor, que o interpretará e executará a tarefa correspondente. Após a análise do pedido, o servidor envia uma resposta ao cliente através do `FIFO` do cliente, que é idenificado unicamente pelo seu `PID`.

= Execução de Tarefas

No sistema de orquestração desenvolvido, a execução de tarefas é a funcionalidade chave que permite ao servidor processar e executar os pedidos feitos pelos clientes, manipulando tanto tarefas simples como as compostas por `pipelines` de comandos. Aliado a todos estes fatores, o escalonamento de tarefas é também uma funcionalidade importante, que permite ao servidor gerir de forma eficiente o processamento de tarefas, garantindo que o sistema não fica sobrecarregado e que as tarefas são executadas de forma eficiente.
Assim, tal como referido anteriormente, o uso das chamadas ao sistema foi a chave para a implementação de todas estas funcionalidades e pelo correto funcionamento do sistema de orquestração de tarefas.

Em seguida, serão apresentadas as principais funcionalidades do sistema, nomeadamente a execução de tarefas simples e compostas, bem como a estratégia utilizada para o escalonamento de tarefas.

== Limite de Tarefas em Paralelo

Na inicialização do servidor, um dos parâmetros de entrada é o número máximo de tarefas que podem ser executadas em paralelo, o que permite uma maior flexibilidade conforme as necessidades e capacidades do servidor. Este limite é crucial para garantir que o servidor não fica sobrecarregado com um número excessivo de tarefas, o que poderia levar a uma degradação do desempenho. É mantida uma lista de tarefas em execução, que é atualizada sempre que uma tarefa termina e um contador de tarefas em execução, que é incrementado sempre que uma nova tarefa é iniciada e decrementado sempre que uma tarefa termina. Assim, o servidor é capaz de gerir de forma eficiente o número de tarefas em execução.
O uso do mecanismo de `fork` é fundamental para alcançar o processamento de multiplas tarefas de forma eficaz, permitindo que o servidor crie um novo processo para cada tarefa a ser executada. Assim, o servidor pode executar várias tarefas em simultâneo, garantindo que o sistema é capaz de processar pedidos de forma eficiente e sem bloquear o servidor/cliente.

Todas as tarefas que não conseguem ser executadas de forma imediata, são colocadas numa fila de espera, que é processada assim que o número de tarefas em execução seja inferior ao limite definido.

= Processos simples

A execução de tarefas simples é feita através da função `execute_simple_process`, que é responsável não só pela execução do comando, como também por redirecionar a saída do mesmo para um ficheiro de texto, que é unicamente identificado. O uso do mecanismo de `dup2` é crucial para redirecionar a saída do comando e dos erros para o ficheiro de texto, garantindo que a informação é guardada de forma correta e sem erros.
Para facilitar a execução do comando,a instrução recebida é cuidadosamente fragmentada em tokens. Esta segmentação é crucial porque o `execvp`, utilizado para executar o comando, requer que o comando e os seus argumentos sejam fornecidos como um array de strings terminado em `NULL`. Esse processo de tokenização assegura que o mesmo seja interpretado e executado corretamente pelo sistema, permitindo uma gestão eficaz dos recursos do servidor e a correta execução das tarefas conforme solicitado pelo cliente.
Comparativamente, a execução de processos simples é consideravelmente mais direta e menos trabalhosa do que a execução de processos compostos, uma vez que não envolve a comunicação entre processos e a gestão de pipes. Assim, esta simplicidade permite uma implementação mais rápida e menos propensa a erros.

== Processos compostos

A execução de tarefas compostas é feita através da função `execute_pipeline_process`, que tal como a função `execute_simple_process`, é responsável pela execução do comando e pelo redirecionamento da saída para um ficheiro de texto. No entanto, a execução de tarefas compostas é mais complexa, uma vez que envolve a execução de vários comandos em sequência, com a saída de um comando a ser redirecionada para a entrada do comando seguinte. Todo este mecanismo foi possível graças ao uso de `pipes`, que permitem a comunicação entre os processos de forma eficiente e sem erros. Na execução destes processos compostos, cada um dos comandos é executado em processos separados, cada qual tratado por uma instância de processo filho criada através da chamada ao sistema `fork`. A comunicação entre esses processos é estabelecida através de `pipes`, que são elementos cruciais para a execução sequencial e interdependente dos comandos. Cada pipe é constituído por um par de `file descriptors` - um para leitura e outro para escrita. Deste modo, o `stdout` de um processo é ligado diretamente ao `stdin` do próximo processo na cadeia através do `file descriptor` correspondente.

Ao usar esta técnica de `pipelining`, a saída de dados de um comando não precisa de ser armazenada em disco antes de ser lida pelo comando seguinte. Isso reduz a latência e o `overhead` associado à leitura e escrita em sistemas de armazenamento, resultando numa execução mais rápida e eficiente das tarefas, otimizando os recursos do sistema e permitindo assim um fluxo de processamento mais fluído entre os comandos envolvidos.
Para garantir a integridade e a correta execução dos processos, é essencial gerir adequadamente os recursos do sistema. Isto inclui fechar apropriadamente os `file descriptors` dos `pipes` que não são mais necessários, evitando assim `leaks` de recursos e possíveis erros de execução. Adicionalmente, cada processo filho verifica se é o último na cadeia de comandos. Caso seja, o seu output é redirecionado para um ficheiro de saída devidamente identificado em função do identificador da tarefa, onde os resultados finais são armazenados.

Finalmente, o processo pai (que neste caso também é um processo filho) aguarda o término de todos os processos filhos, utilizando a `system call` `wait`. Esta espera assegura que todos os comandos na pipeline foram executados até a conclusão e que todos os recursos associados foram corretamente libertados.

Em suma, o principal interveniente na execução de tarefas compostas é o uso de `pipes`, que permite toda esta comunicação entre processos de forma eficiente e sem erros.

= Escalonamento de Tarefas

O escalonador é responsável por armazenar e escolher tarefas para o servidor executar. Esta seleção tem em conta a política de escalonamento previamente definida, e tenta obter o menor tempo possível de espera na fila. A nossa implementação do escalonador conta com duas políticas de escalonamento diferentes: `FCFS` e `SJF`.

== Implementação

O escalonador, ou `scheduler`, está implementado de forma encapsulada e modular, sendo necessário inicialmente instanciá-lo, e podendo depois ser adicionadas ou removidas tarefas, ou pedido um estado da fila com os respetivos métodos. Para exemplificar, a utilização do escalonador é feita da seguinte forma:

```c
// Instanciação
Scheduler scheduler = create_scheduler(FCFS);

// Adição de uma tarefa
enqueue_process(scheduler, "ls /etc/ -la", 10);

// Estado do escalonador - lista de todas as tarefas na fila
Process* status = scheduler_status(scheduler) 

// Escolha de uma tarefa para ser executada
Process p = dequeue_process(scheduler);

// Libertação de toda a memória alocada, bem como das tarefas pendentes
destroy_scheduler(scheduler);
```

Internamente o escalonador aproveita-se da modularidade e da interface bem definida para as políticas de escalonamento para facilmente ser expandido. Para cada método do escalonador deve existir um método equivalente na implementação da política, que será armazenado e usado com recurso a funções de ponteiro. De seguida, podemos ver a estrutura do escalonador, com a assinatura dos seus métodos:

```c
typedef struct queue *Queue;
typedef int (*EnqueueFunction)(Queue, Process);
typedef Process (*DequeueFunction)(Queue);
typedef Process* (*StatusFunction)(Queue);
typedef void (*DestroyFunction)();

typedef struct scheduler
{
  SchedulePolicy policy;

  Queue queue;

  EnqueueFunction enqueue_fun;
  DequeueFunction dequeue_fun;

  StatusFunction status_fun;

  DestroyFunction destroy_fun;

} *Scheduler;
```

== Políticas de Escalonamento

=== First Come First Serve

A política `FCFS` consiste em escolher a tarefa que chegou primeiro à fila. Esta política é implementada com recurso a uma `circular queue`, que permite a adição e remoção de tarefas de forma fácil e eficiente. Assim, sempre que é adicionada ou removida uma tarefa, não há necessidade de se mover os elementos ou alocar mais espaço para lidar com as alterações. Quando a capacidade realmente é atingida, a fila é redimensionada para o dobro e todos os elementos são copiados para o início da nova fila.

=== Shortest Job First

A segunda política disponível é a `SJF`, que escolhe a próxima tarefa com base no tempo de execução estimado. Para tal, é usada uma `min-heap` para se atingir a menor complexidade nas operações de adição e remoção de tarefas.

== Performance

Para perceber que política se adequa melhor a cada cenário realizamos vários testes, onde nos focamos em variáveis como o número de tarefas requesitadas, e o tempo médio que estas demoram.

Para automatizar estes testes desenvolvemos um script em `bash` que requesita um número variado de tarefas, com tempos de execução dados por ordem crescente, decrescente ou aleatório (mas com soma total do tempo igual, independetemente da ordem). Este script executa, intercaladamente, os programas fornecidos pelos professores no BlackBoard - `hello` e `void`.

```bash
$ bin/runner.sh <número de tarefas> <asc | desc | random> <tempo mínimo> <tempo máximo>
```

=== Cenários

Os cenários que escolhemos tentam abrangir uma variadade de situações, em que podem existir várias tarefas curtas, longas, ou uma mistura de ambas.

Os resultados apresentados, em segundos, nas seguintes tabelas, foram recolhidos do ficheiro `log.txt`, que lista o tempo que uma tarefa leva desde a entrada na fila até ao fim da sua execução, e depois tratados no ficheiro `statistics.ods`. Para a nossa análise é nos relevante apenas o tempo de espera, que pode ser calculado ao subtrair o tempo estimado de execução ao tempo total, uma vez que essa estimativa é bastante próxima da realidade nos programas usados.

==== 1º Cenário

Para o primeiro cenário, foram executadas 100 tarefas, com tempos de execução crescentes, descrescentes e aleatórios entre 1 e 100 segundos e com um máximo de 3 tarefas em simultâneo.

#table(
  columns: 2,
  fill: (x, y) => if x == 1 { rgb("FFFFFF") } else { luma(240) },
  "Soma dos tempos de execução", "5050",
)

#table(
  columns: 7,

  "", table.cell( colspan: 3, "FCFS"), table.cell( colspan: 3, "SJF"),

  "", "Cresc.", "Decresc.", "Aleatório", "Cresc.", "Decresc.", "Aleatório",
  "Média", "555.966022", "1126.75302", "804.277562", "555.947148", "604.905078", "580.635962",
  "Máximo", "1665.3687", "1731.0704", "1694.9604",	"1665.1746", "1668.2747", "1663.6163"
)

==== 2º Cenário

No segundo cenário, foram executadas 3 000 tarefas, com tempos de execução aleatórios entre 1 e 5 segundos e um máximo de 6 tarefas em simultâneo.

#table(
  columns: 2,
  fill: (x, y) => if x == 1 { rgb("FFFFFF") } else { luma(240) },
  "Soma dos tempos de execução", "9000",
)

#table(
  columns: 3,

  "", "FCFS", "SJF",

  "Média", "766.4419126", "565.2882803",
  "Máximo", "1539.6931", "1539.6927"
)

==== 3º Cenário

No terceiro cenário, foram executadas 50 tarefas, com tempos de execução aleatórios entre 500 e 550 segundos e um máximo de 3 tarefas em simultâneo.

#table(
  columns: 2,
  fill: (x, y) => if x == 1 { rgb("FFFFFF") } else { luma(240) },
  "Soma dos tempos de execução", "26275",
)

#table(
  columns: 3,

  "", "FCFS", "SJF",

  "Média", "4265.13465", "4195.026936",
  "Máximo", "8629.6755", "8622.8741"
)

==== 4º Cenário

No quarto e último cenário, foram executadas 500 tarefas, com tempos de execução aleatório, em que 250 tarefas têm tempos de execução entre 1 e 10 segundos, e as restantes entre 100 e 350 segundos e novamente um máximo de 3 tarefas em simultâneo.

#table(
  columns: 2,
  fill: (x, y) => if x == 1 { rgb("FFFFFF") } else { luma(240) },
  "Soma dos tempos de execução", "57750",
)

#table(
  columns: 3,

  "", "FCFS", "SJF",

  "Média", "9724.804148", "4233.552515",
  "Máximo", "19674.6588", "19520.8571"
)

=== Conclusões sobre as políticas

Ao analisarmos as tabelas anteriores podemos tirar algumas conclusões sobre a performance de cada uma das políticas. 

Comecemos por notar que no cenário 1, a `FCFS` tem uma performance péssima com tempos de espera médios, quando os tempos de execução vêem de forma decrescente. Este já era um comportamento esperado uma vez que os processos mais demorados e que chegam primeiro monopolizam totalmente os recursos. Ao analisarmos os tempos crescentes percebemos que estes são os melhores em performance. Logo, juntando estas duas análises, a ordem de chegada de primeiro os mais curtos e depois os mais demorados parece ser vantajosa em termos de tempo de espera. Isto significa então que o `SJF` é uma boa estratégia, uma vez que o seu algoritmo é baseado justamente nesse fator? A verdade é que a ordem aleatória - que seria a ordem de tempos mais realista - também nos confirma que o `SJF` consegue ser mais eficiente, pelo menos neste primeiro cenário.

No segundo cenário tentamos executar o máximo de tarefas, todas com tempos relativamente curtos. O resultado mostrou-se uma vez mais favorável ao `SJF`, no entanto sem grandes alterações à ordem de grandeza da difereneça entre as duas políticas, e a nosso ver, ainda não suficiente para tirar conclusões. De notar que embora a difereneça aqui seja menor (200 segundos), foram executadas o dobro das tarefas em paralelo.

A verdade é que o terceiro cenário também não nos traz a certeza que procuravamos. Este cenário, que tenta simular um ambiente com tarefas mais longas, e que já traz uma grande diferença na soma dos tempos de execução, revelou uma das menores diferenças entre as duas políticas. A nosso ver, isto pode ser explicado por uma grande parte do tempo ser passado em execução - como foram executadas menos tarefas, e mais longas, a diferença de tempo de espera não foi tão significativa.

Apesar de não termos uma conclusão clara até agora, o último cenário mostrou-se o mais revelador. Aqui, a diferença entre as duas políticas está completamente dispersa dos últimos testes, mas acreditamos que por um bom motivo - este teste, que também pode ser visto como o mais realista - tenta juntar todos os testes anteriores, com várias tarefas longas, várias tarefas curtas, e uma soma de tempos de execução muito maior que os anteriores. Estes fatores permitiram ao SJF mostrar que consegue lidar tanto com processos curtos como longos, desde que lhe seja dado um número suficiente de tarefas para executar.

Assim, a nossa conclusão é que o SJF mostra-se sempre mais eficiente que o FCFS. Mesmo que inicialmente a diferença fosse mínima, o último teste deu-nos a certeza da decisão, uma vez que é também o teste mais completo. 

= Conclusão

Em suma, o desenvolvimento deste sistema de orquestração de tarefas permitiu-nos explorar e compreender em detalhe o funcionamento de um sistema de comunicação entre processos, bem como a execução de tarefas simples e compostas. A implementação deste orquestrador permitiu aprofundar a utilização das `system calls`, cruciais dentro do universo UNIX. Adicionalmente, podemos explorar diferentes políticas de escalonamento, bem como avaliar a sua eficiência e desempenho em diferentes cenários, permitindo-nos concluir que a política `SJF` é mais eficiente que a política `FCFS` na maioria dos cenários.

Durante o desenrolar do projeto conseguimos sempre superar os desafios que nos foram colocados sem grandes problemas, no entanto isto não tornou o projeto menos interessante, uma vez que aborda um tema que nos é bastante cativante.

Assim, este projeto permitiu-nos consolidar os conhecimentos adquiridos ao longo da unidade curricular de Sistemas Operativos, bem como desenvolver competências de programação em C e de resolução de problemas complexos, que são cruciais para o desenvolvimento de sistemas de software robustos e eficientes.
