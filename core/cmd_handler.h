//
// Created by cycastic on 8/6/2023.
//

#ifndef NEXUS_CMD_HANDLER_H
#define NEXUS_CMD_HANDLER_H

#include "types/vector.h"
#include "types/vstring.h"

class CmdHandler {
private:
    static CmdHandler* singleton;
    Vector<VString> arguments;
    explicit CmdHandler(const Vector<VString>& p_vec) : arguments(p_vec) {}
public:
    static void load_cmd_arguments(int argc, char** argv);
    static void cleanup() { delete singleton; }
    _NO_DISCARD_ _FORCE_INLINE_ static const Vector<VString>& args() { return singleton->arguments; }
};

#endif //NEXUS_CMD_HANDLER_H
