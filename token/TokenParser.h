#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define IS_WORD(chr) ((chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z') || chr == '_' || chr == '!' || chr == '@' || chr == '$')
#define IS_DIGIT(chr) (chr >= '0' && chr <= '9')
#define IS_WS(chr) (chr == ' ' || chr == '\f' || chr == '\n' || chr == '\r' || chr == '\t' || chr == '\v' || chr == '\xA0')
#define IS_QUOTE1(chr) (chr == '\"') 
#define IS_QUOTE2(chr) (chr == '\'')
#define IS_SYMBOL(chr) (chr == '.' || chr == '+' || chr == '-' || chr == '(' || chr == ')' \
            || chr == '{' || chr == '}' || chr == '[' || chr == ']' || chr == '<' \
            || chr == '>' || chr == '=' || chr == ';' || chr == '^' || chr == '/' \
            || chr == ':' || chr == ',' || chr == '~' || chr == '*' || chr == '#' || chr == '&' || chr == '|')
#define TEST_KEYWORD(chr) (IS_WORD(chr) || IS_DIGIT(chr))
#define TEST_NUM_VAL(chr) (IS_DIGIT(chr) || chr == 'e' || chr == 'E' || chr == '.' \
                || chr == '+' || chr == '-' || chr == '*' || chr == '/' || chr == '%')
#define REMARK(chr) (chr == '-')
#define REMARK_LEFT(chr) (chr == '[')
#define REMARK_RIGHT(chr) (chr == ']')
#define IS_PARENT(chr) (chr == '(' || chr == ')' || chr == '{' || chr == '}' || chr == '[' || chr == ']' || chr == ';' || chr ==':')
#define IS_EXCEPT(chr) (chr == '[' || chr == ']' || chr==':')
#define EQ(chr) (chr == '=')


int variable_index = 0;

typedef struct {
    size_t cursor;
    size_t length;
    const char* input;
} parser_t;

typedef struct {
    const char* str;
    size_t offset;
    size_t length;
} token_t;

typedef struct dictionary {
    char *str[10000];
    size_t cnt;
};

typedef struct vec{
    unsigned int enc[10000];
    size_t cnt;
};


struct dictionary dic={
    .cnt = 0
};
struct dictionary val_dic={
    .cnt = 0
};
struct vec enc_vec={
    .cnt =0
};
struct dictionary enc_dic={
    .cnt =0
};


void print_map(){
    for(int i=1;i<=dic.cnt;i++){
        printf("dic.str[%d] : %s\n", i, dic.str[i]);
    }
    for(int i=1;i<=val_dic.cnt;i++){
        printf("val_dic.str[%d] : %s\n", i, val_dic.str[i]);
    }
}

void print_enc(){
    for(int i=0;i<enc_vec.cnt;i++){
        printf("%d ", enc_vec.enc[i]);
    }
}

void init(){
    clean();
    dic.cnt = 0;
    val_dic.cnt = 0;
    enc_vec.cnt = 0;
    enc_dic.cnt = 0;
    memset(dic.str, 0, sizeof(dic.str));
    memset(enc_vec.enc, 0, sizeof(enc_vec.enc));
    memset(val_dic.str, 0, sizeof(val_dic.str));
    memset(enc_dic.str, 0, sizeof(enc_dic.str));
}


char *copy(const char *str, size_t begin, size_t len) {
    char *cp = (char*)calloc(10000, sizeof(char));
    strncpy(cp, &str[begin], len);
    //cout << copy << endl;
    return cp;
}

token_t *newtoken(size_t offset, size_t length) {
    token_t *token = (token_t*)calloc(1,sizeof(token_t));
    token->offset = offset;
    token->length = length;
    return token;
}

int val_dic_find(char* target) {
    int j=0;
    if(val_dic.cnt == 0) return 0;
    for(j=1;j<=val_dic.cnt;j++){
        if(strcmp(val_dic.str[j], target) == 0)
            return (j); // val_dic는 idx가 dict 뒤에 이어져야 함.
    }
    return j-1;
}

