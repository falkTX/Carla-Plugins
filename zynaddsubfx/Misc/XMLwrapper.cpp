/*
  ZynAddSubFX - a software synthesizer

  XMLwrapper.cpp - XML wrapper
  Copyright (C) 2003-2005 Nasca Octavian Paul
  Copyright (C) 2009-2009 Mark McCurry
  Author: Nasca Octavian Paul
          Mark McCurry

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
*/

#include "XMLwrapper.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>
#include <zlib.h>
#include <iostream>
#include <sstream>

#include "globals.h"
#include "Util.h"

using namespace std;

namespace zyncarla {

int  xml_k   = 0;
bool verbose = false;

const char *XMLwrapper_whitespace_callback(mxml_node_t *node, int where)
{
    const char *name = mxmlGetElement(node);

    if((where == MXML_WS_BEFORE_OPEN) && (!strcmp(name, "?xml")))
        return NULL;
    if((where == MXML_WS_BEFORE_CLOSE) && (!strcmp(name, "string")))
        return NULL;

    if((where == MXML_WS_BEFORE_OPEN) || (where == MXML_WS_BEFORE_CLOSE))
        /*	const char *tmp=mxmlGetElement(node);
            if (tmp!=NULL) {
                if ((strstr(tmp,"par")!=tmp)&&(strstr(tmp,"string")!=tmp)) {
                printf("%s ",tmp);
                if (where==MXML_WS_BEFORE_OPEN) xml_k++;
                if (where==MXML_WS_BEFORE_CLOSE) xml_k--;
                if (xml_k>=STACKSIZE) xml_k=STACKSIZE-1;
                if (xml_k<0) xml_k=0;
                printf("%d\n",xml_k);
                printf("\n");
                };

            };
            int i=0;
            for (i=1;i<xml_k;i++) tabs[i]='\t';
            tabs[0]='\n';tabs[i+1]='\0';
            if (where==MXML_WS_BEFORE_OPEN) return(tabs);
                else return("\n");
        */
        return "\n";
    ;

    return 0;
}

//temporary const overload of mxmlFindElement
const mxml_node_t *mxmlFindElement(const mxml_node_t *node,
                                   const mxml_node_t *top,
                                   const char *name,
                                   const char *attr,
                                   const char *value,
                                   int descend)
{
    return const_cast<const mxml_node_t *>(mxmlFindElement(
                                               const_cast<mxml_node_t *>(node),
                                               const_cast<mxml_node_t *>(top),
                                               name, attr, value, descend));
}

//temporary const overload of mxmlElementGetAttr
const char *mxmlElementGetAttr(const mxml_node_t *node, const char *name)
{
    return mxmlElementGetAttr(const_cast<mxml_node_t *>(node), name);
}

XMLwrapper::XMLwrapper()
{
    minimal = true;

    node = tree = mxmlNewElement(MXML_NO_PARENT,
                                 "?xml version=\"1.0f\" encoding=\"UTF-8\"?");
    /*  for mxml 2.1f (and older)
        tree=mxmlNewElement(MXML_NO_PARENT,"?xml");
        mxmlElementSetAttr(tree,"version","1.0f");
        mxmlElementSetAttr(tree,"encoding","UTF-8");
    */

    mxml_node_t *doctype = mxmlNewElement(tree, "!DOCTYPE");
    mxmlElementSetAttr(doctype, "ZynAddSubFX-data", NULL);

    node = root = addparams("ZynAddSubFX-data", 4,
                            "version-major", stringFrom<int>(
                                version.get_major()).c_str(),
                            "version-minor", stringFrom<int>(
                                version.get_minor()).c_str(),
                            "version-revision",
                            stringFrom<int>(version.get_revision()).c_str(),
                            "ZynAddSubFX-author", "Nasca Octavian Paul");

    //make the empty branch that will contain the information parameters
    info = addparams("INFORMATION", 0);

    //save zynaddsubfx specifications
    beginbranch("BASE_PARAMETERS");
    addpar("max_midi_parts", NUM_MIDI_PARTS);
    addpar("max_kit_items_per_instrument", NUM_KIT_ITEMS);

    addpar("max_system_effects", NUM_SYS_EFX);
    addpar("max_insertion_effects", NUM_INS_EFX);
    addpar("max_instrument_effects", NUM_PART_EFX);

    addpar("max_addsynth_voices", NUM_VOICES);
    endbranch();
}

void
XMLwrapper::cleanup(void)
{
    if(tree)
        mxmlDelete(tree);

    /* make sure freed memory is not referenced */
    tree = 0;
    node = 0;
    root = 0;
}

XMLwrapper::~XMLwrapper()
{
    cleanup();
}

void XMLwrapper::setPadSynth(bool enabled)
{
    /**@bug this might create multiple nodes when only one is needed*/
    mxml_node_t *oldnode = node;
    node = info;
    //Info storing
    addparbool("PADsynth_used", enabled);
    node = oldnode;
}

bool XMLwrapper::hasPadSynth() const
{
    /**Right now this has a copied implementation of setparbool, so this should
     * be reworked as XMLwrapper evolves*/
    mxml_node_t *tmp = mxmlFindElement(tree,
                                       tree,
                                       "INFORMATION",
                                       NULL,
                                       NULL,
                                       MXML_DESCEND);

    mxml_node_t *parameter = mxmlFindElement(tmp,
                                             tmp,
                                             "par_bool",
                                             "name",
                                             "PADsynth_used",
                                             MXML_DESCEND_FIRST);
    if(parameter == NULL) //no information availiable
        return false;

    const char *strval = mxmlElementGetAttr(parameter, "value");
    if(strval == NULL) //no information available
        return false;

    if((strval[0] == 'Y') || (strval[0] == 'y'))
        return true;
    else
        return false;
}


/* SAVE XML members */

int XMLwrapper::saveXMLfile(const string &filename, int compression) const
{
    char *xmldata = getXMLdata();
    if(xmldata == NULL)
        return -2;

    int result      = dosavefile(filename.c_str(), compression, xmldata);

    free(xmldata);
    return result;
}

char *XMLwrapper::getXMLdata() const
{
    xml_k = 0;

    char *xmldata = mxmlSaveAllocString(tree, XMLwrapper_whitespace_callback);

    return xmldata;
}


int XMLwrapper::dosavefile(const char *filename,
                           int compression,
                           const char *xmldata) const
{
    if(compression == 0) {
        FILE *file;
        file = fopen(filename, "w");
        if(file == NULL)
            return -1;
        fputs(xmldata, file);
        fclose(file);
    }
    else {
        if(compression > 9)
            compression = 9;
        if(compression < 1)
            compression = 1;
        char options[10];
        snprintf(options, 10, "wb%d", compression);

        gzFile gzfile;
        gzfile = gzopen(filename, options);
        if(gzfile == NULL)
            return -1;
        gzputs(gzfile, xmldata);
        gzclose(gzfile);
    }

    return 0;
}



void XMLwrapper::addpar(const string &name, int val)
{
    addparams("par", 2, "name", name.c_str(), "value", stringFrom<int>(
                  val).c_str());
}

void XMLwrapper::addparreal(const string &name, float val)
{
    union { float in; uint32_t out; } convert;
    char buf[11];
    convert.in = val;
    sprintf(buf, "0x%.8X", convert.out);
    addparams("par_real", 3, "name", name.c_str(), "value",
              stringFrom<float>(val).c_str(), "exact_value", buf);
}

void XMLwrapper::addparbool(const string &name, int val)
{
    if(val != 0)
        addparams("par_bool", 2, "name", name.c_str(), "value", "yes");
    else
        addparams("par_bool", 2, "name", name.c_str(), "value", "no");
}

void XMLwrapper::addparstr(const string &name, const string &val)
{
    mxml_node_t *element = mxmlNewElement(node, "string");
    mxmlElementSetAttr(element, "name", name.c_str());
    mxmlNewText(element, 0, val.c_str());
}


void XMLwrapper::beginbranch(const string &name)
{
    if(verbose)
        cout << "beginbranch()" << name << endl;
    node = addparams(name.c_str(), 0);
}

void XMLwrapper::beginbranch(const string &name, int id)
{
    if(verbose)
        cout << "beginbranch(" << id << ")" << name << endl;
    node = addparams(name.c_str(), 1, "id", stringFrom<int>(id).c_str());
}

void XMLwrapper::endbranch()
{
    auto parent = mxmlGetParent(node);
    if(verbose)
        cout << "endbranch()" << node << "-" << mxmlGetElement(node)
             << " To "
             << parent << "-" << mxmlGetElement(parent) << endl;
    node = parent;
}


//workaround for memory leak
const char *trimLeadingWhite(const char *c)
{
    while(isspace(*c))
        ++c;
    return c;
}

/* LOAD XML members */

int XMLwrapper::loadXMLfile(const string &filename)
{
    cleanup();

    const char *xmldata = doloadfile(filename);
    if(xmldata == NULL)
        return -1;  //the file could not be loaded or uncompressed

    root = tree = mxmlLoadString(NULL, trimLeadingWhite(
                                     xmldata), MXML_OPAQUE_CALLBACK);

    delete[] xmldata;

    if(tree == NULL)
        return -2;  //this is not XML

    node = root = mxmlFindElement(tree,
                                  tree,
                                  "ZynAddSubFX-data",
                                  NULL,
                                  NULL,
                                  MXML_DESCEND);
    if(root == NULL)
        return -3;  //the XML doesnt embbed zynaddsubfx data

    //fetch version information
    _fileversion.set_major(stringTo<int>(mxmlElementGetAttr(root, "version-major")));
    _fileversion.set_minor(stringTo<int>(mxmlElementGetAttr(root, "version-minor")));
    _fileversion.set_revision(
        stringTo<int>(mxmlElementGetAttr(root, "version-revision")));

    if(verbose)
        cout << "loadXMLfile() version: " << _fileversion << endl;

    return 0;
}


char *XMLwrapper::doloadfile(const string &filename) const
{
    char  *xmldata = NULL;
    gzFile gzfile  = gzopen(filename.c_str(), "rb");

    if(gzfile != NULL) { //The possibly compressed file opened
        stringstream strBuf;             //reading stream
        const int    bufSize = 500;      //fetch size
        char fetchBuf[bufSize + 1];      //fetch buffer
        int  read = 0;                   //chars read in last fetch

        fetchBuf[bufSize] = 0; //force null termination

        while(bufSize == (read = gzread(gzfile, fetchBuf, bufSize)))
            strBuf << fetchBuf;

        fetchBuf[read] = 0; //Truncate last partial read
        strBuf << fetchBuf;

        gzclose(gzfile);

        //Place data in output format
        string tmp = strBuf.str();
        xmldata = new char[tmp.size() + 1];
        strncpy(xmldata, tmp.c_str(), tmp.size() + 1);
    }

    return xmldata;
}

bool XMLwrapper::putXMLdata(const char *xmldata)
{
    cleanup();

    if(xmldata == NULL)
        return false;

    root = tree = mxmlLoadString(NULL, trimLeadingWhite(
                                     xmldata), MXML_OPAQUE_CALLBACK);
    if(tree == NULL)
        return false;

    node = root = mxmlFindElement(tree,
                                  tree,
                                  "ZynAddSubFX-data",
                                  NULL,
                                  NULL,
                                  MXML_DESCEND);
    if(root == NULL)
        return false;

    //fetch version information
    _fileversion.set_major(stringTo<int>(mxmlElementGetAttr(root, "version-major")));
    _fileversion.set_minor(stringTo<int>(mxmlElementGetAttr(root, "version-minor")));
    _fileversion.set_revision(
        stringTo<int>(mxmlElementGetAttr(root, "version-revision")));

    return true;
}



int XMLwrapper::enterbranch(const string &name)
{
    if(verbose)
        cout << "enterbranch() " << name << endl;
    mxml_node_t *tmp = mxmlFindElement(node, node,
                                       name.c_str(), NULL, NULL,
                                       MXML_DESCEND_FIRST);
    if(tmp == NULL)
        return 0;

    node = tmp;
    return 1;
}

int XMLwrapper::enterbranch(const string &name, int id)
{
    if(verbose)
        cout << "enterbranch(" << id << ") " << name << endl;
    mxml_node_t *tmp = mxmlFindElement(node, node,
                                       name.c_str(), "id", stringFrom<int>(
                                           id).c_str(), MXML_DESCEND_FIRST);
    if(tmp == NULL)
        return 0;

    node = tmp;
    return 1;
}


void XMLwrapper::exitbranch()
{
    auto parent = mxmlGetParent(node);
    if(verbose)
        cout << "exitbranch()" << node << "-" << mxmlGetElement(node)
             << " To "
             << parent << "-" << mxmlGetElement(parent) << endl;
    node = parent;
}


int XMLwrapper::getbranchid(int min, int max) const
{
    int id = stringTo<int>(mxmlElementGetAttr(node, "id"));
    if((min == 0) && (max == 0))
        return id;

    if(id < min)
        id = min;
    else
    if(id > max)
        id = max;

    return id;
}

int XMLwrapper::getpar(const string &name, int defaultpar, int min,
                       int max) const
{
    const mxml_node_t *tmp = mxmlFindElement(node,
                                             node,
                                             "par",
                                             "name",
                                             name.c_str(),
                                             MXML_DESCEND_FIRST);

    if(tmp == NULL)
        return defaultpar;

    const char *strval = mxmlElementGetAttr(tmp, "value");
    if(strval == NULL)
        return defaultpar;

    int val = stringTo<int>(strval);
    if(val < min)
        val = min;
    else
    if(val > max)
        val = max;

    return val;
}

int XMLwrapper::getpar127(const string &name, int defaultpar) const
{
    return getpar(name, defaultpar, 0, 127);
}

int XMLwrapper::getparbool(const string &name, int defaultpar) const
{
    const mxml_node_t *tmp = mxmlFindElement(node,
                                             node,
                                             "par_bool",
                                             "name",
                                             name.c_str(),
                                             MXML_DESCEND_FIRST);

    if(tmp == NULL)
        return defaultpar;

    const char *strval = mxmlElementGetAttr(tmp, "value");
    if(strval == NULL)
        return defaultpar;

    if((strval[0] == 'Y') || (strval[0] == 'y'))
        return 1;
    else
        return 0;
}

void XMLwrapper::getparstr(const string &name, char *par, int maxstrlen) const
{
    ZERO(par, maxstrlen);
    mxml_node_t *tmp = mxmlFindElement(node,
                                       node,
                                       "string",
                                       "name",
                                       name.c_str(),
                                       MXML_DESCEND_FIRST);

    if(tmp == NULL)
        return;
    mxml_node_t *child = mxmlGetFirstChild(tmp);
    if(child == NULL)
        return;
    if(mxmlGetType(child) == MXML_OPAQUE) {
        snprintf(par, maxstrlen, "%s", mxmlGetElement(child));
        return;
    }
    if(mxmlGetType(child) == MXML_TEXT) {
        const char *text = mxmlGetText(child, NULL);
        if (text != NULL) {
            snprintf(par, maxstrlen, "%s", text);
            return;
        }
    }
}

string XMLwrapper::getparstr(const string &name,
                             const std::string &defaultpar) const
{
    mxml_node_t *tmp = mxmlFindElement(node,
                                       node,
                                       "string",
                                       "name",
                                       name.c_str(),
                                       MXML_DESCEND_FIRST);

    if(tmp == NULL)
        return defaultpar;
    mxml_node_t *child = mxmlGetFirstChild(tmp);
    if(child == NULL)
        return defaultpar;

    if(mxmlGetType(child) == MXML_OPAQUE) {
        const char *name = mxmlGetElement(child);
        if(name != NULL)
            return name;
    }

    if(mxmlGetType(child) == MXML_TEXT) {
        const char *value = mxmlGetText(child, NULL);
        if(value != NULL)
            return value;
    }

    return defaultpar;
}

float XMLwrapper::getparreal(const char *name, float defaultpar) const
{
    const mxml_node_t *tmp = mxmlFindElement(node,
                                             node,
                                             "par_real",
                                             "name",
                                             name,
                                             MXML_DESCEND_FIRST);
    if(tmp == NULL)
        return defaultpar;

    const char *strval = mxmlElementGetAttr(tmp, "exact_value");
    if (strval != NULL) {
        union { float out; uint32_t in; } convert;
        sscanf(strval+2, "%x", &convert.in);
        return convert.out;
    }

    strval = mxmlElementGetAttr(tmp, "value");
    if(strval == NULL)
        return defaultpar;

    return stringTo<float>(strval);
}

float XMLwrapper::getparreal(const char *name,
                             float defaultpar,
                             float min,
                             float max) const
{
    float result = getparreal(name, defaultpar);

    if(result < min)
        result = min;
    else
    if(result > max)
        result = max;
    return result;
}


/** Private members **/

mxml_node_t *XMLwrapper::addparams(const char *name, unsigned int params,
                                   ...) const
{
    /**@todo make this function send out a good error message if something goes
     * wrong**/
    mxml_node_t *element = mxmlNewElement(node, name);

    if(params) {
        va_list variableList;
        va_start(variableList, params);

        const char *ParamName;
        const char *ParamValue;
        while(params--) {
            ParamName  = va_arg(variableList, const char *);
            ParamValue = va_arg(variableList, const char *);
            if(verbose)
                cout << "addparams()[" << params << "]=" << name << " "
                     << ParamName << "=\"" << ParamValue << "\"" << endl;
            mxmlElementSetAttr(element, ParamName, ParamValue);
        }
        va_end(variableList);
    }
    return element;
}

XmlNode::XmlNode(std::string name_)
    :name(name_)
{}

std::string &XmlNode::operator[](std::string name)
{
    //fetch an existing one
    for(auto &a:attrs)
        if(a.name == name)
            return a.value;

    //create a new one
    attrs.push_back({name, ""});
    return attrs[attrs.size()-1].value;
}

bool XmlNode::has(std::string name_)
{
    //fetch an existing one
    for(auto &a:attrs)
        if(a.name == name_)
            return true;
    return false;
}

void XMLwrapper::add(const XmlNode &node_)
{
    mxml_node_t *element = mxmlNewElement(node, node_.name.c_str());
    for(auto attr:node_.attrs)
        mxmlElementSetAttr(element, attr.name.c_str(),
                attr.value.c_str());
}

std::vector<XmlNode> XMLwrapper::getBranch(void) const
{
    std::vector<XmlNode> res;
    mxml_node_t *current = mxmlGetFirstChild(node);
    while(current) {
        if(mxmlGetType(current) == MXML_ELEMENT) {
            XmlNode n(mxmlGetElement(current));
            for(int i=0; i<mxmlElementGetAttrCount(current); ++i) {
                const char *name;
                const char *value = mxmlElementGetAttrByIndex(current, i, &name);
                n[name] = value;
            }
            res.push_back(n);
        }
        current = mxmlWalkNext(current, node, MXML_NO_DESCEND);
    }
    return res;
}

}
