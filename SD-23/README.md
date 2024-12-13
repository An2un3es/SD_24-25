Projeto SD:
Carolina Romeira - 59867
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321 

Notas/Reminders:

FASE 3:
Enquanto se fazia o projeto notou-se que existia um warning no print do status em "client_hashtable.c":
%llu é o certo no Mac e em Ubuntu %lu.
No Mac aparece -> warning: format specifies type 'unsigned long' but the argument has type 'uint64_t'
Estabelecemos permanecer com o que o Ubuntu indica pois é o sistema fornecido/utilizado pela FCUL.



FASE 4:

De maneira aleatória um servidor não se consegue ligar devido a um problema ao trocar ao ordenar os z_nodes. 

Ao ler o guião ficou dúbio se o numero de ligações ao executar o comando "stats" deve incluir o nºtotal de clientes ligados + o servidor anterior ou não. Optamos por deixar o número de ligações total.

Pensámos em criar um ficheiro semlhante a zookeeper_usage.c para o cliente, no entanto devido ao dias a menos que tivemos de entrega, acabamos por optar ser mais diretos.

Se o servidor head for desconectado os clientes entram em loop.

