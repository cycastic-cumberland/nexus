//
// Created by cycastic on 8/6/2023.
//

#include "cmd_handler.h"

CmdHandler* CmdHandler::singleton = nullptr;

void CmdHandler::load_cmd_arguments(int argc, char **argv) {
    delete singleton;
    Vector<VString> args(argc);
    for (int i = 0; i < argc; i++){
        args.push_back(VString::from_utf8(argv[i]));
    }
    singleton = new CmdHandler(args);
}
