// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#error This file has been removed from the build

#include "common/Database.h"
#include "common/stringstream.h"

#include <Atlas/Objects/Decoder.h>
#include <Atlas/Codecs/XML.h>

#include <coal/isoloader.h>

#include <string>
#include <fstream>

class WorldBase : public Database {
  protected:
    WorldBase() { }

  public:
    static WorldBase * instance() {
        if (m_instance == NULL) {
            m_instance = new WorldBase();
        }
        return (WorldBase *)m_instance;
    }

    void storeInWorld(const Atlas::Message::Element::MapType & o, const char * key) {
        putObject(world_db, o, key);
    }
};

class TemplatesLoader : public Atlas::Message::DecoderBase {
    ifstream m_file;
    Atlas::Message::Element::MapType m_db;
    Atlas::Codecs::XML m_codec;
    int m_count;

    virtual void objectArrived(const Atlas::Message::Element& o) {
        Atlas::Message::Element obj(o);
        if (!obj.isMap()) {
            cerr << "ERROR: Non map object in file" << endl << flush;
            return;
        }
        Atlas::Message::Element::MapType & omap = obj.asMap();
        if (omap.find("graphic") == omap.end()) {
            cerr<<"WARNING: Template Object in file has no graphic. Not stored."
                << endl << flush;
            return;
        }
        m_count++;
        const std::string & id = omap["graphic"].asString();
        m_db[id] = obj;
    }
  public:
    TemplatesLoader(const std::string & filename) :
                m_file(filename.c_str()),
                m_codec((iostream&)m_file, this), m_count(0) {
    }

    void read() {
        while (!m_file.eof()) {
            m_codec.poll();
        }
    }

    void report() {
        std::cout << m_count << " template objects loaded."
                  << endl << flush;
    }

    Atlas::Message::Element::MapType & db() {
        return m_db;
    }

    const Atlas::Message::Element & get(const std::string & graphic) {
        return m_db[graphic];
    }
};

static void usage(char * progname)
{
    cout << "usage: " << progname << " filename" << endl << flush;
    return;
}

int main(int argc, char ** argv)
{
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    WorldBase * db = WorldBase::instance();
    db->initWorld(true);

    TemplatesLoader f("templates.xml");
    f.read();
    f.report();

    int id_no = 0;

    CoalDatabase map_database;
    CoalIsoLoader loader (map_database);
    if (!loader.LoadMap(argv[1])) {
        cout << "Map load failed." << endl << flush;
    } else {
        int count = map_database.GetObjectCount();
        for(int i = 0; i < count; i++) {
            CoalObject * object = (CoalObject*)map_database.GetObject (i);
            if (object != NULL) {
                const std::string & graphic = object->graphic->filename;
                size_t b = graphic.rfind('/') + 1;
                size_t e = graphic.rfind('.');
                std::string key(graphic, b, e - b);
                Atlas::Message::Element::MapType & tdb = f.db();
                Atlas::Message::Element::MapType::iterator t = tdb.find(key);
                if (t != tdb.end()) {
                    Atlas::Message::Element o = t->second;
                    Atlas::Message::Element::MapType & omap = o.asMap();
                    omap.erase("graphic");
                    stringstream id;
                    id << omap["name"].asString() << "_" << ++id_no << "_m";
                    omap["id"] = id.str();
                    omap["loc"] = "world_0";
                    Atlas::Message::Element::ListType c(3);
                    c[0] = object->anchor.GetX();
                    c[1] = object->anchor.GetY();
                    c[2] = object->anchor.GetZ();
                    omap["pos"] = c;
                    db->storeInWorld(omap, id.str().c_str());
                }
                // Get basename, lookup custumise and load into database
                cout << graphic << " " << key << endl;
            }
        }
    }
    
    db->shutdownWorld();
    delete db;
}
