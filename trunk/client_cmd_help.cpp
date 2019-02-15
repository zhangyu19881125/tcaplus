#include "client_cmd_help.h"
#include <stdio.h>
#include <string.h>
#include <new>
using namespace Client;

// extern int PRINT_SYMBOL_TIMES = 80;
// extern char *SYMBOL = "-";


ClientCmdHelp::ClientCmdHelp()
{
    m_cmd_type = NULL;
}



ClientCmdHelp::~ClientCmdHelp()
{
    if (NULL != m_cmd_type)
    {
        delete m_cmd_type;
    }
    m_cmd_type = NULL;
}

int ClientCmdHelp::Parse(char *sql_cmd)
{
    char first_cmd[10] = {0};
    char second_cmd[10] = {0}; // 最多扫描9个字符, 因为不可能超过9个
    sscanf(sql_cmd,"%9s%*[ |\t]%9s",first_cmd, second_cmd);


    if (0 != strcasecmp("help", first_cmd))
    {
        fprintf(stderr, "Error: command must start with help!");
        return -1;
    }
    fprintf(stdout, "second command is %s", second_cmd);
    if (strlen(second_cmd) > 0)
    {
        m_cmd_type = new (std::nothrow) char[strlen(second_cmd) + 1];
        strcpy(m_cmd_type, second_cmd);
    }
    return 0;
}

int ClientCmdHelp::Execute(ClientServiceApi *client_service_api)
{
    if (NULL == m_cmd_type)
    {
        PrintGeneralUsage();
    }
    else
    {
        PrintSpecialUsage(m_cmd_type);
    }
    return 0;
}

void Print(int count, const char* symbol, int enter_count = 1)
{
    for (int i = 0; i < count; ++i)
    {
        printf("%s",symbol);
    }
    for (int i = 0; i < enter_count; ++i)
    {
        printf("\n");
    }
}

