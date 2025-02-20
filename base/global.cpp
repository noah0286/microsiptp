#include "global.h"


static QString sDDriver = "QSQLITE";

static QString sSqlTableTelCreate = "\
CREATE TABLE IF NOT EXISTS telephones( \
    id INTEGER PRIMARY KEY AUTOINCREMENT, \
    description TEXT, \
    name TEXT, \
    domain TEXT, \
    username TEXT, \
    password TEXT, \
    active integer, \
    proxy TEXT, \
    should_register_startup integer, \
    should_subscribe_presence integer, \
    should_publish_presence integer, \
    should_use_blf integer, \
    should_disable_ringback_tone integer, \
    custom_ringtone TEXT, \
    transport integer, \
    subscription_expiry_delay integer, \
    keep_alive_expiry_delay integer, \
    registration_expiry_delay integer, \
    use_stun integer, \
    stun_server TEXT, \
    srtp_use integer \
)";


static QString sSqlTableTelSelect = "\
SELECT id, \
    description, \
    name, domain, \
    username, \
    password, \
    active, \
    proxy, \
    should_register_startup, \
    should_subscribe_presence, \
    should_publish_presence, \
    should_use_blf, \
    should_disable_ringback_tone, \
    custom_ringtone, \
    transport, \
    subscription_expiry_delay, \
    keep_alive_expiry_delay, \
    registration_expiry_delay, \
    use_stun, \
    stun_server, \
    srtp_use \
    FROM telephones;";

static QString sSqlTableTelInsert = "\
INSERT INTO telephones (\
description, \
name, \
domain, \
username, \
password, \
active, \
proxy, \
should_register_startup, \
should_subscribe_presence, \
should_publish_presence, \
should_use_blf, \
should_disable_ringback_tone, \
custom_ringtone, \
transport, \
subscription_expiry_delay, \
keep_alive_expiry_delay, \
registration_expiry_delay, \
use_stun, \
stun_server, \
srtp_use \
) VALUES (\
:description, \
:name, \
:domain, \
:username, \
:password, \
:active, \
:proxy, \
:should_register_startup, \
:should_subscribe_presence, \
:should_publish_presence, \
:should_use_blf, \
:should_disable_ringback_tone, \
:custom_ringtone, \
:transport, \
:subscription_expiry_delay, \
:keep_alive_expiry_delay, \
:registration_expiry_delay, \
:use_stun, \
:stun_server, \
:srtp_use \
)";

static QString sSqlTableTelActiveCount = "\
    SELECT COUNT(id) FROM telephones WHERE active = 1;";

static QString sSqlTableTelUpdate = "\
UPDATE telephones \
SET \
description=:description, \
name=:name, \
domain=:domain, \
username=:username, \
password=:password, \
active=:active, \
proxy=:proxy, \
should_register_startup=:should_register_startup, \
should_subscribe_presence=:should_subscribe_presence, \
should_publish_presence=:should_publish_presence, \
should_use_blf=:should_use_blf, \
should_disable_ringback_tone=:should_disable_ringback_tone, \
custom_ringtone=:custom_ringtone, \
transport=:transport, \
subscription_expiry_delay=:subscription_expiry_delay, \
keep_alive_expiry_delay=:keep_alive_expiry_delay, \
registration_expiry_delay=:registration_expiry_delay, \
use_stun=:use_stun, \
stun_server=:stun_server, \
srtp_use=:srtp_use \
WHERE id=:id";

static QString sSqlTableTelDelete = "\
DELETE FROM telephones WHERE id=:id";

static QString sSqlTableGlbCreate = "\
CREATE TABLE IF NOT EXISTS config (\
    id INTEGER PRIMARY KEY AUTOINCREMENT, \
    active_codecs TEXT \
)";

static QString sSqlTableGlbInsert = "\
INSERT INTO config (id, active_codecs) \
VALUES \
    (1, :active_codecs)";

static QString sSqlTableGlbSelect = "\
SELECT active_codecs FROM config;";

static QString sSqlTableGlbUpdate = "\
UPDATE config \
SET \
    active_codecs=:active_codecs \
WHERE id=1";


QString Global::sqlDriverName(){
    return sDDriver;
}

QString Global::sqlTableTelCreate(){
    return sSqlTableTelCreate;
}
QString Global::sqlTableTelSelect(){
    return sSqlTableTelSelect;
}
QString Global::sqlTableTelInsert(){
    return sSqlTableTelInsert;
}
QString Global::sqlTableTelActiveCount(){
    return sSqlTableTelActiveCount;
}
QString Global::sqlTableTelUpdate(){
    return sSqlTableTelUpdate;
}
QString Global::sqlTableTelDelete(){
    return sSqlTableTelDelete;
}
QString Global::sqlTableGlbCreate(){
    return sSqlTableGlbCreate;
}
QString Global::sqlTableGlbInsert(){
    return sSqlTableGlbInsert;
}
QString Global::sqlTableGlbSelect(){
    return sSqlTableGlbSelect;
}
QString Global::sqlTableGlbUpdate(){
    return sSqlTableGlbUpdate;
}
