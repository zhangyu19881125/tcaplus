#include <stdio.h>
#include <string.h>
#include "base64.h"
#include "tlog/tlog.h"
#include <arpa/inet.h>
#include "tcaplus_engine.h"


#define MAX_BUFF_LEN 1024*1024
// dumpenginefile 结果文件, base64 解码, 反序列化, 并以xml 格式输出到文件
int main(int argc, char* argv[])
{
    LPTLOGCTX log_ctx = tlog_init_file_ctx("dumpenginefile", 600, "./dumpenginefile_log/dump_engine_file.log", 10, 1024*1024*20, 0);
    LPTLOGCATEGORYINST g_log_handler = tlog_get_category(log_ctx, "dumpenginefile");
    FILE *pFile;
    pFile = fopen("./TB_PLAYERINFO_31_20190127.log.3.7372", "r");
    char read_buf[MAX_BUFF_LEN];
    fgets(read_buf, MAX_BUFF_LEN, pFile);
    fclose(pFile);

    // 第9段是 HeroInfo
    char* HeroInfoBuf;
    HeroInfoBuf = strtok(read_buf, "|");
    for (int i = 0; i < 8; ++i)
    {
        HeroInfoBuf = strtok(NULL, "|");
    }
    fprintf(stdout, "%s strlen: %d\n", HeroInfoBuf, (int)strlen(HeroInfoBuf));

    // base64 解码
    Base64 g_base64;
    char out_buf[MAX_BUFF_LEN];
    int out_buf_len = MAX_BUFF_LEN;
    memset(out_buf, 0, out_buf_len);
    int ret = g_base64.DecodeBase64(HeroInfoBuf, strlen(HeroInfoBuf), out_buf, out_buf_len, g_log_handler);
    char* temp_buf = out_buf;
    int16_t nVersion = tcaplus_ntoh16(*(int16_t *)temp_buf);
    temp_buf += sizeof(int16_t);
    out_buf_len -= sizeof(int16_t);
    fprintf(stdout, "%s\n%d", temp_buf, out_buf_len); // 打不出来, 可能非二进制安全
    

    // 反序列化, 忽略前两个字节
    TDRDATA stHost = {0};
    TDRDATA stNet = {0};
    stNet.pszBuff = temp_buf;
    stNet.iBuff = out_buf_len;
    char stHost_buff[MAX_BUFF_LEN] = {0};
    stHost.pszBuff = (char*)stHost_buff;
    stHost.iBuff   = MAX_BUFF_LEN;
    LPTDRMETA       pstMeta = NULL;
    LPTDRMETALIB    meta_lib = NULL;
    ret = tdr_load_metalib(&meta_lib, "./31.3.TB_PLAYERINFO.tdr");
    pstMeta = tdr_get_meta_by_name(meta_lib, "TB_PLAYERINFO");
    LPTDRMETAENTRY pstEntry = tdr_get_entryptr_by_name(pstMeta, "HeroInfo");
    pstMeta = tdr_get_entry_type_meta(meta_lib, pstEntry);
    ret = tdr_ntoh(pstMeta, &stHost, &stNet, nVersion);
    fprintf(stdout, "\nstHost.iBuff: %d", (int)stHost.iBuff);

    // 输出到文件
    tdr_output_file(pstMeta, "./HeroInfo.txt", &stHost, 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
    return 0;
}
