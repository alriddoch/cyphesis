// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000 Alistair Riddoch

#include <Atlas/Message/Object.h>
#include <Atlas/Objects/Operation/Login.h>
#include <Atlas/Objects/Operation/Look.h>

using Atlas::Objects::Operation::Look;

#include "Entity.h"
#include "MemMap.h"
#include "Script.h"
#include <modules/Location.h>
#include <common/debug.h>

static const bool debug_flag = false;

inline Entity * MemMap::add_object(Entity * object)
{
    debug( cout << "MemMap::add_object " << object << " " << object->fullid
         << endl << flush;);
    if (object != NULL) {
        things[object->fullid] = object;
    }

    debug( cout << things[object->fullid] << endl << flush;);
    debug( cout << this << endl << flush;);
    list<string>::const_iterator I;
    for(I = add_hooks.begin(); I != add_hooks.end(); I++) {
        script->hook(*I, object);
    }
    return object;
}

inline RootOperation * MemMap::look_id()
{
    debug( cout << "MemMap::look_id" << endl << flush;);
    if (additions_by_id.size() != 0) {
        string id = additions_by_id.front();
        additions_by_id.pop_front();
        Look * l = new Look();
        *l = Look::Instantiate();
        //Object::MapType m;
        //m["id"] = Object(id);
        //l->SetArgs(Object::ListType(1, Object(m)));
        l->SetTo(id);
        return l;
    }
    return(NULL);
}

inline Entity * MemMap::add_id(const string & id)
{
    if (id.size() == 0) { return NULL; }
    debug( cout << "MemMap::add_id" << endl << flush;);
    additions_by_id.push_back(id);
    Object::MapType m;
    m["id"] = Object(string(id));
    Object obj(m);
    return add(obj);
}

inline Entity * MemMap::add(const Object & entity)
{
    debug( cout << "MemMap::add" << endl << flush;);
    if (!entity.IsMap()) {
        return NULL;
    }
    Object::MapType entmap = entity.AsMap();
    if ((entmap.find("id") == entmap.end()) ||
        (entmap["id"].AsString().size() == 0)) {
        return NULL;
    }
    if (get(entmap["id"].AsString())) {
        return update(entity);
    }
    Entity * thing = new Entity;
    thing->fullid = entmap["id"].AsString();
    if (entmap.find("name") != entmap.end() && entmap["name"].IsString()) {
        thing->name = entmap["name"].AsString();
    }
    if (entmap.find("type") != entmap.end() && entmap["type"].IsString()) {
        thing->type = entmap["type"].AsString();
    }
    thing->merge(entmap);
    if (entmap.find("loc") != entmap.end()) {
        get_add(entmap["loc"].AsString());
    }
    thing->getLocation(entmap, things);
    return add_object(thing);
}

inline void MemMap::_delete(const string & id)
{
    if (id.size() == 0) { return; }
    if (things.find(id) != things.end()) {
        Entity * obj = (Entity*)things[id];
        things.erase(id);
        list<string>::const_iterator I;
        for(I = delete_hooks.begin(); I != delete_hooks.end(); I++) {
            script->hook(*I, obj);
        }
        delete obj;
    }
}

inline Entity * MemMap::get(const string & id)
{
    debug( cout << "MemMap::get" << endl << flush;);
    if (id.size() == 0) { return NULL; }
    if (things.find(id) != things.end()) {
        return (Entity*)things[id];
    }
    return(NULL);
}

inline Entity * MemMap::get_add(const string & id)
{
    debug( cout << "MemMap::get_add" << endl << flush;);
    if (id.size() == 0) { return NULL; }
    Entity * obj = MemMap::get(id);
    if (obj != NULL) {
        return obj;
    }
    return add_id(id);
}

inline Entity * MemMap::update(const Object & entity)
{
    debug( cout << "MemMap::update" << endl << flush;);
    if (!entity.IsMap()) {
        return NULL;
    }
    Object::MapType entmap = entity.AsMap();
    if (entmap.find("id") == entmap.end()) {
        return NULL;
    }
    string & id = entmap["id"].AsString();
    if (id.size() == 0) {
        return NULL;
    }
    debug( cout << " updating " << id << endl << flush;);
    if (things.find(id) == things.end()) {
        return add(entity);
    }
    debug( cout << " " << id << " has already been spotted" << endl << flush;);
    Entity * thing = (Entity*)things[id];
    debug( cout << " got " << thing << endl << flush;);
    // I am not sure what the deal is with all the "needTrueValue stuff
    // below yet. FIXME find out exactly what is required.
    if (entmap.find("name") != entmap.end() && entmap["name"].IsString()) {
        thing->name = entmap["name"].AsString();
    }
    if (entmap.find("type") != entmap.end() && entmap["type"].IsString()) {
        thing->type = entmap["type"].AsString();
    }
    debug( cout << " got " << thing << endl << flush;);
    thing->merge(entmap);
    thing->getLocation(entmap,things);
    //needTrueValue=["type","contains","instance","id","location","stamp"];
    //for (/*(key,value) in entity.__dict__.items()*/) {
        //if (value or not key in needTrueValue) {
            //setattr(obj,key,value);
        //}
    //}
    list<string>::const_iterator I;
    for(I = update_hooks.begin(); I != update_hooks.end(); I++) {
        script->hook(*I, thing);
    }
    //for (/*hook in MemMap::update_hooks*/) {
        //hook(obj);
    //}
    return thing;
}
