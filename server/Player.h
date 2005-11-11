// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000,2001 Alistair Riddoch

#ifndef SERVER_PLAYER_H
#define SERVER_PLAYER_H

#include "Account.h"

/// \brief This is a class for handling users who are regular players
class Player : public Account {
  protected:
    virtual int characterError(const Operation &,
                               const Atlas::Objects::Entity::RootEntity &,
                               OpVector &) const;
  public:
    Player(Connection * conn, const std::string & username,
                              const std::string & passwd,
                              const std::string & id, long intId);
    virtual ~Player();

    virtual const char * getType() const;

    virtual void addToMessage(Atlas::Message::MapType &) const;
    virtual void addToEntity(const Atlas::Objects::Entity::RootEntity &) const;

    static std::set<std::string> playableTypes;
};

#endif // SERVER_PLAYER_H
