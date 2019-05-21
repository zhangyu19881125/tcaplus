#include "tcaplus_define.h"
#include "tcaplus_service_api.h"
#include "tcaplus_engine.h"
#include "tcapsvr_list.h"

using namespace TcaplusService;
using namespace TcaplusEngine;
using namespace TCAPSVR;

#define RECORD_TYPE_INDEX 1
#define RECORD_TYPE_ELEMENT 2
#define TCAPLUS_MAX_FIELD_NAME 100

static int callback_func(TCapCache* cache, TCapRecord* record, void* data);

// 全局变量
TcaplusServiceRequest* m_service_request = NULL;
TcaplusServer* m_service_server = NULL;
LPTLOGCATEGORYINST m_log_handler;
LPTLOGCTX m_log_ctx;
char table_name[] = "CEVList";

int main(int argc, char* argv[])
{
    // param 1: 记录类型, 取值: INDEX/ELEMENT
    // param 2: tdr 文件路径
    // param 3: 引擎文件路径
    if (argc != 4)
    {
        fprintf(stderr, "ERROR: param count is not 3. usage: ./api_test INDEX/ELEMENT tdr_file txh_file \n");
        return -1;
    }
    int m_record_type = 0;
    if (strcmp("INDEX", argv[1]) == 0)
    {
        m_record_type = RECORD_TYPE_INDEX;
    }
    else if(strcmp("ELEMENT", argv[1]) == 0)
    {
        m_record_type = RECORD_TYPE_ELEMENT;
    }
    else
    {
        fprintf(stderr, "ERROR: 1st param should be INDEX or ELEMENT \n");
        return -1;
    }

    // 创建日志句柄

    m_log_ctx = tlog_init_file_ctx("api_test", TLOG_PRIORITY_DEBUG, "./api_test.log", 10, 1024*1024*20, 0);
    m_log_handler = tlog_get_category(m_log_ctx, "api_test");
    tlog_debug(m_log_handler, 0, 0, "first log");

    // 加载tdr 和txh
    char m_tdr_file[TCAPLUS_MAX_FIELD_NAME] = {0};
    char m_txh_file[TCAPLUS_MAX_FIELD_NAME] = {0};
    if(strlen(argv[2]) > TCAPLUS_MAX_FIELD_NAME - 1)
    {
        fprintf(stderr, "ERROR: tdr file name too long \n");
        return -1;
    }
    strcpy(m_tdr_file, argv[2]);
    if(strlen(argv[3]) > TCAPLUS_MAX_FIELD_NAME - 1)
    {
        fprintf(stderr, "ERROR: txh file name too long \n");
        return -1;
    }
    strcpy(m_txh_file, argv[3]);

    fprintf(stdout, "params are: %d, %s, %s \n", m_record_type, m_tdr_file, m_txh_file);

    // 加载tdr 文件
    LPTDRMETALIB m_tdr_metalib;
    int ret = tdr_load_metalib(&m_tdr_metalib, m_tdr_file);

    // 测试tdr
    LPTDRMETA m_tdr_meta = NULL;
    m_tdr_meta = tdr_get_meta_by_name(m_tdr_metalib, table_name);
    if (NULL == m_tdr_meta)
    {
        fprintf(stderr, "m_tdr_meta name CEVList's m_tdr_meta is NULL \n");
    }
    else
    {
        const char* meta_name = tdr_get_meta_name(m_tdr_meta);
        int meta_type = tdr_get_meta_type(m_tdr_meta);
        fprintf(stdout, "m_tdr_meta name is: %s, type is: %d \n", meta_name, meta_type);
    }

    // 打开引擎文件
    TCapCache m_cache;
    ret = m_cache.Open(m_txh_file, 0, NULL, EXPECTED_PROCESS_START_SPEED_SLOW, SHM_TYPE_MMAP, 2, 100, table_name, -1, NULL, NULL);
    if (0 != ret)
    {
        fprintf(stderr, "open cache %s failed \n", m_txh_file);
        return -3;
    }
    // 获取meta
    MetaData* m_cache_meta = NULL;
    m_cache_meta = MetaDataManager::GetInstance()->GetMetaDataByIdx(m_cache.GetMetaIndex());
    m_cache_meta->SetTdrMeta(m_tdr_metalib, m_tdr_meta);

    // 初始化api
    m_service_server = new(std::nothrow) TcaplusServer();
    TLogger* m_tlogger = new(std::nothrow) TLogger(m_log_handler);
    ret = m_service_server->Init(m_tlogger, 0, 2, 100, "E28329966258B6B9");
    if (0 != ret)
    {
        fprintf(stderr, "m_service_server->Init failed \n");
        return -4;
    }
    ret = m_service_server->AddDirServerAddress("tcp://10.123.9.60:9999");
    if (0 != ret)
    {
        fprintf(stderr, "m_service_server->AddDirServerAddress failed \n");
        return -5;
    }
    ret = m_service_server->RegistTable(table_name, NULL, 1000);
    if (0 != ret)
    {
        fprintf(stderr, "m_service_server->RegisterTable failed \n");
        return -6;
    }
    ret = m_service_server->ConnectAll(1000, 0);
    if (0 != ret)
    {
        fprintf(stderr, "m_service_server->ConnectAll \n");
        return -7;
    }

    // 哈希遍历
    ret = CacheManager::GetInstance()->InitCacheBuff();  // 必须要调用, 否则会core 掉
    m_cache.setLruShiftOutInToWork(false);
    m_cache.TraverseByHash(callback_func, (void*)m_cache_meta, false, m_log_handler);
    m_cache.Close(EXPECTED_PROCESS_STOP_SPEED_SLOW);

    // 析构api
    delete m_service_server;
    delete m_tlogger;
    return 0;
}

