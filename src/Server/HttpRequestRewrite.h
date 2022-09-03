#ifndef __HTTPREQUESTREWRITE_H__
#define __HTTPREQUESTREWRITE_H__


#include "Basic/StrExt.h"

namespace tinyhttp {

class HttpRequestRewrite {
    public:
        HttpRequestRewrite(const char *from, const char* to ){
            this->from = Str(from);
            this->to = Str(to);
        }
        Str from;
        Str to;

};

}
#endif // __HTTPREQUESTREWRITE_H__