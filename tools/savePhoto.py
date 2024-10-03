import serial
import argparse
import re
import cv2 as cv
import numpy as np
from PIL import Image

defaultTimeout = 3
requestPhotoTimeoutMultiply = 3
defaultDTR = False
defaultRTS = False
defaultMaxSize = 512
defaultStopBytes = ''
cameraResolutionHeight = 0
cameraResolutionWidth = 0
cameraFormat = None

def communicate_device(port, baudrate, message, timeout=defaultTimeout, dtr=defaultDTR, rts=defaultRTS, encodeInput=None, encodeOutput=None, printSent=True, printReceived=True, maxSize=defaultMaxSize, stopBytes=defaultStopBytes):
    try:
        with serial.Serial(port, baudrate, timeout=timeout, dsrdtr=dtr, rtscts=rts) as serialDevice:
            if encodeInput:
                messageSent = message.encode('utf-8')
            else:
                messageSent = message

            serialDevice.write(messageSent)

            if printSent:
                print(f"Bytes sent {len(messageSent)}:\n {messageSent}")
            

            messageReceived = serialDevice.read_until(stopBytes, size=maxSize)
            if encodeOutput:
                messageReceived = messageReceived.decode('utf-8')

            if printReceived:
                print(f"Bytes received {len(messageReceived)}:\n {messageReceived}")
            else:
                print(f"Bytes received {len(messageReceived)}")

            return messageReceived
    except Exception as e:
        print(f"Error: {e}")

def getCameraConfig(port, baudrate, timeout=defaultTimeout, dtr=defaultDTR, rts=defaultRTS, maxSize=defaultMaxSize, stopBytes=defaultStopBytes):
    try:
        response = communicate_device(
            port=port,
            baudrate=baudrate,
            message="getCameraSettings",
            timeout=timeout,
            dtr=dtr,
            rts=rts,
            encodeInput=True,
            encodeOutput=True,
            printSent=True,
            printReceived=False,
            maxSize=maxSize,
            stopBytes=stopBytes
        )

        # Extract resolution (height and width)
        resolution_match = re.search(r'Resolution:\s*\w+\s*\((\d+)x(\d+)\)', response)
        if resolution_match:
            global cameraResolutionWidth 
            cameraResolutionWidth = int(resolution_match.group(1))

            global cameraResolutionHeight 
            cameraResolutionHeight = int(resolution_match.group(2))
        else:
            raise Exception("Failed to extract resolution")

        # Extract format
        format_match = re.search(r'Format:\s*(\w+)', response)
        if format_match:
            cameraFormat = format_match.group(1)
        else:
            raise Exception("Failed to extract format")

        print(f"Camera Resolution: {cameraResolutionWidth}x{cameraResolutionHeight}; Format: {cameraFormat}")

    except Exception as e:
        print(f"Error: {e}")

def requestPhoto(port, baudrate, timeout=defaultTimeout, dtr=defaultDTR, rts=defaultRTS, maxSize=defaultMaxSize, stopBytes=defaultStopBytes):
    try:
        getCameraConfig(
            port=port,
            baudrate=baudrate,
            timeout=timeout,
            dtr=dtr,
            rts=rts,
            maxSize=maxSize,
            stopBytes=stopBytes
        )
        
        response = communicate_device(
            port=port,
            baudrate=baudrate,
            message="takePhoto",
            timeout=requestPhotoTimeoutMultiply * timeout,
            dtr=dtr,
            rts=rts,
            encodeInput=True,
            encodeOutput=False,
            printSent=True,
            printReceived=False,
            maxSize=2*cameraResolutionHeight*cameraResolutionWidth + 2,
            stopBytes=stopBytes
        )

        return response
    except Exception as e:
        print(f"Error: {e}")

