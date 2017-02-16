///
/// @file        RTXDecoder.c
/// @brief       AHABus Packet Radio - frame & packet encoding routines
/// @author      Cesar Parent
/// @copyright   2017 Cesar Parent
///
#include <stdio.h>
#include "RTXDecoder.h"

static void _clearFrame(uint8_t frame[FRAME_SIZE]) {
    for(uint16_t i = 0; i < FRAME_SIZE; ++i) {
        frame[i] = 0x00;
    }
}


static bool _wasteUntilSync(RTXCoder* decoder) {
    int state = 0; // simple state machine. 0 -> waiting, 1 -> 0xaa, 2 -> 0x5a
    uint8_t current = 0x00;
    
    do {
        if(!decoder->readCallback(&current, decoder->readData)) { return false; }
        switch(state) {
            case 0:
                state = current == 0xAA ? 1 : 0;
                break;
            case 1:
                if(current == 0x5a) {
                    return true;
                }
                state = 0;
                break;
            default:
                break;
        }
        
    } while(true);
    return true;
}

static bool _readFrame(RTXCoder* decoder, uint8_t frame[FRAME_SIZE]) {
    _clearFrame(frame);
    if(!_wasteUntilSync(decoder)) { return false; }
    frame[0] = 0xAA;
    frame[1] = 0x5A;
    for(uint16_t i = 2; i < FRAME_SIZE; ++i) {
        if(!decoder->readCallback(&frame[i], decoder->readData)) { return false; }
    }
    return true;
}

static bool _validateFrame(RTXCoder* decoder, uint8_t frame[FRAME_SIZE]) {
    // This is mostly going to be FEC cehcking and correction
    uint8_t idx = 0;
    
    if(frame[idx++] != 0xAA) { return false; }
    if(frame[idx++] != 0x5A) { return false; }
    if(frame[idx++] != PROTOCOL_VERSION) { return false; }
    
    // Check sequence numbers? reigster dropped packets? discuss.
    
    return true;
}

static uint8_t _read16(uint16_t* data, uint8_t* frame) {
    *data = (frame[0] << 8) | (frame[1]);
    return 2;
}

static uint8_t _read32(int32_t* data, uint8_t* frame) {
    *data = 0;
    for(int8_t i = 3; i >= 0; --i) {
        *data |= (*(frame++)) << (i*8);
    }
    return 4;
}

static int32_t _extractHeaderFrame(RTXCoder* decoder,
                                    RTXPacketHeader* header,
                                    uint8_t frame[FRAME_SIZE]) {
    uint8_t idx = FRAME_HEADERSIZE;
    
    if(frame[idx++] != PROTOCOL_VERSION) { return -1; }
    
    header->payloadID = frame[idx++];
    idx += _read16(&header->length, &frame[idx]);
    idx += _read32(&header->latitude, &frame[idx]);
    idx += _read32(&header->longitude, &frame[idx]);
    idx += _read16(&header->altitude, &frame[idx]);
    
    header->length -= PACKET_HEADERSIZE;
    
    uint16_t toRead = header->length;
    
    for(; idx < FRAME_DATASIZE && toRead > 0; ++idx) {
        if(!decoder->writeCallback(frame[idx], decoder->writeData)) { return -1;}
        toRead -= 1;
    }
    return toRead;
}

static int32_t _extractDataFrame(RTXCoder* decoder,
                                  int32_t toRead,
                                  uint8_t frame[FRAME_SIZE]) {
    
    for(uint8_t idx = FRAME_HEADERSIZE; idx < FRAME_DATASIZE && toRead > 0; ++idx) {
        if(!decoder->writeCallback(frame[idx], decoder->writeData)) { return -1;}
        toRead -= 1;
    }
    return toRead;
}

void rtxDecodeFrameStream(RTXCoder* decoder, RTXPacketCallback callback) {
    
    uint8_t frame[FRAME_SIZE];
    RTXPacketHeader header;
    int32_t toRead = 0;
    bool valid = true;
    int state = 0;  // state:   0: expecting packet header
                    //          1: packet data
    
    while(true) {
        if(!_readFrame(decoder, frame)) { return;}
        if(!_validateFrame(decoder, frame)) { return; }
        
        
        // TODO: error (dropped frame) recovery.
        switch(state) {
            case 0:
                toRead = _extractHeaderFrame(decoder, &header, frame);
                break;
            case 1:
                toRead = _extractDataFrame(decoder, toRead, frame);
                break;
            default:
                return;
                break;
        }
        
        if(toRead <= 0) {
            valid = toRead == 0;
            callback(&header, valid);
            state = 0;
        } else {
            state = 1;
        }
        state = toRead == 0 ? 0 : 1;
        
    }
}
