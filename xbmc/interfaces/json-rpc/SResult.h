#if !defined(__S_RESULT__H__)
#define __S_RESULT__H__

/************************************************************************/
/* SResult(Summary Result)                                              */
/************************************************************************/

typedef long SResult;


/************************************************************************/
/* General SResult                                                      */
/************************************************************************/

const SResult kUnknown  = 0; 
const SResult kOk       = 1;
const SResult kYes      = 2;
const SResult kError    = 3;


/************************************************************************/
/* Utility functions                                                    */
/************************************************************************/

inline bool IsUnknown(const SResult sResult) 
{ 
    return sResult == kUnknown; 
}

inline bool IsOk(const SResult sResult) 
{ 
    return sResult == kOk; 
}

inline bool SResultInRange(const SResult sResult,
    const SResult base, const SResult maximum)
{
    return sResult >= base && sResult < maximum;
}


/************************************************************************/
/* Macro definition                                                     */
/************************************************************************/

#define RESULT_IN_ERRORS_2(SERROR)                                      \
    inline bool In##SERROR##s_2(const SResult sResult)                  \
    {                                                                   \
        return  SResultInRange(sResult, SERROR##Base, SERROR##Maximum); \
    }                                                                   \

#define DEFINE_ERROR_RANGE(SERROR, BASE, SIZE)                          \
    const SResult SERROR = BASE;                                        \
    const SResult SERROR##Base = SERROR;                                \
    const SResult SERROR##Maximum = SERROR + SIZE;                      \
    RESULT_IN_ERRORS_2(SERROR)                                          \

#define STATIC_FUNCTION(function_name)                                  \
    STATIC_FUNCTION_2(static_function_##function_name)                  \

#define STATIC_FUNCTION_2(static_function)                              \
    class static_function                                               \
    {                                                                   \
    private:                                                            \
        static_function();                                              \
        static static_function run;                                     \
    };                                                                  \
    static_function static_function::run;                               \
    static_function::static_function( void )                            \

#define STATIC_CHECK_ERROR_MAXIMUM(SERROR, Maximum)                     \
    STATIC_FUNCTION(SERROR)                                             \
    {                                                                   \
        assert( SERROR##Maximum >= Maximum);                            \
    }                                                                   \

#if 0
STATIC_CHECK_ERROR_MAXIMUM(SessionResult, srMaximum);
#endif


/************************************************************************/
/* Application Level SResult Base                                       */
/************************************************************************/

/***  common applications ***/
DEFINE_ERROR_RANGE(CommunicationError, 10000, 10000);
DEFINE_ERROR_RANGE(ApplicationError, 20000, 10000);

/***  specific applications ***/
//DEFINE_ERROR_RANGE(VDMServerError, 30000, 10000);
//DEFINE_ERROR_RANGE(VDMCenterError, 40000, 10000);

/************************************************************************/
/* CommunicationError SResult Base                                      */
/************************************************************************/

/*** (10000 -- 12000) The transport layer ***/
DEFINE_ERROR_RANGE(CurlFileError    , CommunicationError, 20);
DEFINE_ERROR_RANGE(TCPClientError   , CommunicationError + 20, 20);
DEFINE_ERROR_RANGE(UDPClientError   , CommunicationError + 40, 20);
DEFINE_ERROR_RANGE(PipeError        , CommunicationError + 60, 20);

/*** (12000 -- 19000) The presentation layer ***/
DEFINE_ERROR_RANGE(JsonRpcError     , CommunicationError + 2000, 20);
DEFINE_ERROR_RANGE(XmlRpcError      , CommunicationError + 2020, 20);
DEFINE_ERROR_RANGE(SoapError        , CommunicationError + 2040, 20);
DEFINE_ERROR_RANGE(SambaError       , CommunicationError + 2060, 20);
DEFINE_ERROR_RANGE(HttpError        , CommunicationError + 2080, 20);
DEFINE_ERROR_RANGE(WebSocketError   , CommunicationError + 2100, 20);

/*** (19000 -- 20000) The Proxy layer ***/
DEFINE_ERROR_RANGE(RemoteModuleError    , CommunicationError + 9000, 20);


/************************************************************************/
/* ApplicationError SResult Base                                        */
/************************************************************************/

DEFINE_ERROR_RANGE(SessionError         , ApplicationError + 20, 20);
//DEFINE_ERROR_RANGE(SessionActionError   , ApplicationError + 40, 20);
//DEFINE_ERROR_RANGE(SourcesManagerError  , ApplicationError + 60, 20);


//module, application part
DEFINE_ERROR_RANGE(MediaServerError     , ApplicationError +  500, 500);
DEFINE_ERROR_RANGE(ImageServerError     , ApplicationError + 1000, 500);


#endif