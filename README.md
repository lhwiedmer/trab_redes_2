# trab_redes_2
Implementação de um sistema cliente-servidor capaz de replicar arquivos do cliente para o servidor, e replicar os mesmos para N servidores réplicas.

Comandos possíveis:
#Imprime os comandos e suas descrições na tela
-> help
#Faz upload do arquivo com o nome <file> para o servidor princiapl e suas réplicas
-> upload <file>
#Imprime uma lista de todos os arquivos presentes no servidor principal na tela
-> list
#Fecha a conexão com o servidor principal
-> exit

Protocolo de comunicação
1024 bytes
1 byte     -> Código 
1023 bytes -> Payload

Mensagem de ID
1 byte     -> Código
1 byte     -> Tamanho do ID
n bytes    -> ID

Mensagem de FILEINFO
1 byte     -> Código
8 bytes    -> Tamanho do arquivo
1 byte     -> Tamanho do nome
n bytes    -> Nome

Mensagem de LISTASW
1 byte      -> Código
1 byte      -> More, last ou none(2, 1 e 0)
2 bytes     -> Número de nomes
n vezes
    1 byte  -> Tamanho do nome
    m bytes -> Nome






