#ifndef _APPCLASS_HPP
#define _APPCLASS_HPP

#include "AppClass_sm.h"
#include <string>

enum states{
    digit,
    charFull,
    special1,
    separator,
    special2,
    unknow
};

class AppClass{
    AppClassContext _fsm;
    bool isAcceptable = false;

    std::string TempBuff;
    char currentChar;

    void clearBuffs();

    public:
    AppClass():_fsm(*this){}

    ~AppClass(){}

    inline bool isBackslash()
    {return currentChar == '/';}
    inline bool IsCharShortcut()
    {return currentChar >= 'a' && currentChar <= 'f' || currentChar >= 'A' && currentChar <= 'F';}

    bool CheckString(const char*);

    inline void Acceptable()
    {isAcceptable = true;}

    inline void Unacceptable()
    {isAcceptable = false;}

    inline void AddToBuff()
    {TempBuff += currentChar;}

    inline void Digit(char cur)
    {
        currentChar = cur;
        _fsm.Digit();
    }

    inline void CharFull(char cur)
    {
        currentChar = cur;
        _fsm.CharFull();
    }    
    
    inline void Special1(char cur)
    {
        currentChar = cur;
        _fsm.Special1();
    }

    inline void Special2(char cur)
    {
        currentChar = cur;
        _fsm.Special2();
    }

    inline bool CheckTitle()
    {
        bool result =  TempBuff == "ed2k://";
        clearBuff();
        return result;
    }

    inline bool CheckHash()
    {return TempBuff.length() == 32;}

    inline bool CheckIdentifier()
    {
        bool result =  TempBuff == "file";
        clearBuff();
        return result;
    }

    inline std::string getHashBuff()
    {return TempBuff;}

    inline void clearBuff()
    {TempBuff.clear();}

    inline bool checkName()
    {
        bool result = TempBuff.length();
        clearBuff();
        return result;
    }
};

#endif