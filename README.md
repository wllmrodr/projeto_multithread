Projeto Multithread (Simulação de Fábrica de Tecelagem)<br>
MC504A-Sistemas Operacionais (1º semestre / 2025)  
Professora Islene Calciolari Garcia  
Alunos:  
        • Antônio Carlos Carvalho Macedo (RA 199152)  
        • Giovanni Mesquita Micaroni     (RA 231702)  
        • Vicente José Campitelli Real   (RA 239834)  
        • William Rodrigues Lopes        (RA 248499)  

 
📌 Visão Geral do Projeto

    Este projeto implementa uma simulação multithread de uma fábrica de tecelagem, demonstrando conceitos fundamentais de sistemas operacionais como sincronização de threads, gestão de recursos compartilhados e prevenção de condições de corrida. O sistema modela:

        • Tecelões como threads trabalhadoras
        • Rolos de tecido como tarefas a serem processadas
        • Teares como recursos críticos
        • Bobinas como área de espera limitada

    A implementação utiliza:

        • a biblioteca pthreads para lidar com paralelismo
        • Semáforos para sincronização
        • ncurses para visualização interativa


🛠️ Estrutura do Código

    (fabrica_tecelagem.c) → Contém a lógica principal e visualização com ncurses → [Apresentação]
    (tecelao.h)	→ Cabeçalho com definições de constantes, estados e protótipos de funções → [Interface]
    (tecelao.c) → Implementação da lógica dos tecelões e rolos (sincronização) → [Lógica]

    O nosso grupo optou por modularizar o código dessa forma para obter uma manutenção e testabilidade mais simplificada, compilação mais eficiente e facilitar a compreensão do projeto como um todo.

🔍 Explorando os conceitos abordados

    🧵 Threads no Contexto do Projeto

        | Elemento      | Tipo de Thread | Comportamento          | Prioridade |
        | ------------- | -------------  | ---------------------- | ---------- |
        | Tecelão       | Worker         | Processa rolos         | Alta       |
        | Rolo          | Task           | Aguarda processamento  | Variável   |

        Fluxo típico:
            • Thread de rolo é criada
            • Tenta adquirir bobina (semáforo contador)
            • Espera por tear disponível (semáforo binário) 
            • Sinaliza término (variável de condição)

    🔐 Semáforos: O Coração da Sincronização

        | Semáforo      | Tipo          | Função                      | Valor Inicial |
        | ------------- | ------------- | --------------------------- | ------------- |
        | sem_bobinas   | Contador      | Controla vagas nas bobinas  |   N_BOBINAS   |
        | sem_tear[]    | Binário       | Gerencia acesso aos tears   |       1       |
        | sem_estados   | Mutex         | Protege estados globais     |       1       |


🎮 Controles e Visualização

    • Legenda de cores:

        🟢 Verde: Rolo sendo tecido
        🟡 Amarelo: Rolo esperando em bobina
        🔴 Vermelho: Tecelão ocioso
        🔵 Ciano: Bobina vazia

    • Símbolos:

        >: Rolo entrando na fábrica
        X: Rolo pronto (tecido finalizado)
        T: Rolo sendo tecido
        ... : Esperando
        ZZZ: Tecelão dormindo



📚 Bibliotecas externas utilizadas

    • time.h: Usada para controlar tempos e atrasos no funcionamento das threads.
    • errno.h: Usada para verificar erros em chamadas de sistema (como criação de threads, arquivos e alocação).
    • unistd.h: Fornece funções de baixo nível do sistema operacional UNIX/Linux, como: sleep(int segundos) – pausa uma thread por um número de segundos.
    
