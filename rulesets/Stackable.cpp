// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000,2001 Alistair Riddoch

// A stackable object, ie one which can represent multiple object of the
// same type. Used for things like coins.

#include "Stackable.h"

#include "Script.h"

#include <Atlas/Objects/Operation/Combine.h>
#include <Atlas/Objects/Operation/Divide.h>
#include <Atlas/Objects/Operation/Delete.h>

Stackable::Stackable(const std::string & id) : Stackable_parent(id),
                                               m_num(1)
{
    subscribe("combine", OP_COMBINE);
    subscribe("divide", OP_DIVIDE);
}

Stackable::~Stackable()
{
}

bool Stackable::get(const std::string & aname, Element & attr) const
{
    if (aname == "num") {
        attr = m_num;
        return true;
    }
    return Stackable_parent::get(aname, attr);
}

void Stackable::set(const std::string & aname, const Element & attr)
{
    if ((aname == "num") && attr.isInt()) {
        m_num = attr.asInt();
    } else {
        Stackable_parent::set(aname, attr);
    }
}

void Stackable::addToObject(Element::MapType & omap) const
{
    if (m_num != 1) {
        omap["num"] = m_num;
    }
    Stackable_parent::addToObject(omap);
}

OpVector Stackable::CombineOperation(const Combine & op)
{
    OpVector res;
    if (m_script->Operation("combine", op, res) != 0) {
        return res;
    }
    const Element::ListType & args = op.getArgs();
    for(Element::ListType::const_iterator I = args.begin(); I!= args.end(); I++) {
        const std::string & id = I->asMap().find("id")->second.asString();
        if (id == getId()) { continue; }
        Entity * ent = m_world->getObject(id);
        if (ent == NULL) { continue; }
        Stackable * obj = dynamic_cast<Stackable *>(ent);
        if (obj == NULL) { continue; }
        if (obj->m_type != m_type) { continue; }
        m_num = m_num + obj->m_num;

        Delete * d = new Delete(Delete::Instantiate());
        Element::MapType dent;
        dent["id"] = id;
        d->setTo(id);
        d->setArgs(Element::ListType(1,dent));
        res.push_back(d);
    }
    return res;
    // Currently does not send sight ops, as the Sight ops for this type of
    // thing have not been discussed
}

OpVector Stackable::DivideOperation(const Divide & op)
{
    OpVector res;
    if (m_script->Operation("divide", op, res) != 0) {
        return res;
    }
    const Element::ListType & args = op.getArgs();
    for(Element::ListType::const_iterator I = args.begin(); I!=args.end(); I++) {
        const Element::MapType & ent = I->asMap();
        int new_num = 1;
        Element::MapType::const_iterator J = ent.find("num");
        if (J != ent.end()) {
            if (J->second.isInt()) { new_num = J->second.asInt(); }
        }
        if (m_num <= new_num) { continue; }
        
        Element::MapType new_ent;
        Element::ListType parents(1,m_type);
        new_ent["parents"] = parents;
        new_ent["num"] = new_num;
        Create * c = new Create( Create::Instantiate());
        c->setArgs(Element::ListType(1,new_ent));
        c->setTo(getId());
        res.push_back(c);
    }
    return res;
    // Currently does not send sight ops, as the Sight ops for this type of
    // thing have not been discussed
}
