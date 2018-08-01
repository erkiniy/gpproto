//
// Created by ProDigital on 8/1/18.
//

#ifndef GPPROTO_CUSTOMSTRINGCONVERTABLE_H
#define GPPROTO_CUSTOMSTRINGCONVERTABLE_H
#include <string>

namespace gpproto {
    class CustomStringConvertable {
    public:
        virtual std::string description() const = 0;
    };
}
#endif //GPPROTO_CUSTOMSTRINGCONVERTABLE_H
