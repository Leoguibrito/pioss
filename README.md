# PIOSS Lite — experimentos de amostragem de carga

Simulador em C para estudar a distribuição de bytes de escrita entre servidores
de dados (DTS) de um sistema de arquivos paralelo.

Esta extensão compara informação parcial e global de carga, variabilidade de
tamanhos de arquivos e largura de striping. Carga significa **bytes atribuídos
durante esta execução**, iniciando todos os DTS em zero. A execução é sequencial:
os bytes de um arquivo são atribuídos antes da seleção dos destinos do próximo.

Não há rede, filas, discos, migração ou modelo temporal de I/O. O tempo de execução
do programa não é uma previsão de throughput ou latência. Consultas contam
informação acessada, não mensagens ou custo real de comunicação.

## Principais alterações deste fork

Este fork estende o [PIOSS original](https://github.com/ecamiloinacio/pioss) para
investigar como o número de servidores consultados afeta o balanceamento das
escritas, considerando a variabilidade dos tamanhos dos arquivos e a largura de
striping. As principais mudanças são:

- **Tamanhos individuais por arquivo:** além do volume fixo por cliente definido
  por `-b`, o simulador aceita uma sequência de tamanhos via `--workload_path`.
  O simulador valida a sequência, calcula estatísticas dos tamanhos e identifica
  a entrada por SHA-256.
- **Políticas com consulta de carga:** foram adicionadas `PowerOfD`, que consulta
  até `d` candidatos para cada destino do arquivo, e `LeastLoaded`, que consulta
  todos os DTS uma vez por arquivo. A interface das políticas passou a receber
  um contexto com configuração, gerador aleatório e função de consulta de carga.
- **Instrumentação e métricas:** o núcleo contabiliza consultas totais e DTS
  distintos consultados por arquivo. Também calcula o coeficiente de variação
  dos bytes entre servidores e a razão entre carga máxima e média, verificando
  a conservação do volume total de escrita.
- **Resultados completos e rastreáveis:** `--results_dir` exporta `servers.csv`,
  `files.csv` e `summary.json`, com destinos por arquivo, consultas, configuração,
  semente da política e identificação do workload e do código. O CSV simples
  produzido por `-o` continua disponível.
- **Correções e validação do software:** foram corrigidos o parser de argumentos,
  validações numéricas, estouros, vazamentos e falhas de exportação. O build
  declara explicitamente o projeto C e oferece execução com sanitizadores.

**Compatibilidade:** Random, RoundRobin e N2R2 continuam disponíveis. Random
passou a usar Fisher–Yates com sorteios limitados sem viés; por isso, suas
sequências diferem da versão original mesmo com a mesma semente. RoundRobin
mantém o início aleatório por arquivo, sem cursor global. As novas políticas e
os workloads externos usam arquivos independentes; o modo compartilhado `-s`
permanece disponível com volume fixo por cliente e as políticas originais.

## Compilar

Requisitos: compilador C99, CMake >= 3.12 e uma ferramenta de build compatível
com o gerador escolhido pelo CMake, como Make ou Ninja. O código utiliza
interfaces POSIX.

Na raiz do repositório, onde está o arquivo `CMakeLists.txt`, execute:

~~~bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
~~~

O executável será gerado em `build/src/piosslite`. Após alterar os fontes,
recompile com `cmake --build build --parallel`.

Opcionalmente, para compilar com AddressSanitizer e UndefinedBehaviorSanitizer
em um ambiente GCC/Clang compatível:

~~~bash
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DPIOSS_SANITIZE=ON
cmake --build build-sanitize --parallel
./build-sanitize/src/piosslite -n 4 -m 4 -w 2 -f N2R2 -r 42
~~~

## Executar o simulador

Execute os comandos a seguir na raiz do repositório:

~~~bash
# Mostrar as opções.
./build/src/piosslite -h

# Executar com os parâmetros padrão.
./build/src/piosslite

# Quatro clientes, 1 MiB cada, quatro DTS e dois destinos por arquivo.
./build/src/piosslite -n 4 -b 1048576 -m 4 -w 2 -z 65536 -f N2R2 -r 42

# 128 clientes, 64 MiB cada, 16 DTS, quatro destinos por arquivo,
# fragmentos de 1 MiB e dois candidatos por seleção de destino.
./build/src/piosslite \
  -n 128 -b 67108864 -m 16 -w 4 -z 1048576 \
  -f PowerOfD --sample_size 2 -r 42
~~~

### Tamanhos diferentes por arquivo

Crie um arquivo chamado `workload.txt` com um tamanho inteiro positivo em bytes
por linha. Por exemplo, para três clientes escrevendo 1 MiB, 8 MiB e 2 MiB:

~~~text
1048576
8388608
2097152
~~~

Informe a sequência ao simulador e use o mesmo número de clientes:

~~~bash
./build/src/piosslite \
  -n 3 -m 4 -w 2 -z 65536 -f PowerOfD --sample_size 2 -r 42 \
  --workload_path workload.txt
~~~

Sem `--workload_path`, todos os clientes usam o volume definido por `-b`.
Fixe a semente `-r` e mantenha a mesma sequência e ordem dos arquivos para
reproduzir uma execução.

### Exportar resultados

~~~bash
mkdir -p output

./build/src/piosslite \
  -n 4 -b 1048576 -m 4 -w 2 -z 65536 -f N2R2 -r 42 \
  --results_dir output/exemplo
~~~

O diretório passado a `--results_dir` deve ser novo e seu pai já deve existir.
Os resultados não são sobrescritos.

Para exportar apenas os bytes por servidor, indique um arquivo novo com `-o`:

~~~bash
./build/src/piosslite -n 4 -m 4 -w 2 -f N2R2 -r 42 -o output/servidores.csv
~~~

### Parâmetros

| Opção | Padrão | Significado |
|---|---|---|
| -n / --num_cli | 1 | Número de clientes; um arquivo independente por cliente |
| -b / --data_size | 1048576 | Bytes por cliente no modo homogêneo |
| -s / --shared | desligado | Arquivo compartilhado; somente políticas originais |
| -m / --num_dts | 2 | Total N de DTS |
| -w / --stripe_width | 1 | Número w de destinos do arquivo; 1 <= w <= N |
| -z / --stripe_size | 65536 | Bytes por fragmento; positivo |
| -f / --file_dist | Random | Política de distribuição |
| -r / --rng_seed | relógio | Semente uint32 da política; fixe-a para reproduzir |
| --sample_size | ausente | Número d de candidatos; obrigatório e exclusivo de PowerOfD |
| --workload_path | ausente | Sequência externa de tamanhos |
| -o / --out_path | ausente | CSV original DTS,Bytes; caminho novo ou diretório existente |
| --results_dir | ausente | Pacote de resultados em diretório novo |
| -q / --quiet | desligado | Suprimir a saída normal |
| -h / --help | — | Ajuda |

O arquivo de workload tem um inteiro decimal positivo em bytes por linha não
vazia, exatamente n valores. Espaços nas extremidades são aceitos. A ordem das
linhas é preservada. A soma deve caber em uint64 (2^64-1).
workload_path é incompatível com -b explícito e -s.
out_path e results_dir são mutuamente exclusivos.
As novas políticas não aceitam -s, mesmo com carga homogênea.
Falhas de entrada ou exportação retornam código de saída diferente de zero.

### Políticas

| Nome | Seleção | Consultas por arquivo |
|---|---|---|
| Random | Amostra uniforme ordenada de w DTS distintos | 0 |
| RoundRobin | Início aleatório por arquivo, seguido de destinos circulares | 0 |
| N2R2 | Base comum e início definido por fid e max(N/num_files,1) | 0 |
| PowerOfD | Para cada destino, amostra candidatos elegíveis e escolhe o menos carregado | soma min(d,N-j), j=0..w-1 |
| LeastLoaded | Consulta N DTS uma vez e escolhe os w menos carregados | N |

PowerOfD exige 1 <= d <= N. A amostra é renovada a cada destino, sem reposição
dentro da amostra; o destino escolhido é removido dos elegíveis daquele arquivo.
Um candidato não escolhido pode ser consultado novamente. Não há cache entre
seleções. Empates nas políticas informadas são resolvidos aleatoriamente.
Nenhuma consulta observa as atribuições do próprio arquivo antes de sua seleção
terminar. As políticas não antecipam o workload futuro.

LeastLoaded é uma heurística com informação completa, não uma solução ótima.
PowerOfD com d=N encontra as mesmas cargas mínimas por posição em um mesmo
estado, mas pode usar mais consultas e resolver empates de outra forma; isso
pode produzir distribuições posteriores diferentes quando há fragmentos residuais.

RoundRobin mantém o comportamento histórico do PIOSS: não existe cursor global
entre arquivos. A versão anterior de Random tinha um embaralhamento enviesado e
um vazamento; Fisher–Yates com sorteios limitados sem viés corrige ambos.
**As sequências de Random mudaram**, mesmo com a mesma semente. RoundRobin e
N2R2 preservam o sorteio TinyMT original; o produto fid*stride usa uint64 para
evitar estouro em cenários grandes.

### Pacote de resultados

- servers.csv: colunas DTS,Bytes.
- files.csv: client,fid,size_bytes,destinations,queries,distinct_queried.
  Os destinos aparecem na ordem das posições de striping, separados por ponto
  e vírgula. Incluem destinos que podem receber zero bytes em arquivos pequenos.
- summary.json: configuração, semente da política, hash do workload, revisão,
  hash dos fontes C/cabeçalhos, métricas do workload e métricas dos DTS.

A identificação do workload é SHA-256 da sequência canônica:
um inteiro decimal por linha com newline, independentemente de espaços na entrada.
O hash dos fontes é SHA-256 da concatenação ordenada dos hashes de cada fonte e
cabeçalho em src e lib.

A carga por destino pode ser reconstruída com o tamanho do arquivo, stripe_size,
stripe_width e a posição de cada DTS na lista. No modo compartilhado original,
files.csv contém uma linha por cliente, com fid=0 e a mesma lista de destinos.

O CV é populacional, calculado sobre todos os DTS, inclusive os que receberam zero
bytes. max_mean_ratio é a maior carga dividida pela média.
Consultas para métricas/exportação não entram nos contadores das políticas.

## Referência do projeto original

INACIO, Eduardo C.; DANTAS, Mario A. R. PIOSS: A Simulation Model for the Analysis
of Parallel I/O Performance Variability on Large-scale Applications.
WSCAD 2022, p. 300–311. DOI: https://doi.org/10.5753/wscad.2022.226350

Licença: ver LICENSE.
