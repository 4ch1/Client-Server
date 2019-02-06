#include "Protocol.h"

Protocol::Protocol(const std::string &inMessage,protocolType type,protocolCreator creator,bool success):
    creator(creator),
    data(inMessage),
    type(type),
    success(success)
{
}
std::string Protocol::prepToSend() const
{
    std::string retString = this->creator == CLIENT ? "C" : "S";
    switch(this->type)
    {
        case UINFO:
            retString.push_back('U');
            break;
        case HOMEDIR:
            retString.push_back('H');
            break;
        case LIST:
            retString.push_back('L');
            break;
        default:
            break;
    }

    if(this->creator == CLIENT)
        retString.append("X");
    else
        retString.append(this->success == true ? "T" : "F");

    retString.append("K" + std::to_string(this->data.size()));

    retString.append(std::string(":") + this->data);

    return retString;
}
