#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>
#include <camera.h>
#include <argtable3.h>

#define COMMANDS 5
#define CAMERA_CONFIGURATION_MAXTRIES 3

#define REG_EXTENDED 1
#define REG_ICASE (REG_EXTENDED << 1)
void arg_print_syntax_custom(HardwareSerial* printable, void** argtable, const char* suffix) {
    arg_dstr_t ds = arg_dstr_create();
    arg_print_syntax_ds(ds, argtable, suffix);
    printable->print(arg_dstr_cstr(ds));
    printable->print(suffix);
    arg_dstr_destroy(ds);
}

void arg_print_glossary_custom(HardwareSerial* printable, void** argtable, const char* format) {
    arg_dstr_t ds = arg_dstr_create();
    arg_print_glossary_ds(ds, argtable, format);
    printable->print(arg_dstr_cstr(ds));
    arg_dstr_destroy(ds);
}

typedef struct{
    void** argtable;
    int parseErrors;
    const char* helpMsg;
    void (*function)();
} command_struct;

command_struct** commandList;

struct {
    struct arg_rex* arg_cmd;
    struct arg_end* arg_end;
} help_argtable;
command_struct help_command_struct; // help_command symbol is already used by .platformio\packages\framework-arduino-mbed\variants\ARDUINO_NANO33BLE\libs\libmbed.a

void help_function(){
    Serial.println("List of commands:");
    for(size_t commandIndex = 0; commandIndex < COMMANDS; commandIndex++){
        Serial.print("\t");
        arg_print_syntax_custom(&Serial, commandList[commandIndex]->argtable, "\t\t");
        Serial.println(commandList[commandIndex]->helpMsg);
    }
}

struct {
    struct arg_rex* arg_cmd;
    struct arg_int* arg_int;
    struct arg_lit* arg_help;
    struct arg_end* arg_end;
} setResolution_argtable;
command_struct setResolution_command;

void setResolution_function(){
    if(setResolution_argtable.arg_help->count == 1){
        Serial.println("Usage: ");
        arg_print_syntax_custom(&Serial, setResolution_command.argtable, "\n");
        arg_print_glossary_custom(&Serial, setResolution_command.argtable,"      %-20s %s\n");
        Serial.println("\nDetails: ");
        Serial.println(setResolution_command.helpMsg);
        Serial.println("<resolution> is a unsigned integer with possible values:");
        Serial.println("\t0 -> VGA (640x480).");
        Serial.println("\t1 -> CIF (352x240).");
        Serial.println("\t2 -> QVGA (320x240).");
        Serial.println("\t3 -> QCIF (176x144).");
        Serial.println("\t4 -> QQVGA (160x120).");
        return;
    }

    if(setResolution_argtable.arg_int->count == 0){
        Serial.println("Missing <resolution> value, use \"setResolution --help\" for more details.");
        return;
    }

    uint8_t selectedResolution = setResolution_argtable.arg_int->ival[0];
    if(selectedResolution > 4){
        Serial.println("Invalid <resolution> value, use \"setResolution --help\" for more details.");
        return;
    }

    if(configureResolution(selectedResolution, CAMERA_CONFIGURATION_MAXTRIES) != 0){
        Serial.println("Unexpected error, check syntax with the flag \"--help\" and try again.");
        return;
    }

    Serial.println("Camera resolution applied correctly.");
}

struct {
    struct arg_rex* arg_cmd;
    struct arg_int* arg_int;
    struct arg_lit* arg_help;
    struct arg_end* arg_end;
} setFormat_argtable;
command_struct setFormat_command;

void setFormat_function(){
    if(setFormat_argtable.arg_help->count == 1){
        Serial.println("Usage: ");
        arg_print_syntax_custom(&Serial, setFormat_command.argtable, "\n");
        arg_print_glossary_custom(&Serial, setFormat_command.argtable,"      %-20s %s\n");
        Serial.println("\nDetails: ");
        Serial.println(setFormat_command.helpMsg);
        Serial.println("<format> is a unsigned integer with possible values:");
        Serial.println("\t0 -> YUV422 (1 byte per pixel).");
        Serial.println("\t1 -> RGB444 (1 byte per pixel).");
        Serial.println("\t2 -> RGB565 (2 bytes per pixel).");
        Serial.println("\t3 -> GRAYSCALE (2 bytes per pixel).");
        return;
    }

    if(setFormat_argtable.arg_int->count == 0){
        Serial.println("Missing <format> value, use \"setFormat --help\" for more details.");
        return;
    }

    uint8_t selectedFormat = setFormat_argtable.arg_int->ival[0];
    if(selectedFormat > 3){
        Serial.println("Invalid <format> value, use \"setFormat --help\" for more details.");
        return;
    }

    selectedFormat = (selectedFormat == 3) ? 4 : selectedFormat; // GRAYSCALE enum is value 4.

    if(configureFormat(selectedFormat, CAMERA_CONFIGURATION_MAXTRIES) != 0){
        Serial.println("Unexpected error, check syntax with the flag \"--help\" and try again.");
        return;
    }
    
    Serial.println("Camera format applied correctly.");
}

struct {
    struct arg_rex* arg_cmd;
    struct arg_lit* arg_help;
    struct arg_end* arg_end;
} getCameraSettings_argtable;
command_struct getCameraSettings_command;