int isKeyword(char* target){
    for(int i=1;i<=dic.cnt;i++){
        if(strcmp(target, dic.str[i]) == 0){
            return 1;
        }
    }
    return 0;
}

token_t *parse_keyword(parser_t *p){
    size_t begin = p->cursor;
    size_t current = p->cursor;
    while (TEST_KEYWORD(p->input[current])){
        current++;
    }
    size_t length = current - begin;
    char* word = copy(p->input, begin, length);
    if(!isKeyword(word)) { //variable
        if(val_dic_find(word) == val_dic.cnt){
            //printf("Not Found keyword: %s\n", word);
            val_dic.cnt++;
            val_dic.str[val_dic.cnt] = (char*)calloc(10000, 1);
            strcpy(val_dic.str[val_dic.cnt], word);
        }
    }
    free(word);
    return newtoken(begin, length);
}


void get_token_dict() {
    char* buf;
    size_t len = 0;
    ssize_t r;
    FILE *fp = fopen("./token", "r");
    
    if (fp == NULL) exit(-1);
    
    while((r = getline(&buf, &len, fp)) != -1){
        char *t;
        int idx = 0;
        t = strtok(buf, " ");
        idx = atoi(strtok(NULL, " "));
        //printf("%s %d\n", t, idx);
        
        dic.str[idx] = (char*)calloc(100, 1);
        strcpy(dic.str[idx], t);
        
        dic.cnt++;
    }
    free(buf);
    fclose(fp);
}

token_t *parse_value(parser_t *p, const char quote) {
    size_t begin = p->cursor;
    size_t current = p->cursor;
    
    if(quote == '\"') {
        current++;
        while (!IS_QUOTE1(p->input[current])) {
            current++;
        }
        current++;
    }
    else if(quote == '\''){
        current++;
        while (!IS_QUOTE2(p->input[current])) {
            current++;
        }
        current++;
    }
    else { //flaot or integer
        while (TEST_NUM_VAL(p->input[current])) {
            current++;
        }
    }

    size_t length = current - begin;
    char* word = copy(p->input, begin, length);

    if(val_dic_find(word) == val_dic.cnt){
        val_dic.cnt++;
        val_dic.str[val_dic.cnt] = (char*)calloc(10000, 1);
        strcpy(val_dic.str[val_dic.cnt], word);
    }
    free(word);
    return newtoken(begin, length);
}

token_t *parse_symbol(parser_t *p) {
    size_t begin = p->cursor;
    size_t current = p->cursor;
    
    if(IS_PARENT(p->input[current])){
        if(IS_EXCEPT(p->input[current])){
            if(IS_EXCEPT(p->input[current+1])){
                current++;
            }
        }
        current++;
    }
    else{
        while (IS_SYMBOL(p->input[current])) {
            current++;
        }
    }
    
    size_t length = current - begin;
    //const char* word = copy(p->input, begin, length);
    //delete []word;
    return newtoken(begin, length);
}



char* get_enc_dic(){
    char *text = (char*)calloc(10000, sizeof(char));

    for(int i=0;i<enc_dic.cnt;i++){
        strcat(text, enc_dic.str[i]);
    }
    //clean_enc();
    return text;
}

void clean(){
    //printf("[*]cleaning!\n");
    for(int i=1;i<=dic.cnt;i++){
        free(dic.str[i]);
    }
    for(int i=1;i<=val_dic.cnt;i++){
        free(val_dic.str[i]);
    }
}

void clean_enc(){
    for(int i=0;i<enc_dic.cnt;i++){
        free(enc_dic.str[i]);
    }
}


unsigned int encoding(char* target) {
    int i, j;
    for(i=1;i<=dic.cnt; i++){
        if(strcmp(dic.str[i], target) == 0)
            break;
    }
    if ( i <= dic.cnt){
        return i;
    }
    else {
        for(j=1;j<=val_dic.cnt;j++){
            if(strcmp(val_dic.str[j], target) == 0)
                return (j+dic.cnt); // val_dic는 idx가 dict 뒤에 이어져야 함.
        }
    }
    //printf("ERROR TARGET : %s;\n", target);
    return 0;
}