int callback_func(TCapCache* cache, TCapRecord* record, void* data)
{
    int ret = 0;
    MetaData* l_cache_meta = (MetaData*)data;
    bool l_is_index = (NULL == record->GetKeyField(l_cache_meta->GetAutoGenKeyIndex())) ? true : false;

    // 构造请求
    m_service_request = m_service_server->GetRequest(table_name);
    // 区分是index 记录还是element 记录
    if (l_is_index)
    {
        fprintf(stdout, "\n====== this is index ====== \n");
        // index 记录
        m_service_request->Init(TCAPLUS_API_LIST_INDEX_MOVEREPLACE_REQ, NULL, 0, 0, 0, 0);
        TcaplusServiceRecord* service_record = m_service_request->AddRecord();
        service_record->SetVersion(record->GetVersion());

        for (uint8_t i = 0; i < record->GetKey().nNum; ++i)
        {
            TCapKeyField l_key_field = record->GetKey().m_astFields[i];
            const TCapKeyMeta* l_key_meta = l_cache_meta->GetKeyMeta(i);
            if (NULL == l_key_meta)
            {
                // fprintf(stderr, "l_key_meta is NULL \n");
                continue;
            }
            if (l_key_meta->m_bIsGenKeyMeta == 1)
            {
                fprintf(stdout, "ignore auto gen key, i = %d \n", i);
                continue;
            }
            size_t l_buff_len = l_key_field.m_dwLen;
            const char * l_key_buff = l_key_field.m_pszBuff;
            ret = service_record->SetKey(l_key_meta->m_szName, l_key_buff, l_buff_len);
            if (0 != ret)
            {
                fprintf(stderr, "service_record->SetKey() failed, key name is: %s, ret is: %d \n", l_key_meta->m_szName, ret);
                return -1;
            }
        }

        TcapList list;
        TcapListIndex list_index;
        // auto gen value field 反序列化
        TCapValueField l_value_field = record->GetValue().m_astFields[l_cache_meta->GetAutoGenValueIndex()];
        ret = list.DerializeIndex(list_index, l_value_field.m_pszBuff, l_value_field.m_dwLen);
        if (0 != ret)
        {
            fprintf(stderr, "list.DerializeIndex failed, ret: %d \n", ret);
            return -5;
        }
        ret = m_service_request->SetListIndexMoveReplaceReq(list_index);
        if (0 != ret)
        {
            fprintf(stderr, "m_service_request->SetTcapList failed \n");
            return -6;
        }


        // 发送请求
        m_service_server->SendRequest(m_service_request);

    }
    else  // element 记录
    {
        fprintf(stdout, "\n====== this is element ====== \n");
        m_service_request->Init(TCAPLUS_API_LIST_ELEMENT_MOVEREPLACE_REQ, NULL, 0, 0, 0, 0);

        // 获取索引字段
        const TCapKeyField* index_field = record->GetKeyField(l_cache_meta->GetAutoGenKeyIndex());
        uint8_t field_type = index_field->m_bType;
        uint32_t field_len = index_field->m_dwLen;
        fprintf(stdout, "index field type: %d, len: %d \n", (int)field_type, (int)field_len);

        if (CAPLUS_RECORD_TYPE_INT32 != field_type)
        {
            fprintf(stderr, "index field type is not CAPLUS_RECORD_TYPE_INT32 \n");
            return -4;
        }
        int32_t i_Idx = tcaplus_ntoh32(*(int32_t *)index_field->m_pszBuff);
        fprintf(stdout, "index field idx: %d \n", (int)i_Idx);  // 到这里没问题
        TcaplusServiceRecord* service_record = m_service_request->AddRecord(i_Idx);
        service_record->SetVersion(record->GetVersion());

        // 打印key value 
        for (uint8_t i = 0; i < record->GetKey().nNum; ++i)
        {
            TCapKeyField l_key_field = record->GetKey().m_astFields[i];
            // fprintf(stdout, "i = %d \n", i);
            // key 字段好像没有设置valid, 这里不能写
            // if (l_key_field.m_bValid == false)
            // {
            //     fprintf(stderr, "l_key_field is invalid \n");
            //     continue;
            // }
            const TCapKeyMeta* l_key_meta = l_cache_meta->GetKeyMeta(i);
            if (NULL == l_key_meta)
            {
                // fprintf(stderr, "l_key_meta is NULL \n");
                continue;
            }
            if (l_key_meta->m_bIsGenKeyMeta == 1)
            {
                fprintf(stdout, "ignore auto gen key, i = %d \n", i);
                continue;
            }
            size_t l_buff_len = l_key_field.m_dwLen;
            const char * l_key_buff = l_key_field.m_pszBuff;
            ret = service_record->SetKey(l_key_meta->m_szName, l_key_buff, l_buff_len);
            if (0 != ret)
            {
                fprintf(stderr, "service_record->SetKey() failed, key name is: %s, ret is: %d \n", l_key_meta->m_szName, ret);
                return -1;
            }
            if (l_key_meta->m_bType == CAPLUS_RECORD_TYPE_BINARY)
            {
                fprintf(stdout, "key name is: %s, key len is: %d, key buff is: ", l_key_meta->m_szName, (int)l_buff_len);
                for (size_t j = 0; j < l_buff_len; ++j)
                {
                    fprintf(stdout, "0x%02x ", l_key_buff[j]);
                }
                fprintf(stdout, "\n");
            }

        }

        for (uint8_t i = 0; i < TCAPLUS_MAX_VALUE_FIELD_NUM; ++i)
        {
            TCapValueField l_value_field = record->GetValue().m_astFields[i];
            // fprintf(stdout, "i = %d \n", i);
            bool l_is_valid = l_value_field.m_bValid;
            if (l_is_valid)
            {
                const TCapValueMeta* l_value_meta = l_cache_meta->GetValueMeta(i);
                if (NULL == l_value_meta)
                {
                    // fprintf(stderr, "l_value_meta is NULl \n");
                    continue;
                }
                size_t l_buff_len = l_value_field.m_dwLen;
                const char * l_value_buff = l_value_field.m_pszBuff;
                ret = service_record->SetValue(l_value_meta->m_szName, l_value_buff, l_buff_len);
                if (0 != ret)
                {
                    fprintf(stderr, "service_record->SetValue() failed, value name is: %s, ret is: %d \n", l_value_meta->m_szName, ret);
                    return -2;
                }
                if (l_value_meta->m_bType == CAPLUS_RECORD_TYPE_BINARY)
                {
                    fprintf(stdout, "value name is: %s, value len is: %d, value buff is: ", l_value_meta->m_szName, (int)l_buff_len);
                    for (size_t j = 0; j < l_buff_len; ++j)
                    {
                        fprintf(stdout, "0x%02x ", l_value_buff[j]);
                    }
                    fprintf(stdout, "\n");
                }
            }
            else
            {
                // fprintf(stdout, "field is invalid, i = %d \n", i);
            }
        }



        // 发送请求
        m_service_server->SendRequest(m_service_request);
    }




    return 0;
}