#pragma once

#include <binge/Types.hpp>

class ContextParser {
    private:
    ContextParser();
    static ContextParser* instance;
public:
    static ContextParser* getInstance();
    bool parse(Frame *frame);
};