void getCameraSettings_function(){
    if(getCameraSettings_argtable.arg_help->count == 1){
        Serial.println("Usage: ");
        arg_print_syntax_custom(&Serial, getCameraSettings_command.argtable, "\n");
        arg_print_glossary_custom(&Serial, getCameraSettings_command.argtable,"      %-20s %s\n");
        Serial.println("\nDetails: ");
        Serial.println(getCameraSettings_command.helpMsg);
        return;
    }

    printCameraSettings();
}

struct {
    struct arg_rex* arg_cmd;
    struct arg_lit* arg_help;
    struct arg_end* arg_end;
} takePhoto_argtable;
command_struct takePhoto_command;

void takePhoto_function(){
    if(takePhoto_argtable.arg_help->count == 1){
        Serial.println("Usage: ");
        arg_print_syntax_custom(&Serial, takePhoto_command.argtable, "\n");
        arg_print_glossary_custom(&Serial, takePhoto_command.argtable,"      %-20s %s\n");
        Serial.println("\nDetails: ");
        Serial.println(takePhoto_command.helpMsg);
        return;
    }


    if(takePhoto(&frameBuffer, &frameBufferSize)){
        Serial.println("Failed to take photo.");
        return;
    }

    Serial.write(frameBuffer, frameBufferSize);
    Serial.print("\r\n");
}

int setupCommands(){
    commandList = (command_struct**) malloc(COMMANDS * sizeof(command_struct*));

    help_argtable.arg_cmd = arg_rex1(NULL, NULL, "help", NULL, REG_ICASE, NULL);
    help_argtable.arg_end = arg_end(1);
    help_command_struct.argtable = (void**) &help_argtable;
    help_command_struct.helpMsg = "Shows a list of commands.";
    help_command_struct.function = &help_function;
    commandList[0] = &help_command_struct;

    setResolution_argtable.arg_cmd = arg_rex1(NULL, NULL, "setResolution", NULL, REG_ICASE, NULL);
    setResolution_argtable.arg_int = arg_int0(NULL, NULL, "<resolution>", "Camera resolution");
    setResolution_argtable.arg_help = arg_lit0(NULL, "help", "Show help");
    setResolution_argtable.arg_end = arg_end(3);
    setResolution_command.argtable = (void**) &setResolution_argtable;
    setResolution_command.helpMsg = "Sets the camera resolution.";
    setResolution_command.function = &setResolution_function;
    commandList[1] = &setResolution_command;
    
    setFormat_argtable.arg_cmd = arg_rex1(NULL, NULL, "setFormat", NULL, REG_ICASE, NULL);
    setFormat_argtable.arg_int = arg_int0(NULL, NULL, "<format>", "Camera format");
    setFormat_argtable.arg_help = arg_lit0(NULL, "help", "Show help");
    setFormat_argtable.arg_end = arg_end(3);
    setFormat_command.argtable = (void**) &setFormat_argtable;
    setFormat_command.helpMsg = "Sets the camera format.";
    setFormat_command.function = &setFormat_function;
    commandList[2] = &setFormat_command;

    getCameraSettings_argtable.arg_cmd = arg_rex1(NULL, NULL, "getCameraSettings", NULL, REG_ICASE, NULL);
    getCameraSettings_argtable.arg_help = arg_lit0(NULL, "help", "Show help");
    getCameraSettings_argtable.arg_end = arg_end(2);
    getCameraSettings_command.argtable = (void**) &getCameraSettings_argtable;
    getCameraSettings_command.helpMsg = "Shows the camera current settings.";
    getCameraSettings_command.function = &getCameraSettings_function;
    commandList[3] = &getCameraSettings_command;

    takePhoto_argtable.arg_cmd = arg_rex1(NULL, NULL, "takePhoto", NULL, REG_ICASE, NULL);
    takePhoto_argtable.arg_help = arg_lit0(NULL, "help", "Show help");
    takePhoto_argtable.arg_end = arg_end(2);
    takePhoto_command.argtable = (void**) &takePhoto_argtable;
    takePhoto_command.helpMsg = "Take a photo and send it.";
    takePhoto_command.function = &takePhoto_function;
    commandList[4] = &takePhoto_command;

    bool failedCommand = false;
    for(size_t commandIndex = 0; commandIndex < COMMANDS; commandIndex++){
        while(true){
            Serial.print("Command ");
            Serial.print(commandIndex + 1);
            Serial.print("/");
            Serial.print(COMMANDS);
            if(arg_nullcheck(commandList[commandIndex]->argtable) == 0){
                Serial.println(" registered succesfully.");
                break;
            }
            else{
                Serial.println(" failed register.");
            }
            delay(500);
        }
    }

    return (failedCommand) ? 1 : 0;
}

void executeCommands(int argc, char** argv){
    for(size_t commandIndex = 0; commandIndex < COMMANDS; commandIndex++){
        commandList[commandIndex]->parseErrors = arg_parse(argc, argv, commandList[commandIndex]->argtable);
        if(commandList[commandIndex]->parseErrors == 0){
            commandList[commandIndex]->function();
            return;
        }
    }

    Serial.println("Invalid command, use the command \"help\" to get a list of commands.");
}
#endif