int encode(char** testcase, unsigned int* len){
    //printf("[*] START ENCODING !!\n");
    init();
    get_token_dict();
    //printf("[*] GET TOKEN DICT!!\n");
    //print_map();

    char * code = (char*)calloc(10000, sizeof(char));
    memcpy(code, *testcase, *len);
    //printf("code: %s\n", code);
    //exit(1);
    parser_t parser = {
        .cursor = 0,
        .length = *len,
        .input = code
    };

    
    //printf("parser.length: %ld\n", parser.length);

    while (parser.cursor < parser.length) {
        const char chr = parser.input[parser.cursor];
        token_t * token = NULL;
        //printf("chr: %c\n", chr);

        if(REMARK(chr)){
            const char tmp = parser.input[parser.cursor+1];
            if(REMARK(tmp)){
                //printf("REMARK!\n");
                parser.cursor++;
                //printf("r1: %c\n", parser.input[parser.cursor]);
                //printf("r2: %c\n", parser.input[parser.cursor+1]);
                
                if(REMARK_LEFT(parser.input[parser.cursor+1])){
                    parser.cursor++;
                    if(REMARK_LEFT(parser.input[parser.cursor+1])){
                        //printf("REMARK2!\n");
                        parser.cursor++;
                        char ex1 = 0;
                        char ex2 = 0;
                        do{
                            parser.cursor++;
                            ex1 = parser.input[parser.cursor++];
                            ex2 = parser.input[parser.cursor];
                        } while(!REMARK_RIGHT(ex1) && !REMARK_RIGHT(ex2));
                        parser.cursor++;
                    }
                    else if(EQ(parser.input[parser.cursor+1])){
                        //printf("REMARK3!\n");
                        parser.cursor++;
                        if(REMARK_LEFT(parser.input[++parser.cursor])){
                            char ex1 = 0;
                            char ex2 = 0;
                            char ex3 = 0;
                            do{
                                ex1 = parser.input[parser.cursor+1];
                                ex2 = parser.input[parser.cursor+2];
                                ex3 = parser.input[parser.cursor+3];
                                //printf("ex1 : %c\n", ex1);
                                //printf("ex2 : %c\n", ex2);
                                //printf("ex3 : %c\n", ex3);
                                parser.cursor++;
                            } while((!REMARK_RIGHT(ex1)) || (!EQ(ex2)) || (!REMARK_RIGHT(ex3)));
                    
                        parser.cursor++;
                        parser.cursor++;
                        parser.cursor++;
                        }
                    }
                    
                }
                
                else {
                    while (parser.input[parser.cursor] != '\n'){
                        parser.cursor++;
                    }
                }
            }
            else {
                token = parse_symbol(&parser);
            }
        }
        else if (IS_WORD(chr)) { //variable or keyword
            //printf("IS_WORD\n");
            token = parse_keyword(&parser);
            }
            else if (IS_QUOTE1(chr) || IS_QUOTE2(chr)) { //string value
                if(IS_QUOTE1(chr)) {
                token = parse_value(&parser, '\"');
            }
            else {
                token = parse_value(&parser, '\'');
            }
        }
        
        else if (IS_WS(chr)) { //whitespace
            //printf("IS_WS\n");
            parser.cursor++;
        }
        else if (IS_SYMBOL(chr)) { //symbol | class?
            //printf("IS_SYMBOL\n");
            token = parse_symbol(&parser);	
        }
        else { //number value
            //printf("IS_NON\n");
            //printf("chr: %c\n", chr);
            //print_map();
            //print_enc();
            //exit(1);
            token = parse_value(&parser, ' ');
        }

        //print_map();
        if (token) {
            parser.cursor += token->length;
            //printf("token->offset : %d\n", token->offset);
            //printf("token->length : %d\n", token->length);
            
            char* str = copy(code, token->offset, token->length);
            
            //encoding "str" and store it
            //c++ : enc_vec.push_back(encoding(str))
            //printf("str: %s\n", str);
            
            enc_vec.enc[enc_vec.cnt++] = encoding(str);
            free(str);
            free(token);
        }
    }

    MakeBinary(testcase);
    *len = enc_vec.cnt*2;
    free(code);
}

