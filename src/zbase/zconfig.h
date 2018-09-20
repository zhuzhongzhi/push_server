
#ifndef _X_CONFIG_H_
#define _X_CONFIG_H_

#include "zbasedef.h"

enum ECfgErrCode
{
    CFG_OK                              = 0,        
    CFG_INPUT_INVALID                   = 1,        
    CFG_FILE_OPENED                     = 2,        
    CFG_OPEN_FILE_FAILED                = 3,        
    CFG_FORMAT_ERROR                    = 4,        
    CFG_SECTION_REPEAT                  = 5,        
    CFG_ITEM_REPEAT                     = 6,        
    CFG_SYS_ERROR                       = 7,        
    CFG_SECTION_NOT_EXIST               = 8,        
    CFG_ITEM_NOT_EXIST                  = 9,        
    CFG_VALIE_INVALID                   = 10        
};


enum ECfgLineType
{
    CFG_LINE_COMMENT                    = 0,        
    CFG_LINE_SECTION                    = 1,        
    CFG_LINE_ITEM                       = 2,        
    CFG_LINE_EMPTY                      = 3,        
    CFG_LINE_INVALID                    = 4         
};


struct TCfgItem
{
    STLString                              m_strItemName;          
    STLString                              m_strItemValue;         
    STLString                              m_strComment;          
};

struct TCfgSection
{
    STLString                              m_strSecName;           
    STLString                              m_strComment;           
    std::list<TCfgItem*>                   m_strItemList;          
};

class ZConfig
{
public:
    ZConfig();

    ~ZConfig();

    ECfgErrCode             open_file(const char* file);
    ECfgErrCode             close_file(const char* file);
    ECfgErrCode             read_string(const char* sec_name, const char* item_name, STLString& value);
    ECfgErrCode             read_string(const char * sec_name, const char * item_name, char* value, int len);
    ECfgErrCode             read_int(const char * sec_name, const char * item_name, int & value, int min_value = INT_MIN, int max_value = INT_MAX);
    ECfgErrCode             read_uint(const char * sec_name, const char * item_name, unsigned int & value, unsigned int min_value = 0, unsigned int max_value = UINT_MAX);
    ECfgErrCode             modify_string(const char* sec_name, const char* item_name, const STLString& value);
    ECfgErrCode             modify_string(const char* sec_name, const char* item_name, const char* value);
    ECfgErrCode             modify_int(const char* sec_name, const char* item_name, const int value);
    ECfgErrCode             modify_uint(const char* sec_name, const char* item_name, const unsigned int value);
private:
    DEF_COPY_AND_ASSIGN(ZConfig);

    ECfgErrCode             save_file();
    bool                    check_sec_exist(const STLString& sec_name, TCfgSection* & sec);
    bool                    check_item_exist(const TCfgSection* sec, const STLString& item_name);
    ECfgLineType            get_line(STLString & line);
    TCfgSection*            parser_section(const STLString & line);
    TCfgItem*               parser_item(const STLString& line, TCfgSection * & sec);
    void                    clear();
    ECfgErrCode             read_string_i(const char* sec_name, const char* item_name, STLString& value);
    ECfgErrCode             modify_string_i(const char* sec_name, const char* item_name, const STLString& value);
    
private:
    bool                                        m_bOpen;               
    int                                         m_fd;        
    STLString                                   m_strFileName;
    std::list<TCfgSection*>                     m_listSec;            
    STLString                                   m_strLastComment;     
    int                                         m_iLineNo;         
};
#endif
