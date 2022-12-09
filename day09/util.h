#ifndef UTIL_H_
#define UTIL_H_

#include <string>

void errif(bool condition, const char *errmsg);
// void setnonblocking(int fd);


class Buffer 
{
public:
    void Append(const char* str, int size)
    {
        buf_.append(str, size);
    }
    ssize_t Size() const { return buf_.size(); }
    const char* c_str() const { return buf_.c_str(); }
    void Clear() { buf_.clear(); }
    
private:
    std::string buf_;
};


#endif //UTIL_H_

