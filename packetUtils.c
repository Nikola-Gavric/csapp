

#include "packetUtils.h"

void *PACKETS_CATALOG_NOT_FOUND;

PacketNumVal *PacketNumVal_new(int num, size_t *data_len) {
    PacketNumVal *out = Malloc(sizeof(PacketNumVal));

    if (NULL != out) {
        out->num = num;
        out->pd.data_len = *data_len;
        out->pd.data = Malloc(*data_len);
        if (NULL == out->pd.data) {
            Free(out);
            out = NULL;
        }
    }
    return out;
}

void PacketNumVal_free(PacketNumVal *in) {
    if (NULL != in) {
        if (NULL != in->pd.data) {
            Free(in->pd.data);
        }

        Free(in);
    }
}


unsigned char PacketNumVal_matches(PacketNumVal const *in, const int num) {
    return (num == in->num);
}


PacketCatalog *PacketCatalog_new(const char *fileName) {
    static int pcnf;
    if (!PACKETS_CATALOG_NOT_FOUND) PACKETS_CATALOG_NOT_FOUND = &pcnf;
    PacketCatalog *out = Malloc(sizeof(PacketCatalog));

    if (NULL != out) {
        *out = (PacketCatalog) {};
        strncpy(out->fname, fileName, MAX_CATALOG_FNAME);
    }

    return out;
}

void PacketCatalog_free(PacketCatalog *in) {
    if (NULL != in) {
        for (int i = 0; i < in->length; i++)
            PacketNumVal_free(in->pairs[i]);

        Free(in);
    }
}

static void PacketCatalogAddPacketNumVal(PacketCatalog *in, PacketNumVal *kv) {

    in->length++;
    in->pairs = Realloc(in->pairs, sizeof(PacketNumVal *) * in->length);

    if (NULL != in->pairs) {
        in->pairs[in->length - 1] = kv;
    }
}

void PacketCatalog_add(PacketCatalog *in, int num, size_t *data_len) {
    PacketCatalogAddPacketNumVal(in, PacketNumVal_new(num, data_len));
}

void *PacketCatalog_findPacketData(PacketCatalog const *in, const int num) {
    for (int i = 0; i < in->length; i++)
        if (PacketNumVal_matches(in->pairs[i], num))
            return &(in->pairs[i]->pd);

    return PACKETS_CATALOG_NOT_FOUND;
}

void PacketCatalog_xorData(PacketCatalog const *in, const unsigned char xorVal) {
    for (int i = 0; i < in->length; i++) {
        for (int j = 0; j < in->pairs[i]->pd.data_len; j++) {
            in->pairs[i]->pd.data[j] ^= xorVal;
        }
    }
}

void PacketCatalog_convertToImage(PacketCatalog const *in) {
    FILE *fptr;
    PacketData *tmpPd;

    fptr = Fopen(in->fname, "wb");

    if (NULL != fptr) {
        for (int i = 0; i < in->length; i++) {
            tmpPd = PacketCatalog_findPacketData(in, i);

            if (PACKETS_CATALOG_NOT_FOUND != tmpPd) {
                Fwrite(tmpPd->data, 1, tmpPd->data_len, fptr);
            }
        }

        Fclose(fptr);
    }

}
