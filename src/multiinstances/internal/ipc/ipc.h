/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_IPC_IPC_H
#define MU_IPC_IPC_H

#include <QString>
#include <QByteArray>

namespace mu::ipc {
static const QString SERVER_NAME("musescore-app-ipc");

using ID = QString;

static const int TIMEOUT_MSEC(500);

static const ID BROADCAST_ID("broadcast");
static const ID DIRECT_SOCKET_ID("socket");
static const ID SERVER_ID("server");

static const QByteArray ACK("ipc_ack");
static const QString IPC_("ipc_");
static const QString IPC_INIT("ipc_init");
static const QString IPC_WHOIS("ipc_whois");
static const QString IPC_METAINFO("ipc_metainfo");
static const QString IPC_PING("ipc_ping");

enum class MsgType {
    Undefined = 0,
    Notify,
    Request,
    Response
};

struct Msg
{
    QString destID;
    MsgType type = MsgType::Undefined;
    QString method;
    QStringList args;

        << << << < HEAD
        bool isValid() const { return type != MsgType::Undefined && !method.isEmpty(); }
        ==
        == ===bool isValid() const { return !method.isEmpty(); }
        >> >> >> > moved ipc server to separated thread
};

struct Meta
{
    QString id;
    bool isValid() const { return !id.isEmpty(); }
};

static void serialize(const Meta& meta, const Msg& msg, QByteArray& data);
static void deserialize(const QByteArray& data, Meta& meta, Msg& msg);

static QString socketErrorToString(int err);
}

#endif // MU_IPC_IPC_H