void PrintUsageOfShow()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    show tables\t output all table names (not implemented)\n\n");
	fprintf(stdout, "    show status\t output appID, zoneID, dir server status, tcaproxy status,\n\n");
	fprintf(stdout, "               \t all the dir server addresses\n\n");
	fprintf(stdout, "    show router\t output last Proxy addr\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void PrintUsageOfDir()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    dir server_url\t add the giving dir server url\n\n");
	fprintf(stdout, "    \t\t\t possibly adding more server_url at one time, and useing comma to split\n\n");
	fprintf(stdout, "    \t\t\t e.g. dir 172.25.40.181:3399\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void PrintUsageOfDesc()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    desc table       \t show table description\n\n");
	fprintf(stdout, "    desc table.member\t show table description with member(TDR mode only)\n\n");
	fprintf(stdout, "                     \t e.g. desc table_list\n\n");
	fprintf(stdout, "                     \t       desc table_list.items\n\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void PrintUsageOfCount()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    count table\t show record count\n\n");
	fprintf(stdout, "               \t e.g. count table_list\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void PrintUsageOfSelect()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    select\t for query, e.g.: \n\n");
	fprintf(stdout, "    \t select * from table_name where uin=12 and name=\"test\" [and -index=0]\n\n");
	fprintf(stdout, "    \t select level,age from table_name where uin=12 and name=\"test\" [and -index=0]\n\n");
	fprintf(stdout, "    \t select can be use for part value querying (none tdr mode only), or part key query.\n\n");
	fprintf(stdout, "    \t when using -index key, it means list table single value querying.\n\n    \t otherwise Generic or List will be computed automatically\n\n");
	fprintf(stdout, "    \t where must be followed by key or -index field. \n\n");
	fprintf(stdout, "    \t support keyword\"or\", during batch querying, all key field can be connected by the keyword. \n\n");
	fprintf(stdout, "    \t e.g.:select * from table_name where uin=12 and name=\"test\" or uin=13 and name = \"test2\"\n\n");
	fprintf(stdout, "    \t select supports pipelining result to a given file (xml format for tdr). \n\n");
	fprintf(stdout, "    \t e.g.:select * into outfile file_name.txt (or .xml) from table_name where uin = 12 and name = \"test\"\n");
	fprintf(stdout, "         or:select * into file_name.txt (or .xml) from table_name where uin = 12 and name = \"test\"\n\n");
	fprintf(stdout, "    \t select supports \"\\G\", for row display mode, one row a member\n\n");
	fprintf(stdout, "    \t or displayed in column mode , one row for a record\n\n");
	fprintf(stdout, "    \t e.g. select * from table_name where uin=12 and name=\"test\" \\G \n");
    fprintf(stdout, "    \t select supports \"\\P\", for performance mode, which display the time-delay\n\n");
    fprintf(stdout, "    \t from API, ProxyFront, ProxyEnd, SvrMain, SvrWorker to the opposite direction.\n\n");
    fprintf(stdout, "    \t e.g. select * from table_name where uin=12 and name=\"test\" \\P \n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void PrintUsageOfInsert()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    insert\t  insert a single record , not implemented\n\n");
	fprintf(stdout, "          \t  insert uin=12345 name=\"OMG\" level=1 age=2 into table_name\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void PrintUsageOfUpdate()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    update\t  update single recored, insert the record where not exist\n\n");
	fprintf(stdout, "          \t  update success in database, when with list table, it fails. e.g.:\n\n");
	fprintf(stdout, "          \t  update table_name set level=1, age=2 where uin=12345 and name=\"OMG\" [and -index=1]\n\n");
	fprintf(stdout, "          \t  update must be used with key field for updating a single record\n\n\t\t  when following more key fields with \"where\" using \"and\" to split. \n\n");
	fprintf(stdout, "          \t  key can be integer or string which needs qoutes\n\n");
	fprintf(stdout, "          \t  \"-index\" means the given index for list updating\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void PrintUsageOfDelete()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    delete\t  delete record e.g.\n\n");
	fprintf(stdout, "          \t  delete from table_name where uin=12345 and name=\"OMG\" [and -index=1]\n\n");
	fprintf(stdout, "          \t  command delete must be used with key field for updating a single record\n\n\t\t  when following more key fields with \"where\" using \"and\" to split. \n\n");
	fprintf(stdout, "          \t  \"-index\" means the given index for list deleting\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void PrintUsageOfDump()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    dump\t  dump records from tcaplus and write the records to file with csv format\n\n");
	fprintf(stdout, "        \t  dump * from TableName into OutputFile, for example:\n\n");
	fprintf(stdout, "        \t  dump * from table_test into result.txt\n\n");
	fprintf(stdout, "        \t  dump value1, value2 from table_test into result.txt\n\n");
	fprintf(stdout, "        \t  note: value1, value2 must be value field name of the table_test\n\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void PrintUsageOfLoad()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    load\t  load records from csv file and import the records to tcaplus\n\n");
	fprintf(stdout, "        \t  load TableName from InputFile, for example:\n\n");
	fprintf(stdout, "        \t  load table_test from inputfile.txt\n\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void PrintUsageOfClean()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL, 1);
	fprintf(stdout, "    clean\t  clean table\n\n");
	fprintf(stdout, "         \t  clean TableName, for example:\n\n");
	fprintf(stdout, "         \t  clean table_test\n\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL, 2);
}

void PrintUsageOfQuit()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	fprintf(stdout, "    quit\t quit the client\n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void ClientCmdHelp::PrintWelcome()
{
    printf("+");
    Print(PRINT_SYMBOL_TIMES-2, SYMBOL, 0);
    printf("+\n");
    int len = strlen("tcaplus_client x86_64 build at Wed Jan 18 22:08:38 CST 2017");
    fprintf(stdout, "|    tcaplus_client x86_64  build at Wed Jan 18 22:08:38 CST 2017");
    printf("              |\n");
    printf("|");
    Print(PRINT_SYMBOL_TIMES-2, " ", 0);
    printf("|\n");
    char format[20];
    printf("|    ");
    sprintf(format, "%%-%ds", len);
    fprintf(stdout, format, "Welcome!");
    printf("               |\n");
    printf("+");
    Print(PRINT_SYMBOL_TIMES-2, SYMBOL, 0);
    printf("+\n");
    printf("\n");
}

// parameter error
void ClientCmdHelp::PrintErrorMessage()
{
    Print(PRINT_SYMBOL_TIMES, SYMBOL);
    fprintf(stdout, "invalid parameters, please start the client as following:\n\n");
    fprintf(stdout, "    ./tcaplus_client -a app_id -z zone_id -s signature -d dir_server_url \n");
    fprintf(stdout, "                    [-t table_name] [-l client_log.xml] [-T tdr_file.tdr] [-e execute_command]\n\n");
    fprintf(stdout, "    -a(--app_id)       APP ID\n\n");
    fprintf(stdout, "    -z(--zone_id)      ZONE ID\n\n");
    fprintf(stdout, "    -s(--signature)    SIGNATURE\n\n");
    fprintf(stdout, "    -d(--dir)          dir server addr\n\n");
    fprintf(stdout, "    -t(--table)        table to add\n\n");
    fprintf(stdout, "    -l(--log)          log file name that must be client_log.xml, and log class name be client\n\n");
    fprintf(stdout, "    -T(--tdr)          tdr filename \n\n");
    fprintf(stdout, "    -e(--execute)      content following should be with qoutes.\n\n");
    fprintf(stdout, "    e.g. ./tcaplus_client -a 2 -z 3 -s FE6533875C8385C3 -d 172.25.40.181:9999 -T ./table_test.tdr -e 'select * from table_test limit 1;'\n");
    Print(PRINT_SYMBOL_TIMES, SYMBOL);
    printf("\n");
}

void ClientCmdHelp::PrintGeneralUsage()
{
	Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
	// ????help??????息
	fprintf(stdout, "    show\t\t get server status related information. executing help show for details.\n\n");
	fprintf(stdout, "    dir\t\t\t add dir server url. if no dir_server_url added ,\n\n\t\t\t commands will fail when executing.\n\n");
	fprintf(stdout, "    desc\t\t output current table struction description\n\n");
	fprintf(stdout, "    count\t\t output table row count\n\n");
	fprintf(stdout, "    select\t\t query data\n\n");
	fprintf(stdout, "    update\t\t update record. if record does not exist then add it or update it if exists.\n\n");
	fprintf(stdout, "    insert\t\t insert a new record (not implemented)\n\n");
	fprintf(stdout, "    delete\t\t delete record(s)\n\n");
    fprintf(stdout, "    bson\t\t execute bson query\n\n");
	fprintf(stdout, "    dump\t\t dump records from tcaplus to file with csv format\n\n");
	fprintf(stdout, "    load\t\t load records from csv file and import the records to tcplus\n\n");
	fprintf(stdout, "    clean\t\t clean table\n\n");
	fprintf(stdout, "    quit\t\t exit the client\n\n");
	fprintf(stdout, "    help\t\t follow a command to get details.\n\n\t\t\t e.g. help show, help dir etc.\n\n");
	fprintf(stdout, "    note: tdr mode means starting client with -T, and none tdr mode starting client without -T\n\n");
	fprintf(stdout, "         \n");
	Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
}

void ClientCmdHelp::PrintSpecialUsage(char* command_type)
{
	if (0 == strcasecmp(command_type, "show"))
    {
        PrintUsageOfShow();
    }
    else if (0 == strcasecmp(command_type, "dir"))
    {
        PrintUsageOfDir();
    }
    else if (0 == strcasecmp(command_type, "desc"))
    {
        PrintUsageOfDesc();
    }
    else if (0 == strcasecmp(command_type, "count"))
    {
        PrintUsageOfCount();
    }
    else if (0 == strcasecmp(command_type, "select"))
    {
        PrintUsageOfSelect();
    }
    else if (0 == strcasecmp(command_type, "insert"))
    {
        PrintUsageOfInsert();
    }
    else if (0 == strcasecmp(command_type, "update"))
    {
        PrintUsageOfUpdate();
    }
    else if (0 == strcasecmp(command_type, "delete"))
    {
        PrintUsageOfDelete();
    }
    else if (0 == strcasecmp(command_type, "dump"))
    {
        PrintUsageOfDump();
    }
    else if (0 == strcasecmp(command_type, "load"))
    {
        PrintUsageOfLoad();
    }
    else if (0 == strcasecmp(command_type, "clean"))
    {
        PrintUsageOfClean();
    }
    else if (0 == strcasecmp(command_type, "quit"))
    {
        PrintUsageOfQuit();
    }
}
