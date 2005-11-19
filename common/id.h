// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2005 Alistair Riddoch

#ifndef COMMON_ID_H
#define COMMON_ID_H

#include <string>

long newId(std::string & id);
long integerId(const std::string & id);
int integerIdCheck(const std::string & id);

#endif // COMMON_ID_H
