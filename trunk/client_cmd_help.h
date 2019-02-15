#ifndef CLIENT_CMD_HELP_H
#define CLIENT_CMD_HELP_H

#include "i_command_type.h"


namespace Client
{
class ClientCmdHelp : public ICommandType
{
    public:
        ClientCmdHelp();
        virtual ~ClientCmdHelp();
        virtual int Parse(char *);
        virtual int Execute(ClientServiceApi *client_service_api);

        void PrintWelcome();
        void PrintErrorMessage();
        void PrintGeneralUsage();
        void PrintSpecialUsage(char *);

    private:
        char *m_cmd_type;


};



}



#endif