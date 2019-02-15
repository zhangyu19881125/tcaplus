// command type ½Ó¿ÚÀà

#ifndef CLIENT_I_COMMAND_TYPE
#define CLIENT_I_COMMAND_TYPE

#include "client_service_api.h"

namespace Client
{

class ICommandType
{
    public:
        ICommandType(){};
        virtual ~ICommandType(){};
        virtual int Parse(char *) = 0;
        virtual int Execute(ClientServiceApi *client_service_api) = 0;
};
}
#endif