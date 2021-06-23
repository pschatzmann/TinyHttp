#ifndef __STRMEM_H__
#define __STRMEM_H__

#include "Str.h"
#include "HttpLogger.h"

namespace tinyhttp {


/**
 * @brief Str which keeps the data on the heap. We grow the allocated 
 * memory only if the copy source is not fitting.
 * 
 * While it should be avoided to use a lot of heap allocatioins in 
 * embedded devices it is sometimes more convinent to allocate a string
 * once on the heap and have the insurance that it might grow
 * if we need to process an unexpected size.
 * 
 * We also need to use this if we want to manage a vecor of strings.
 * 
 */

class StrExt : public Str {

    public:
        StrExt(){            
        }

        StrExt(int initialAllocatedLength){
            maxlen = initialAllocatedLength;
        }

        StrExt(Str &source){
            set(source);
        }
        
        StrExt(StrExt &source){
            set(source);
        }

        StrExt(const char* str){
            if (str!=nullptr){
                len = strlen(str);
                maxlen = len; 
                grow(maxlen);
                strcpy(chars, str);
            }
        }

        // move constructor
        StrExt (StrExt &&obj) = default;

        // copy assignment
        StrExt& operator = (StrExt &&obj) = default;

        // move assingment
        StrExt& operator = (StrExt &obj) {
            set(obj.c_str());
        };


        ~StrExt() {
            if (chars!=nullptr){
                delete [] chars;
                chars = nullptr;
            }
        }

        bool isOnHeap() {
            return true;
        }
        
        bool isConst() {
            return false;
        }

        void operator=(const char* str)  {
            set(str);
        }
    
        void operator=(char* str)  {
            set(str);
        }

        void operator=(int v)  {
            set(v);
        }

        void operator=(double v)  {
            set(v);
        }

        size_t capacity() {
            return maxlen;
        }

        void setCapacity(size_t newLen){
            grow(newLen);
        }
    
    protected:

        bool grow(int newMaxLen){
            bool grown = false;
            
            if (chars==nullptr || newMaxLen > maxlen ){
                char msg[80];
                sprintf(msg,"StrExt::grow(%d)",newMaxLen);
                Log.log(Info, msg);

                grown = true;
                // we use at minimum the defined maxlen
                int newSize = newMaxLen > maxlen ? newMaxLen : maxlen;                
                if (chars!=nullptr){
                    char* tmp = chars;
                    chars = new char[newSize+1];
                    strcpy(chars,tmp);
                    delete [] tmp;
                } else {
                    chars = new char[newSize+1];
                    chars[0] = 0;
                }
                maxlen = newSize;
                
            }
            return grown;
        }
};

}

#endif // __STRMEM_H__