Projeto SD:
Carolina Romeira - 59867
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321 

Notas/Reminders:

Enquanto se fazia o projeto notou-se que existia um warning no print do status em "client_hashtable.c":
%llu é o certo no Mac e em Ubuntu %lu.
No Mac aparece -> warning: format specifies type 'unsigned long' but the argument has type 'uint64_t'
Estabelecemos permanecer com o que o Ubuntu indica pois é o sistema fornecido/utilizado pela FCUL.

Já como acontecia na fase2, ao encerrar o servidor com CTRL+C o cliente não desconecta automaticamente do sistema,sendo necessário sempre escrever o comando "quit" no cliente.







Cada servidor criado/connectado com o zookeeper vai pedir a este mesmo pra criar um znode filho de /chain com o conteúdo do seu IP:porto.

Acrescentar ao server_hastable o IP:porto do zookeper como atributo.
Todos os servidores criados com o mesmo numero de listas na tabela.

Cliente passa-se a ligar ao zookeeper e a 2 servidores (head e tail) write-> head | read-> tail

