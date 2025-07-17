#include "AppClass.hpp"

    bool AppClass::CheckString(const char* ch){
        clearBuffs();
        while (*ch != '\0') {
            if (*ch >= '0' && *ch <= '9') {
                Digit(*ch);
            }
            else if ((*ch >= 'A' && *ch <= 'Z') || (*ch >= 'a' && *ch <= 'z')) {
                CharFull(*ch);
            }
            else if (*ch == '|') {
                _fsm.Separator();
            }
            else if (*ch == ':' || *ch == '/') {
                Special1(*ch);
            }
            else if (*ch == '+' || *ch == '-' || *ch == '_' || *ch == '.') {
                Special2(*ch);
            }
            else {
                _fsm.Unknown();
            }
            ++ch;
        }
        return isAcceptable;
}

void AppClass::clearBuffs(){
    _fsm.setState(Map1::Start);
    clearBuff();
    isAcceptable = false;
}
