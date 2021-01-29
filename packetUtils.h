#ifndef PACKETUTILS_H
#define PACKETUTILS_H

#include "csapp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	MAX_CATALOG_FNAME	 256

/**
 * packet storage unit data
 */
typedef struct PacketData{
    char *data;
    size_t data_len;
} PacketData;

/**
 * packet storage unit
 */
typedef struct PacketNumVal{
    int num;
    PacketData pd;
} PacketNumVal;

/**
 * Allocate memory unit packet
 * @param num packet placeholder in the container
 * @param data_len the len of data that it will contains
 * @return a ref to the allocated memory
 */
PacketNumVal *PacketNumVal_new(int num, size_t *data_len);

/**
 * free the memory allocated for the unit packet
 * @param in ref to the unit packet
 */
void PacketNumVal_free(PacketNumVal *in);

/**
 * Check if packet matches given it placeholder
 * @param in ref to the unit packet
 * @param num the placeholder to compare with
 * @return 0 if not matches else 1
 */
unsigned char PacketNumVal_matches(PacketNumVal const *in, const int num);

/** used to check if packet query is empty */
extern void *PACKETS_CATALOG_NOT_FOUND;

/**
 * Packets container
 */
typedef struct PacketCatalog{
    char fname[MAX_CATALOG_FNAME];
    PacketNumVal **pairs;
    int length;
} PacketCatalog;

/**
 * Allocate memory for a packet container
 * @param fileName filename that will be used to convert the container into image 
 * @return a ref to the allocated memory
 */
PacketCatalog *PacketCatalog_new (const char *fileName);

/**
 * free the memory allocated for the packet container
 * @param in ref to the packet container
 */
void PacketCatalog_free(PacketCatalog *in);

/**
 * Add a packet in the catalog
 * @param in a ref to the catalog
 * @param num the placeholder
 * @param data_len the lenght of the data that the packet will store
 */
void PacketCatalog_add(PacketCatalog *in, int num, size_t *data_len);

/**
 * Retreive a packet data given it placeholder
 * @param in a ref to the catalog
 * @param num the placeholder to match
 * @return a ref to the packet data
 */
void *PacketCatalog_findPacketData(PacketCatalog const *in, const int num);

/**
 * Xor all the packets in the container
 * @param in a ref to the catalog
 * @param xorVal the value used for the xor operation
 */
void PacketCatalog_xorData(PacketCatalog const *in, const unsigned char xorVal);

/**
 * Convert the container into an image
 * @param in a ref to the catalog
 */
void PacketCatalog_convertToImage(PacketCatalog const *in);

#ifdef __cplusplus
}
#endif

#endif /* PACKETUTILS_H */

