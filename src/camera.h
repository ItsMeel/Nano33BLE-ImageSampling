#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>
#include <Arduino_OV767X.h>

#define CAMERA_VSYNC 8
#define CAMERA_HREF  A1
#define CAMERA_PCLK  A0
#define CAMERA_XCLK  9
#define CAMERA_D0    1
#define CAMERA_D1    10
#define CAMERA_D2    0
#define CAMERA_D3    2
#define CAMERA_D4    3
#define CAMERA_D5    5
#define CAMERA_D6    6
#define CAMERA_D7    4

const int CAMERA_DPINS[8] = {CAMERA_D0, CAMERA_D1, CAMERA_D2, CAMERA_D3, CAMERA_D4, CAMERA_D5, CAMERA_D6, CAMERA_D7};

uint8_t cameraResolution = QVGA;
uint8_t cameraFormat = RGB565;
uint8_t cameraModel = OV7675;
const uint8_t cameraFPS = 1;

byte* frameBuffer;
size_t frameBufferSize;

int setupCamera(uint8_t maxTries){
    uint8_t tries = 1;
    while(true){
        Serial.println("Setting up camera.");
        Camera.setPins(CAMERA_VSYNC, CAMERA_HREF, CAMERA_PCLK, CAMERA_XCLK, CAMERA_DPINS);
        if(Camera.begin(cameraResolution, cameraFormat, cameraFPS, cameraModel)){ // Camera setup correctly
            Serial.println("Cammera settings apllied correctly.");
            return 0;
        }
        else if(maxTries != 0 && maxTries > tries){ // maxTries is setup and there is tries left
            Serial.println("Failed to setup camera, trying again.");
            tries++;
        }
        else if(maxTries != 0){ // maxTries is setup and there isn't tries left
            Serial.println("Failed to setup camera.");
            return 1;
        }
        delay(500);
    }
}

int configureResolution(uint8_t resolution, uint8_t maxTries){
    if(resolution > 4){
        return 1;
    }
    cameraResolution = resolution;
    Serial.print("Configuring camera resolution to: ");
    switch (cameraResolution){
        case VGA:
            Serial.println("VGA (640x480).");
        break;

        case CIF:
            Serial.println("CIF (352x240).");
        break;

        case QVGA:
            Serial.println("QVGA (320x240).");
        break;

        case QCIF:
            Serial.println("QCIF (176x144).");
        break;

        case QQVGA:
            Serial.println("QQVGA (160x120).");
        break;
    
        default:
            Serial.println("Invalid value.");
        break;
    }
    return setupCamera(maxTries);
}

int configureFormat(uint8_t format, uint8_t maxTries){
    if(format != 4 || format > 2){
        return 1;
    }
    cameraFormat = format;
    Serial.print("Configuring camera format to: ");
    switch (cameraFormat){
        case YUV422:
            Serial.println("YUV422 (1 byte per pixel).");
        break;

        case RGB444:
            Serial.println("RGB444 (1 byte per pixel).");
        break;

        case RGB565:
            Serial.println("RGB565 (2 bytes per pixel).");
        break;

        case GRAYSCALE:
            Serial.println("GRAYSCALE (1 byte per pixel).");
        break;
    
        default:
            Serial.println("Invalid value.");
        break;
    }
    return setupCamera(maxTries);
}

void printCameraSettings(){
    Serial.print("Current camera settings:\n");

    Serial.print("\tModel: ");
    switch (cameraModel){
        case OV7670:
            Serial.print("OV7670.\n");
        break;

        case OV7675:
            Serial.print("OV7675.\n");
        break;

        default:
            Serial.print("Invalid value.\n");
        break;
    }

    Serial.print("\tVSYNC_PIN: ");
    Serial.print(CAMERA_VSYNC);
    Serial.print("\n\tHREF_PIN: ");
    Serial.print(CAMERA_HREF);
    Serial.print("\n\tPCLK_PIN: ");
    Serial.print(CAMERA_PCLK);
    Serial.print("\n\tXCLK_PIN: ");
    Serial.print(CAMERA_XCLK);
    Serial.print("\n\tD0_PIN: ");
    Serial.print(CAMERA_D0);
    Serial.print("\n\tD1_PIN: ");
    Serial.print(CAMERA_D1);
    Serial.print("\n\tD2_PIN: ");
    Serial.print(CAMERA_D2);
    Serial.print("\n\tD3_PIN: ");
    Serial.print(CAMERA_D3);
    Serial.print("\n\tD4_PIN: ");
    Serial.print(CAMERA_D4);
    Serial.print("\n\tD5_PIN: ");
    Serial.print(CAMERA_D5);
    Serial.print("\n\tD6_PIN: ");
    Serial.print(CAMERA_D6);
    Serial.print("\n\tD7_PIN: ");
    Serial.print(CAMERA_D7);
    
    Serial.print("\n\tResolution: ");
    switch (cameraResolution){
        case VGA:
            Serial.print("VGA (640x480).\n");
        break;

        case CIF:
            Serial.print("CIF (352x240).\n");
        break;

        case QVGA:
            Serial.print("QVGA (320x240).\n");
        break;

        case QCIF:
            Serial.print("QCIF (176x144).\n");
        break;

        case QQVGA:
            Serial.print("QQVGA (160x120).\n");
        break;
    
        default:
            Serial.print("Invalid value.\n");
        break;
    }

    Serial.print("\tFormat: ");
    switch (cameraFormat){
        case YUV422:
            Serial.print("YUV422 (1 byte per pixel).\n");
        break;

        case RGB444:
            Serial.print("RGB444 (1 byte per pixel).\n");
        break;

        case RGB565:
            Serial.print("RGB565 (2 bytes per pixel).\n");
        break;

        case GRAYSCALE:
            Serial.print("GRAYSCALE (1 byte per pixel).\n");
        break;
    
        default:
            Serial.print("Invalid value.\n");
        break;
    }

    Serial.print("\tFPS: ");
    Serial.print(cameraFPS);
    Serial.println(".");
}

void freeFrameBuffer(byte** frameBuffer){
    if(*frameBuffer != NULL){
        free(*frameBuffer);
        *frameBuffer = NULL;
    }
}

int setupFrameBuffer(byte** frameBuffer, size_t* frameBufferSize){
    freeFrameBuffer(frameBuffer);
    
    *frameBufferSize = Camera.width() * Camera.height() * Camera.bytesPerPixel();

    *frameBuffer = (byte *) malloc(*frameBufferSize * sizeof(byte));

    if(*frameBuffer == NULL){
        Serial.print("No enough memory for frame buffer, requested: ");
        Serial.print(*frameBufferSize * sizeof(byte));
        Serial.println(" bytes. Try to downgrade the camera resolution and format.");
        *frameBufferSize = 0;
        return 1;
    }

    return 0;
}

int takePhoto(byte** frameBuffer, size_t* frameBufferSize){
    if(setupFrameBuffer(frameBuffer, frameBufferSize)){
        Serial.println("Failed to setup frame buffer.");
        return 1;
    }

    Camera.readFrame(*frameBuffer);

    if(*frameBuffer == NULL){
        *frameBufferSize = 0;
        Serial.println("Failed to read frame.");
        return 1;
    }
    return 0;
}

#endif