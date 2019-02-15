#include "readline.h"
#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
#else
#include <unistd.h>
#endif
#include <pal/pal.h>
#include <tlog/tlog.h>
#include <tloghelp/tlogload.h>
#include "tcaplus_service_api.h"
#include "version.h"
#include "TMemoryUtil.h"
#include "google/protobuf/stubs/common.h"
#include "client_parameter_init.h"
#include "i_command_type.h"
#include "client_cmd_help.h"
#include "client_cmd_desc.h"
#include "string_processor.hpp"



using namespace Client;

//const int MAX_ARRAY_SIZE = 50;
static const int BUFFER_SIZE = 50000;
static bool g_thread_stop_flag = false;
static int g_thread_sleep_us = 500 * 1000;
//const int PRINT_SYMBOL_TIMES = 80;

#define MAX_SQL_CMD_LEN 10*1024  // SQL �����󳤶� 10KB

int main(int argc, char* argv[])
{
    // ���SQL �����ַ�����buffer
    char sql_cmd_buf[MAX_SQL_CMD_LEN] = {0};
    char * const sql_cmd_ptr                 = sql_cmd_buf;

    // service api ����
    ClientServiceApi   client_service_api_inst;

    // ���������в�����ʼ��
    int ret = ClientParameterInit()(argc, argv, &client_service_api_inst, sql_cmd_ptr);
    // fprintf(stdout, "sql_cmd_ptr: %s \n", sql_cmd_ptr);
    if (0 != ret)
    {
        return -1;
    }
    // ������־���
    // LPTLOGCATEGORYINST m_log_handler = client_service_api_inst.GetTlogCategoryInst();

    // if (NULL == sql_cmd) PrintWelcome();


    bool quit_flag                    = false;  // �˳�ѭ����־λ
    ICommandType       *command_type  = NULL;   // ָ��ͬ�������͵Ľӿ�ָ��
    while (true)
    {
        if (0 != strlen(sql_cmd_ptr))  // ʹ�� -e ����, �������д���SQL ���, ִ�к��˳�
        {
            quit_flag = true;
        }
        else
        {
            // char* read_line = NULL;
            // read_line = readline("tcaplus>");
            // InitializeReadline();
            char *read_line = readline("tcaplus> ");
            // ����
            if (true == StringProcessor().IsEmptyLine(read_line)) continue;
            // read_line = readline("tcaplus> ");
            strcpy(sql_cmd_ptr, read_line);

            // while (true)  // �����ԷֺŽ�β
            // {
            //     read_line = readline("      ->");
            //     strcpy(command+count, read_line);
            //     strcpy(sql_cmd_ptr, "help select;");
            //     break;
            // }
            add_history(sql_cmd_ptr);
            // command[count-1] = '\0';
        }

        // ɾ���ַ�����ͷ�Ŀո��tab����
        StringProcessor().RemoveSpaceOfHead(sql_cmd_ptr);
        // ��ȡ��һ��������, new һ������
        char first_cmd[10] = {0};  // ��һ������������ᳬ��9���ַ�
        sscanf(sql_cmd_ptr,"%9s", first_cmd);

        // fprintf(stdout, "first_cmd: %s, strlen(sql_cmd_ptr): %d \n", first_cmd, (int)strlen(sql_cmd_ptr));

        if (0 == strcasecmp("help", first_cmd))
        {
            command_type = (ICommandType*) new (std::nothrow) ClientCmdHelp();
        }
        else if (0 == strcasecmp("desc", first_cmd))
        {
            command_type = (ICommandType*) new (std::nothrow) ClientCmdDesc();
        }
        else
        {
            // δ֪����
            fprintf(stdout, "command is invalid, use help");
        }

        if (NULL != command_type)
        {
            ret = command_type->Parse(sql_cmd_ptr);  // �����sql_cmd_ptr ��ͷ�Ŀո��tab�����ѱ�ɾ��, �����м��δ����, ��Ҫÿ�������Լ�����
            if (0 != ret)
            {
                quit_flag = true;
            }

            ret = command_type->Execute(&client_service_api_inst);  // ִ��SQL ���
            if (0 != ret)
            {
                quit_flag = true;
            }

            delete command_type;  // ��ʽ����
        }

        if (quit_flag == true)
        {
            break;
        }
        else
        {
            memset(sql_cmd_ptr, 0, MAX_SQL_CMD_LEN);
        }
    }

    return 0;

}

