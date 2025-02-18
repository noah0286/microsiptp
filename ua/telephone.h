#pragma once

#include <QString>

struct Telephone {
    QString id;
    QString description;
    QString name;
    QString domain;
    QString username;
    QString password;
    int active;
    QString proxy;

    /* prefs */
    int should_register_startup;
    int should_subscribe_presence;
    int should_publish_presence;
    int should_use_blf;
    int should_disable_ringback_tone;
    QString custom_ringtone;

    int transport;
    int subscription_expiry_delay;
    int keep_alive_expiry_delay;
    int registration_expiry_delay;
    int use_stun;
    QString stun_server;
};
