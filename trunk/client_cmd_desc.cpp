#include "client_cmd_desc.h"
#include <stdio.h>
#include <string.h>
#include <new>
#include "tcaplus_service_api.h"
#include "public_define.h"
using namespace Client;

ClientCmdDesc::ClientCmdDesc()
{
    m_table_name = NULL;
}



ClientCmdDesc::~ClientCmdDesc()
{
    if (NULL != m_table_name)
    {
        delete m_table_name;
    }
    m_table_name = NULL;
}

int ClientCmdDesc::Parse(char *sql_cmd)
{
    char first_cmd[10] = {0};
    char second_cmd[100] = {0}; // todo: 表名最大长度
    sscanf(sql_cmd,"%9s%*[ |\t]%99s",first_cmd, second_cmd);


    if (0 != strcasecmp("desc", first_cmd))
    {
        fprintf(stderr, "Error: command must start with desc!");
        return -1;
    }
    fprintf(stdout, "tablename is %s\n", second_cmd);
    if (strlen(second_cmd) > 0)
    {
        m_table_name = new (std::nothrow) char[strlen(second_cmd) + 1];
        strcpy(m_table_name, second_cmd);
    }
    return 0;
}

int ClientCmdDesc::Execute(ClientServiceApi *client_service_api)
{
    if (NULL == m_table_name)
    {
        return -1;
    }
    else
    {
		int ret = client_service_api->InitServiceApi(m_table_name);
        tlog_debug(client_service_api->GetTlogCategoryInst(), 0, 0, "client_service_api->InitServiceApi() success!");
		if (0 != ret)
		{
			// tlog_error(m_log_handler, 0, 0, "client_service_api->InitServiceApi()failed.");
			return -1;
		}

		TcaplusService::KeyValueInfo key_value_info;
		ret = client_service_api->GetTableDescInfo(m_table_name, key_value_info);
		if (0 != ret)
		{
			// tlog_error(m_log_handler, 0, 0, "client_service_api->GetTableDescInfo(\"%s\") failed.", m_table_name);
			return -1;
		}

		printf("\n%s:%s\n", "TableName", key_value_info.table_name);

        if(key_value_info.idl_type == TCAPLUS_IDL_TYPE_PROTOBUF)
        {
            printf("%s:%s\n", "TableType", "PROTOBUF");
        }
        else
        {
            printf("%s:%s\n", "TableType", (key_value_info.table_type == 0 ? "GENERIC" : "LIST"));
            printf("%s:%d\n", "SvrTdrCurrentVersion", key_value_info.svr_tdr_meta_version);
        }
		printf("-------------------------------------------------------------------------\n");
		printf("| %-30s%-30s%-10s|\n", "Field", "Type", "Key");
		printf("-------------------------------------------------------------------------\n");
		for (uint32_t i = 0; i < key_value_info.key_num; ++i)   /*lint !e574 */
		{
			if (0 == key_value_info.key_info[i].generate_by_key_meta)  //表示不是由系统产生的key
			{
				fprintf(stdout, "| %-30s%-30s%-10s|\n", key_value_info.key_info[i].key_field_name,
						types_info[key_value_info.key_info[i].key_field_type], "key");
			}
		}

		for (uint32_t i = 0; i < key_value_info.value_num; ++i)     /*lint !e574 */
		{
			if (0 == key_value_info.value_info[i].generate_by_value_mate) //表示不是由系统产生的value
			{
				fprintf(stdout, "| %-30s%-30s%-10s|\n", key_value_info.value_info[i].value_field_name,
						types_info[key_value_info.value_info[i].value_field_type],"");
			}
		}
		printf("-------------------------------------------------------------------------\n\n");
    }
    return 0;
}

