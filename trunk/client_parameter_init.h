#ifndef CLIENT_PARAMETER_INIT_H
#define CLIENT_PARAMETER_INIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client_service_api.h"
#include "tcaplus_service_api.h"
#include "tcaplus/src/protocol/tcapdir_common_define.h"
#include "tcaplus_type.h"

using namespace std;

namespace Client
{

class ClientServiceApi;

class ClientParameterInit
{
public:
    ClientParameterInit();
    ~ClientParameterInit();

    // ≥ı ºªØservice_api
    int operator()(int argc, char* argv[], Client::ClientServiceApi *, char *);

};
}
#endif