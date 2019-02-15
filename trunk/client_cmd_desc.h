#ifndef CLIENT_CMD_DESC_H
#define CLIENT_CMD_DESC_H

#include "i_command_type.h"


namespace Client
{

class ClientCmdDesc : public ICommandType
{
    public:
        ClientCmdDesc();
        virtual ~ClientCmdDesc();
        virtual int Parse(char *);
        virtual int Execute(ClientServiceApi *client_service_api);

    private:
        char *m_table_name;


};



}



#endif