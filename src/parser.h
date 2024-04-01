#ifndef PARSER_H
#define PARSER_H

#include <Arduino.h>

#define SPACE ' '
#define SIMPLE_COMMA '\''
#define DOUBLE_COMMA '\"'
#define BACKSLASH '\\'
#define NULLCHAR '\0'

typedef struct{
    char* argdata;
    int argc;
    char** argv;
} argx_type;

argx_type parseArgx(char* command, size_t commandSize, bool foolProgramName=false){
    char* argdata = (char*) malloc((commandSize + 1) * sizeof(char));
    memcpy(argdata, command, commandSize);
    argdata[commandSize] = NULLCHAR;
    
    int argc = (foolProgramName) ? 2 : 1;
    bool insideSimpleComma = false;
    bool insideDoubleComma = false;
    for(size_t index = 0; index < commandSize; index++){
        if(argdata[index] == SPACE && !insideSimpleComma && !insideDoubleComma && argdata[index+1] != NULLCHAR){
            argc++;
        }
        else if(argdata[index] == SIMPLE_COMMA && argdata[index-1] != BACKSLASH){
            insideSimpleComma = !insideSimpleComma;
        }
        else if(argdata[index] == DOUBLE_COMMA && argdata[index-1] != BACKSLASH){
            insideDoubleComma = !insideDoubleComma;
        }
    }
    
    char** argv = (char**) malloc(argc * sizeof(char*));
    
    insideSimpleComma = false;
    insideDoubleComma = false;
    if(foolProgramName){
        char falseProgramName = NULLCHAR;
        argv[0] = &falseProgramName;
        argv[1] = &argdata[0];
    }
    else{
        argv[0] = &argdata[0];
    }
    int argcIndex = (foolProgramName) ? 2 : 1;
    for(size_t index = 0; index < commandSize; index++){
        if(argdata[index] == SPACE && !insideSimpleComma && !insideDoubleComma && argdata[index+1] != NULLCHAR){
            argdata[index] = NULLCHAR;
            argv[argcIndex] = &argdata[index+1];
            argcIndex++;
        }
        else if(argdata[index] == SIMPLE_COMMA && argdata[index-1] != BACKSLASH){
            insideSimpleComma = !insideSimpleComma;
        }
        else if(argdata[index] == DOUBLE_COMMA && argdata[index-1] != BACKSLASH){
            insideDoubleComma = !insideDoubleComma;
        }
    }
    argx_type output;
    output.argdata = argdata;
    output.argc = argc;
    output.argv = argv;

    return output;
}

#endif