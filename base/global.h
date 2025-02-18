#pragma once

#include <QString>

struct Global {
    QString mActiveCodecs;

    static QString sqlDriverName();
    static QString sqlTableTelCreate();
    static QString sqlTableTelSelect();
    static QString sqlTableTelInsert();
    static QString sqlTableTelActiveCount();
    static QString sqlTableTelUpdate();
    static QString sqlTableTelDelete();
    static QString sqlTableGlbCreate();
    static QString sqlTableGlbInsert();
    static QString sqlTableGlbSelect();
    static QString sqlTableGlbUpdate();
};
