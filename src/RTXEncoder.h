///
/// @file        RTXEncoder.h
/// @brief       AHABus Packet Radio - frame & packet encoding routines
/// @author      Cesar Parent
/// @copyright   2017 Cesar Parent
///
#pragma once
#include "RTXCommon.h"

typedef struct {
    uint16_t    sequenceNumber;
    void*       readData;
    RTXRead     readCallback;
    void*       writeData;
    RTXWrite    writeCallback;
} RTXEncoder;


// Encodes the packet defined by [header], using [encoder]. [header->lenght]
// bytes will be read using [encoder]'s read callback. The frame data will be
// written to a user buffer using [encoder]'s write callback.
//
// Returns the number of frames generated, or -1 if an error occurred.
int16_t rtxEncodePacket(RTXEncoder* encoder, RTXPacketHeader* header);
