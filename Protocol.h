#pragma once

#include <string>

enum protocolType { UINFO, HOMEDIR, LIST};
enum protocolCreator { CLIENT, SERVER};

class Protocol
{
    public:
        Protocol(const std::string &inMessage,protocolType type,protocolCreator creator,bool success);

        size_t inline getDataSize() const { return data.size(); }

        std::string prepToSend() const;

        enum bytesMeaning {
            creatorByte = 0,
            typeByte,
            successByte,
            sizeStartByte
        };

    private:
        size_t messageSize;
        protocolCreator creator;
        std::string data;
        protocolType type;
        bool success;
};