def processPhoto(rawBytes, photoWidth, photoHeight, bitShuffle=True):
    rawData = np.zeros(len(rawBytes), dtype=np.uint8)
    rawImage = np.zeros((photoHeight, photoWidth, 3), dtype=np.uint8)

    for byteIndex in range(0, len(rawBytes)):
        rawData[byteIndex] = rawBytes[byteIndex]

    for rowIndex in range(0, photoHeight):
        for colIndex in range(0, photoWidth):
            pixelLowIndex = (rowIndex * photoWidth + colIndex) * 2 + 1
            pixelHighIndex = (rowIndex * photoWidth + colIndex) * 2

            if bitShuffle:
                rawData[pixelLowIndex] = (rawData[pixelLowIndex] & 0xFC) | ((rawData[pixelLowIndex] & 0x01) << 1) | ((rawData[pixelLowIndex] & 0x02) >> 1)
                rawData[pixelHighIndex] = (rawData[pixelHighIndex] & 0xFC) | ((rawData[pixelHighIndex] & 0x01) << 1) | ((rawData[pixelHighIndex] & 0x02) >> 1)

            # BGR565
            # FEDCBA98 76543210
            # RRRRRGGG GGGBBBBB

            #blue
            rawImage[rowIndex, colIndex, 0] = ((rawData[pixelLowIndex] >> 0) & 0x1F) << 3
            #green
            rawImage[rowIndex, colIndex, 1] = (((rawData[pixelHighIndex] >> 0) & 0x07) << 5) | (((rawData[pixelLowIndex] >> 5) & 0x07) << 2) 
            #red
            rawImage[rowIndex, colIndex, 2] = ((rawData[pixelHighIndex] >> 3) & 0x1F) << 3
    
    return rawImage

def savePhoto(rawImage, outputPath, displayImage=False):
    cv.imwrite(outputPath, rawImage)
    if displayImage:
        image = Image.open(outputPath)
        image.show()

def main():
    parser = argparse.ArgumentParser(description="Communicate with a serial device")
    parser.add_argument("command", choices=["sendCommand", "getCameraConfig", "requestPhoto"], help="Command to execute")
    parser.add_argument("--port", "-p", type=str, help="Serial port")
    parser.add_argument("--baudrate", "-b", type=int, help="Baud rate")
    parser.add_argument("--message", "-m", type=str, help="Message to send")
    parser.add_argument("--timeout", "-t", type=int, help="Timeout in seconds", default=defaultTimeout)
    parser.add_argument("--dtr", "-d", action="store_true", help="Set DTR", default=defaultDTR)
    parser.add_argument("--rts", "-r", action="store_true", help="Set RTS", default=defaultRTS)
    parser.add_argument("--maxSize", "-s", type=int, help="Max receive size in bytes", default=defaultMaxSize)
    parser.add_argument("--stopBytes", "-sb", type=str, help="Stop bytes", default=defaultStopBytes)

    args = parser.parse_args()

    if args.command == "sendCommand":
        communicate_device(
            port=args.port,
            baudrate=args.baudrate,
            message=args.message,
            timeout=args.timeout,
            dtr=args.dtr,
            rts=args.rts,
            maxSize=args.maxSize,
            stopBytes=args.stopBytes,
            encodeInput=True,
            encodeOutput=True,
            printSent=True,
            printReceived=True
        )

    elif args.command == "getCameraConfig":
        getCameraConfig(
            port=args.port,
            baudrate=args.baudrate,
            timeout=args.timeout,
            dtr=args.dtr,
            rts=args.rts,
            maxSize=args.maxSize,
            stopBytes=args.stopBytes
        )

    elif args.command == "requestPhoto":
        rawImage = requestPhoto(
            port=args.port,
            baudrate=args.baudrate,
            timeout=args.timeout,
            dtr=args.dtr,
            rts=args.rts,
            maxSize=args.maxSize,
            stopBytes=args.stopBytes
        )

        #print(f"Raw image dimensions: {cameraResolutionWidth}x{cameraResolutionHeight}")
        processedImage = processPhoto(rawImage, cameraResolutionWidth, cameraResolutionHeight)
        savePhoto(processedImage, "test.jpg", True)

if __name__ == "__main__":
    main()
