#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

typedef struct str_info{
    int str_len;
    int indent_len;
} str_info;

typedef struct table{
    int lines;
    str_info* matrix;
} table;

int fd;
int flen = 0;
table* strs_info = NULL;
char* buff = NULL;
off_t offset = 0;

void print_table(){
    printf("\nnumber indent_len str_len\n");
    for(int i = 0; i < strs_info->lines; i++){
        printf("%d\t%d\t%d\n", i + 1, 
                strs_info->matrix[i].indent_len,
                strs_info->matrix[i].str_len);
    }
    printf("lines: %d, flen: %d\n\n", strs_info->lines, flen);
}

void new_line(){
    strs_info->lines += 1;
    strs_info->matrix = (str_info*)realloc(strs_info->matrix, sizeof(str_info) * strs_info->lines);
    strs_info->matrix[strs_info->lines - 1].indent_len = 0;
    strs_info->matrix[strs_info->lines - 1].str_len = 0;
}

int count_indents(){
    char sym;
    while(mmap(&sym, sizeof(char), PROT_READ, MAP_PRIVATE, fd, sizeof(char) * offset++) != -1){
        flen += 1;
        if(sym == ' '){
            strs_info->matrix[strs_info->lines - 1].indent_len += 1;
        }
        else{
            strs_info->matrix[strs_info->lines - 1].str_len += 1;
            if(sym == '\0'){
                return 0;
            }
            if(sym == '\n'){
                return 1;
            }
            break;
        }
    }
}

int count_len(){
    char sym;
    while(mmap(&sym, sizeof(char), PROT_READ, MAP_PRIVATE, fd, sizeof(char) * offset++) != -1){
        flen += 1;
        strs_info->matrix[strs_info->lines - 1].str_len += 1;
        if(sym == '\n'){
            return 1;
        }
        if(sym == '\0'){
            return 0;
        }
    }
}

void get_strs_info(int fd){
    strs_info = (table*)malloc(sizeof(table));
    strs_info->lines = 0, strs_info->matrix = NULL;
    new_line();
    while (1){
        if(count_indents() == 0){
            break;
        }
        if(count_len() == 0){
            break;
        }
        new_line(); 
    }
}

void print_str(int line){
    line -= 1;
    offset = 0;
    for(int i = 0; i < line; i++){
        offset += strs_info->matrix[i].indent_len;
        offset += strs_info->matrix[i].str_len;
    }
    offset += strs_info->matrix[line].indent_len;

    int len = strs_info->matrix[line].str_len;
    mmap(buff, sizeof(char) * len, PROT_READ, MAP_PRIVATE, fd, sizeof(char) * offset);
    buff[len] = '\0';
    printf("%s\n", buff);
}

void free_mem(){
    free(buff);
    free(strs_info->matrix);
    free(strs_info);
}

void print_file(int signum){
    mmap(buff, sizeof(char) * flen, PROT_READ, MAP_PRIVATE, fd, 0);
    printf("\n\nFILE:\n%s\n", buff);
    free_mem();
    exit(0);
}

int main(int argc, char** argv){
    if(argc != 2){
        printf("ERROR\nWrong number of parametres\n");
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if(fd == -1){
        perror("\nERROR\nCannot open the file");
        return 1;
    }

    get_strs_info(fd);
    print_table(strs_info);
    buff = (char*)malloc(sizeof(char) * (flen + 1));
    buff[flen] = '\0';
    
    printf("USAGE:\nEnter line numbers one at time\nPrint 0 to end\n\n");
    int line;
    while(1){
        printf("in: ");
        signal(SIGALRM, print_file);
        alarm(5);
        scanf("%d", &line);
        if(line == 0 || line < 0  || line > strs_info->lines){
            return 0;
        }
        printf("out: ");
        print_str(line);
    }
    
    free_mem();
    return 0;
}