int encode_file(char* fn, unsigned int* len){
    FILE *fp = fopen(fn, "r");
    char *buf = (char*)calloc(10000, sizeof(char));
    char *tmp = (char*)calloc(10000, sizeof(char));

    //printf("fn: %s\n", fn);
    //printf("fp: %p\n", fp);
    
    fread(buf, 1, 10000, fp);
    
    *len = strlen(buf);

    //printf("enc buf: %s\n", buf);
    //printf("enc len: %d\n", *len);
    encode(&buf, len);
    //print_enc();
    //print_map();
    memcpy(tmp, buf, *len);
    fclose(fp);


    FILE *fd = fopen(fn, "w+");
    //printf("len: %d", *len);

    fwrite(tmp, 1, *len, fd);
    fclose(fd);
    free(buf);
    free(tmp);
}

char *decoding(int idx){
    if(idx <= 0) return "";
    if(idx <= dic.cnt) return dic.str[idx];
    else if(idx <= dic.cnt+val_dic.cnt) return val_dic.str[idx-dic.cnt];
    return "";
}

int decode(void** text, int* len){
    //printf("[*] START DECODING!!\n");
    unsigned short *buf = (unsigned short*)calloc(10000, sizeof(wchar_t));
    if(!buf) {
        printf("malloc ERROR!\n");
        exit(1);
    };
    
    memcpy(buf, *text, *len);
    //printf("buf: %s\n", buf);
    for(int i=0;i<enc_vec.cnt;i++){
        char *tmp = " ";
        //printf("buf[%d] : %d\n", i, buf[i]);
        if ( buf[i] > (dic.cnt + val_dic.cnt))
            buf[i] = (buf[i] % (dic.cnt+val_dic.cnt));
        enc_dic.str[enc_dic.cnt++] = decoding(buf[i]);
        enc_dic.str[enc_dic.cnt++] = tmp;
    }
    *text = get_enc_dic();
    *len = strlen(*text);
    
    //clean_enc();
    enc_dic.cnt = 0;
    free(buf);
    //clean();
    return 0;
}

int decode_file(char* fn, int *len){
    FILE *fp = fopen(fn, "r");
    char *buf = (char*)calloc(10000, sizeof(char));
    char *tmp = (char*)calloc(10000, sizeof(char));
    if(!buf) return 0;
    if(!tmp) return 0;

    memset(buf, 0, 10000);
    memset(tmp, 0, 10000);

    fread(buf, 1, 10000, fp);
    //printf("fn: %s\n", fn);
    //printf("fp: %p\n", fp);
    //*len = strlen(buf);

    //printf("buf: %s\n", buf);
    //printf("len: %d\n", *len);
    decode(&buf, len);
    //printf("buf: %s", buf);
    memcpy(tmp, buf, *len);
    fclose(fp);


    FILE *fd = fopen(fn, "w+");
    //printf("len: %d", *len);

    fwrite(tmp, 1, *len, fd);
    fclose(fd);
    free(buf);
    free(tmp);
}

void MakeBinary(char** testcase){
    unsigned short *result = (unsigned short*)calloc(enc_vec.cnt+1, sizeof(wchar_t));
    char *text = (char*)calloc(2*(enc_vec.cnt+1), sizeof(char));
    if(!result) exit(0);
    if(!text) exit(0);

    for(int i=0; i<=enc_vec.cnt;i++){
        result[i] = (wchar_t)enc_vec.enc[i];
    }
    memcpy(*testcase, result, 2*(enc_vec.cnt+1));
    free(result);
    free(text);
}