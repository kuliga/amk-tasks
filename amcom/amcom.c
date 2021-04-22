#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "amcom.h"

/// Start of packet character
const uint8_t  AMCOM_SOP         = 0xA1;
const uint16_t AMCOM_INITIAL_CRC = 0xFFFF;

static uint16_t AMCOM_UpdateCRC(uint8_t byte, uint16_t crc)
{
	byte ^= (uint8_t)(crc & 0x00ff);
	byte ^= (uint8_t)(byte << 4);
	return ((((uint16_t)byte << 8) | 
		(uint8_t)(crc >> 8)) ^ 
		(uint8_t)(byte >> 4) ^ ((uint16_t)byte << 3));
}


void AMCOM_InitReceiver(AMCOM_Receiver* receiver, 
		AMCOM_PacketHandler packetHandlerCallback, 
				void* userContext) 
{
	if (receiver == NULL)
		return;
	
	*receiver = (AMCOM_Receiver) {
		.receivedPacketState = AMCOM_PACKET_STATE_EMPTY,
		.packetHandler = packetHandlerCallback,
		.userContext = userContext
	};
}

/*
 * No memcpy() usage, because we will read info from SRAM memory only (I mean access time).
 */
size_t AMCOM_Serialize(uint8_t packetType, const void* payload,
	 size_t payloadSize, uint8_t* destinationBuffer) 	
{	
	AMCOM_Packet *pobj;
	AMCOM_PacketHeader *hobj;

	hobj = (AMCOM_PacketHeader*) destinationBuffer;
	*hobj = (AMCOM_PacketHeader) {
		.sop = AMCOM_SOP,
		.type = packetType,
		.length = payloadSize
	};
	hobj->crc = AMCOM_UpdateCRC(hobj->type, AMCOM_INITIAL_CRC);
	hobj->crc = AMCOM_UpdateCRC(hobj->length, hobj->crc);

	if (hobj->length == 0) 
		return sizeof(*hobj);
	
	pobj = (AMCOM_Packet*) hobj;	
	for (size_t i = 0; i < pobj->header.length; i++) {
		pobj->payload[i] = *(uint8_t*) (payload + i);
		pobj->header.crc = AMCOM_UpdateCRC(pobj->payload[i], 
						pobj->header.crc);
	}

	return pobj->header.length + sizeof(*hobj);
}	

void state_update(AMCOM_Receiver *recv, AMCOM_PacketState state)
{
	recv->receivedPacketState = state;
}

void recv_clear(AMCOM_Receiver *recv) 
{
	memset(&recv->receivedPacket, 0, sizeof(AMCOM_Packet));
	recv->payloadCounter = 0;
}

void AMCOM_Deserialize(AMCOM_Receiver* receiver, const void* data, 
						size_t dataSize) 
{
	static uint8_t shift = 0;
	static uint16_t curr_crc = 0;

	for (size_t i = 0; i < dataSize; i++) {
		uint8_t byte = *(uint8_t*) (data + i);
		switch (receiver->receivedPacketState) {
		case AMCOM_PACKET_STATE_EMPTY:
			if (byte == AMCOM_SOP) {
				receiver->receivedPacket.header.sop = byte;
				state_update(receiver, AMCOM_PACKET_STATE_GOT_SOP);
			}
			break;
		case AMCOM_PACKET_STATE_GOT_SOP:
			receiver->receivedPacket.header.type = byte;
			curr_crc = AMCOM_UpdateCRC(receiver->receivedPacket.header.type, 
								AMCOM_INITIAL_CRC);
			state_update(receiver, AMCOM_PACKET_STATE_GOT_TYPE);
			break;
		case AMCOM_PACKET_STATE_GOT_TYPE:
			if (byte < 201) {
				receiver->receivedPacket.header.length = byte;
				curr_crc = AMCOM_UpdateCRC(receiver->receivedPacket.header.length,
											curr_crc);
				state_update(receiver, AMCOM_PACKET_STATE_GOT_LENGTH);
			} else {
				state_update(receiver, AMCOM_PACKET_STATE_EMPTY);
			}
			break;
		case AMCOM_PACKET_STATE_GOT_LENGTH:
			receiver->receivedPacket.header.crc  |= (byte << shift++ * 8);
			if (shift == 2) 	
				state_update(receiver, AMCOM_PACKET_STATE_GOT_CRC);
			else
				break;
		case AMCOM_PACKET_STATE_GOT_CRC:
			shift = 0;
			if (receiver->receivedPacket.header.length == 0) {
				state_update(receiver, AMCOM_PACKET_STATE_GOT_WHOLE_PACKET);
				goto packet_ready;
			} else { 
				state_update(receiver, AMCOM_PACKET_STATE_GETTING_PAYLOAD);
				break; 
			}
		case AMCOM_PACKET_STATE_GETTING_PAYLOAD:
			receiver->receivedPacket.payload[receiver->payloadCounter] = byte;
			curr_crc = AMCOM_UpdateCRC(receiver->receivedPacket
					.payload[receiver->payloadCounter++], curr_crc);
			if (receiver->payloadCounter == receiver->receivedPacket.header.length) {
				state_update(receiver, AMCOM_PACKET_STATE_GOT_WHOLE_PACKET);
				goto packet_ready;
			} else {
				break;
			}
	packet_ready:	
		case AMCOM_PACKET_STATE_GOT_WHOLE_PACKET:
			if (receiver->receivedPacket.header.crc == curr_crc)
				receiver->packetHandler(&(receiver->receivedPacket), 
							receiver->userContext);
			state_update(receiver, AMCOM_PACKET_STATE_EMPTY);
			recv_clear(receiver);
			break;
		default:
			break;
		}
	}
}


