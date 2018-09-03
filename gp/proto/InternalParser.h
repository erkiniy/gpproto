//
// Created by Jaloliddin Erkiniy on 9/3/18.
//

#ifndef GPPROTO_INTERNALPARSER_H
#define GPPROTO_INTERNALPARSER_H

#include "gp/proto/ProtoInternalMessage.h"
#include "gp/utils/StreamSlice.h"
#include "gp/utils/Logging.h"
#include "gp/utils/InputStream.h"

#include <zlib.h>

namespace gpproto::InternalParser {

    static std::shared_ptr<StreamSlice> decompressGZip(const std::shared_ptr<StreamSlice>& data)
    {
        const int kMemoryChunkSize = 1024;
        auto length = data->size;
        int windowBits = 15 + 32;
        int retCode;

        unsigned char output[kMemoryChunkSize];
        uInt gotBack;
        StreamSlice* result;
        size_t currentResultSize = 0;
        z_stream stream;

        if (length == 0)
            return nullptr;

        bzero(&stream, sizeof(z_stream));
        stream.avail_in = (uInt)length;
        stream.next_in = data->bytes;

        retCode = inflateInit2(&stream, windowBits);
        if (retCode != Z_OK)
        {
            LOGE("\"%s: inflateInit2() failed with error %i\", __PRETTY_FUNCTION__, retCode");
            return nullptr;
        }

        result = new StreamSlice(length * 4);

        do {
            stream.avail_in = kMemoryChunkSize;
            stream.next_out = output;
            retCode = inflate(&stream, Z_NO_FLUSH);

            if ((retCode != Z_OK) && (retCode != Z_STREAM_END))
            {
                LOGE("\"%s: inflate() failed with error %i\", __PRETTY_FUNCTION__, retCode");
                inflateEnd(&stream);
                return nullptr;
            }
            gotBack = kMemoryChunkSize - stream.avail_out;
            if (gotBack > 0) {
                memcpy(result->begin(), output, gotBack);
                currentResultSize += gotBack;
            }
        } while (retCode == Z_OK);

        inflateEnd(&stream);

        if (retCode != Z_STREAM_END)
            return nullptr;

        result->size = currentResultSize;

        return std::shared_ptr<StreamSlice>(result);
    }

    static std::shared_ptr<StreamSlice> unwrapMessage(const std::shared_ptr<StreamSlice>& data)
    {
        if (data->size < 4)
            return data;

        uint32_t signature = 0;
        memcpy(&signature, data->rbegin(), 4);

        if (signature == 0x3072cfa1)
        {
            auto packetData = data->subData(4, data->size - 4);
            return decompressGZip(packetData);
        }
        return data;
    }

    static std::shared_ptr<ProtoInternalMessage> parseMessage(const std::shared_ptr<StreamSlice>& data)
    {
        InputStream is = InputStream(data);

        uint32_t signature = is.readUInt32();

        switch (signature)
        {
            //case
        }

        return nullptr;
    }
}
#endif //GPPROTO_INTERNALPARSER_H
