// Public.h
#ifndef AUDIORESAMPLE_PUBLIC_H
#define AUDIORESAMPLE_PUBLIC_H

// 单例模式宏定义
#define DECLARE_SINGLETON(ClassName)                  \
public:                                               \
    static ClassName& instance() {                   \
        static ClassName inst;                        \
        return inst;                                  \
    }                                                 \
private:                                              \
    ClassName(const ClassName&) = delete;            \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete;                 \
    ClassName& operator=(ClassName&&) = delete;


#endif // AUDIORESAMPLE_PUBLIC_H