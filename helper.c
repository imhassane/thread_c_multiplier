#include "helper.h"

int get_int_from_string(char * c, int start, int end){
    int i = start, s = 0;
    for(; i < end; s = s*10+(c[i]-'0'), i++);
    return s;
}