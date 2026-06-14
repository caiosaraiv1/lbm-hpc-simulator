## Fase 1: Fundações, Ambiente e I/O Inicial
_O objetivo desta fase é garantir que o projeto compile no ecossistema da AMD e consiga ler/armazenar configurações corretamente antes de qualquer matemática complexa._
- [x] **Configuração do Ambiente de Build:** Criar o `Makefile` ou `CMakeLists.txt` configurado para usar o `hipcc` (compilador do ROCm) e ativar as flags de otimização pesada (`-O3`, `-ffast-math`).
- [ ] **Dicionário de Dados Base:** Criar o arquivo `types.h` definindo o macro de precisão simples (`typedef float real_t;`) e as structs mapeadas: `SimConfig`, `LatticeSoA` e `LBM_Context`.
- [ ] **Parser de Configuração:** Criar o arquivo `config.txt` com as variáveis do _Lid-driven cavity_ (nx, ny, max_iters, save_interval, omega, u_lid).
- [ ] **Leitura de Input:** Implementar a função em C (`load_config`) para ler o `.txt` via `fscanf` e popular a struct `SimConfig`.
## Fase 2: Alocação de Memória e Setup do Device (VRAM)
_O foco aqui é gerenciar o layout de memória (SoA) no Host (CPU) e no Device (GPU), preparando o terreno para o processamento._
- [ ] **Alocação no Host (RAM):** Implementar a função para alocar os arrays contíguos da struct `LatticeSoA` na memória da CPU (usando `malloc` ou `posix_memalign` para alinhamento de cache).
- [ ] **Alocação no Device (VRAM):** Implementar a função correspondente usando `hipMalloc` para espelhar a `LatticeSoA` na memória da GPU.
- [ ] **Inicialização do Fluido:** Implementar o kernel de setup inicial (zerar as flutuações $\Delta f_i$, definir densidade base como 1.0 e velocidades como 0.0).
- [ ] **Lógica de Ping-Pong:** Criar a infraestrutura do loop temporal principal instanciando os dois ponteiros (`lattice_in` e `lattice_out`) e testar a troca de ponteiros (Pointer Swapping) rodando um loop vazio.
## Fase 3: O Motor Matemático na GPU (Kernels)
_O coração do simulador. Aqui a física ganha vida utilizando a arquitetura de alta performance (Shifted LBM, Fusão de Kernels e Separação de Contornos)._
- [ ] **Kernel Principal (Miolo do Fluido):** Implementar o kernel fundido de Colisão + Propagação rodando estritamente para as coordenadas internas ($1$ a $nx-2$, $1$ a $ny-2$). Utilizar a abordagem _Pull (Gather)_ para ler os 9 vizinhos e gravar o resultado de forma coalescida.
- [ ] **Kernel de Contorno - Paredes Estáticas:** Implementar o kernel separado para tratar o _Bounce-Back_ (rebatimento) nas paredes esquerda, direita e inferior.
- [ ] **Kernel de Contorno - Tampa Móvel (Lid):** Implementar o kernel dedicado à parede superior, injetando a velocidade macroscópica da tampa (`u_lid`) nas distribuições.
- [ ] **Sincronização Host-Device:** Implementar a cópia dos dados (`hipMemcpy` com flag `hipMemcpyDeviceToHost`) da GPU para a CPU apenas quando for o momento de salvar/exportar dados, evitando gargalos no loop principal.
## Fase 4: Exportação de Dados e Validação Visual
_Garantir que os números calculados na VRAM se transformem em algo que o ParaView consiga renderizar._
- [ ] **Cálculo Macroscópico:** Implementar a função (ou kernel) que reduz os valores de $\Delta f_i$ para encontrar a flutuação da densidade ($\Delta \rho$) e o campo de velocidade ($u_x, u_y$) em cada nó.
- [ ] **Exportador VTK:** Escrever a função `export_vtk` que formata a malha macroscópica em um arquivo ASCII `.vtk` legível pelo ParaView (utilizando `fprintf`).
- [ ] **Integração do Loop Principal:** Amarrar os kernels da Fase 3 e o exportador da Fase 4 dentro do `while (t < max_iters)`, controlando a chamada do VTK através da variável `save_interval`.
- [ ] **Primeiro Teste de Fumaça:** Rodar a simulação, abrir o `.vtk` gerado no ParaView e validar se o redemoinho clássico do _Lid-driven cavity_ está se formando corretamente.
## Fase 5: Tolerância a Falhas e HPC Profiling
_A camada de engenharia de software avançada que transforma um projeto acadêmico em um projeto de classe corporativa/AWS._
- [ ] **Sistema de Checkpointing (Save):** Implementar a escrita do estado binário completo do `LatticeSoA` (as 9 direções) no disco a cada $X$ milhares de iterações.
- [ ] **Sistema de Recovery (Load):** Alterar a inicialização do programa para checar se existe um arquivo de checkpoint válido; se sim, carregar via `hipMemcpyHostToDevice` e continuar do passo temporal salvo em vez de reiniciar do zero.
- [ ] **Aceleração de Pré/Pós Processamento:** Injetar diretivas `#pragma omp parallel for` nas funções de leitura/gravação da CPU para acelerar a manipulação de arrays no Host antes de enviá-los ou após recebê-los da GPU.
- [ ] **Profiling com `rocprof`:** Rodar a ferramenta de análise da AMD para verificar a ocupação dos _Wavefronts_ e confirmar se as leituras/escritas da GPU estão operando em regime 100% coalescido.