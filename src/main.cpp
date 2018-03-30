#include "parser.h"

#include <iostream>
#include <string>

const char * FlowDefinition =
    "node SoundNode\n" 
    "{\n"
    "   in event Play;\n"
    "   in event Stop;\n"
    "   out event Playing;\n"
    "   out event Stopped;\n"
    "   bool boolean_value;\n"
    "   float float_value = 1;\n"
    "}\n"
    "query Test\n"
    "{\n"
    "   out event Status;\n"
    "   out event Play;\n"
    "   out bool boolean_value;\n"
    "}\n";

int main()
{
    flow::FlowDocument  Document;
    flow::Parser        parser;

    if (!parser.Parse(FlowDefinition, Document)) {
        std::cout << parser.GetErrorString() << std::endl;
        return -1;
    }

    for(auto it = Document.Nodes.begin(); it != Document.Nodes.end(); it++) {
        std::cout << "### Node: " << parser.GetString(it->NameIndex) << std::endl;
        for(auto nit = it->Variables.begin(); nit != it->Variables.end(); nit++) {
            std::cout <<"\tVariable: " << parser.GetString(nit->NameIndex) << std::endl;
        }
        for(auto nit = it->Events.begin(); nit != it->Events.end(); nit++) {
            std::cout <<"\tEvent: " << parser.GetString(nit->NameIndex) << std::endl;
        }
    }

    for(auto it = Document.Queries.begin(); it != Document.Queries.end(); it++) {
        std::cout << "### Query: " << parser.GetString(it->NameIndex) << std::endl;
        for(auto nit = it->Variables.begin(); nit != it->Variables.end(); nit++) {
            std::cout <<"\tVariable: " << parser.GetString(nit->NameIndex) << std::endl;
        }
        for(auto nit = it->Events.begin(); nit != it->Events.end(); nit++) {
            std::cout <<"\tEvent: " << parser.GetString(nit->NameIndex) << std::endl;
        }
    }


    return 0;
}