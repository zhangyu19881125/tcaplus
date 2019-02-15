#include "client_parameter_init.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <inttypes.h>
#include "pal/getopt.h"
#include "pal/tstring.h"
#else
#include <unistd.h>
#include <getopt.h>
#include "mytypes.h"
#endif

using namespace Client;

ClientParameterInit::ClientParameterInit()
{

}

ClientParameterInit::~ClientParameterInit()
{

}

int ClientParameterInit::operator()(int argc, char* argv[], Client::ClientServiceApi* client_service_api, char *sql_cmd)
{
	const char* short_options = "a:z:s:d:t:l:e:T:";
	const struct option long_option[] = {
		{"app_id", 1, NULL, 'a'},
		{"zone_id", 1, NULL, 'z'},
		{"signature", 1, NULL, 's'},
		{"dir", 1, NULL, 'd'},
		{"table", 1, NULL, 't'},
		{"log", 1, NULL, 'l'},
		{"execute", 1, NULL, 'e'},
		{"tdr", 1, NULL, 'T'},
		{0, 0, 0, 0}};

    LPTLOGCTX m_log_ctx = NULL;
	LPTLOGCATEGORYINST m_log_handler = NULL;
	bool init_log_flag = false;  //flag for initialization
	char ch;
	while ((ch = getopt_long(argc, argv, short_options, long_option, NULL)) != -1)
	{

		switch(ch)
		{
		case 'l':
			{
				m_log_ctx = tlog_init_from_file(optarg);
				if (NULL == m_log_ctx)
				{
					// Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
					fprintf(stdout, "	  log init failed.\n");
					// Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
					return -1;
				}

				m_log_handler = tlog_get_category(m_log_ctx,"client");
				if (NULL == m_log_handler)
				{
					// Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
					fprintf(stdout, "	  the log instance handle init failed, please check log config file that the log class name is \"client\"\n");
					// Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
					return -1;
				}

				init_log_flag = true;

				break;
			}
		default:
			{
				break;
			}
		}
	}

	if (false == init_log_flag)
	{
		do
		{
			if (0 != access("../cfg/tcaplus/client_log.xml", 0))
			{
				break;
			}
			m_log_ctx = tlog_init_from_file("../cfg/tcaplus/client_log.xml");
			if (NULL == m_log_ctx)
			{
				break;
			}

			m_log_handler = tlog_get_category(m_log_ctx,"client");
			if (NULL == m_log_handler)
			{
				// Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
				fprintf(stdout, "     the log instance handle init failed, please check log config file that the log class name is \"client\"\n");
				// Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
				break;
			}

			init_log_flag = true;

		}while (0);

		if (false == init_log_flag)
		{
			m_log_ctx = tlog_init_file_ctx("client", TLOG_PRIORITY_DEBUG,
										   "./client.log", 10, 1024*1024*20, 0);
			if (NULL == m_log_ctx)
			{
				// Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
				fprintf(stdout, "    log init failed\n");
				// Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
				return -1;
			}

			m_log_handler = tlog_get_category(m_log_ctx, "client");
			if (NULL == m_log_handler)
			{
				// Print(PRINT_SYMBOL_TIMES, SYMBOL,1);
				fprintf(stdout, "      the log instance handle init failed, please check log config file that the log class name is \"client\"\n");
				// Print(PRINT_SYMBOL_TIMES, SYMBOL,2);
				return -1;
			}

			init_log_flag = true;
		}
	}

	int ret = client_service_api->InitLog(m_log_handler, m_log_ctx);
	if (0 != ret)
	{
		return -1;
	}

	//初始化错误信息
	ret = TcapErrCode::TcapErrCodeInit(m_log_handler);
	if (TcapErrCode::GEN_ERR_SUC != ret)
	{
		return ret;
	}

	optind = 0;  //getopt_long()函数用到的一个全局变量，赋值为0表示
	//从头开始索引argv的值

	while ((ch = getopt_long(argc, argv, short_options, long_option, NULL)) != -1)
	{
		switch(ch)
		{
		case 'a':
			{
				int64_t app_id = atoi(optarg);
				client_service_api->SetAppId(app_id);
				break;
			}
		case 'z':
			{
				int zone_id = atoi(optarg);
				client_service_api->SetZoneId(zone_id);
				break;
			}
		case 's':
			{
				client_service_api->SetPassWord(optarg);
				break;
			}
		case 'd':
			{
				//  for (int i = 0; i < m_array_size; ++i)
				//  {
				//  	if (m_command_field[i])
				//  	{
				//  		delete[] m_command_field[i];
				//  		m_command_field[i] = NULL;
				//  	}
				//  }
				//  SetCommandField("dir");
				//  SetCommandField(optarg);
				client_service_api->AddDirServer(optarg);
				break;
			}
		case 't':
			{
				client_service_api->SetTableName(optarg);
				break;
			}

		case 'e':
			{
				// todo: 字符串超长
				strcpy(sql_cmd, optarg);
				break;
			}
		case 'T':
			{
				client_service_api->SetTdrFile(optarg);
				break;
			}
		case '?':
			{
				break;
			}
		default:
			{
				break;
			}
		}
	}


    // 开一个子线程, 保持api 连接

	return 0;
}

