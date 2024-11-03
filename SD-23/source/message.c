/* Grupo 23
Gabriel Gameiro - 56299
Rodrigo Antunes - 56321
Carolina Romeira - 59867
*/

#include <unistd.h> // para o write() e read()
#include "message-private.h"
/**
 * Escreve a o conteudo que esta no socket
 * @param sock descritor do socket
 * @param buf buffer em questão
 * @param len tamanho do babuffer
 */
int write_all(int sock, void *buf, int len) {
    int total_written = 0;
    int bytes_left = len;
    char *ptr = (char *)buf;

    while (total_written < len) {
        int written = write(sock, ptr + total_written, bytes_left);
        
        if (written <= 0) {
            // Se ocorrer erro, devolve -1
            return -1;
        }

        total_written += written;
        bytes_left -= written;
    }


    return 0;
}
/**
 * Le a o conteudo que esta no socket
 * @param sock descritor do socket
 * @param buf buffer em questão
 * @param len tamanho do babuffer
*/
int read_all(int sock, void *buf, int len) {
    int total_read = 0;
    int bytes_left = len;
    char *ptr = (char *)buf;

    while (total_read < len) {
        int read_bytes = read(sock, ptr + total_read, bytes_left);
        
        if (read_bytes < 0) {
            // Erro de leitura
            return -1;
        } else if (read_bytes == 0) {
            // Socket fechado antes de receber tudo, erro
            return -1;
        }

        total_read += read_bytes;
        bytes_left -= read_bytes;
    }


    return 0;
}
