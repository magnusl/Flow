#include "parser.h"
#include "token.h"

#include <sstream>

namespace flow
{
    /** 
     * \brief   Parses a document containing flow definitions.
     * \param   a_String    The document as a string.
     * \param   a_Document  The document as a string.
     *
     * \return  true if the document was parsed successfully, or false otherwise.
     */
    bool Parser::Parse(const std::string & a_String, FlowDocument & a_Document)
    {
        m_ErrorString = "";
        std::stringstream ss(a_String);
        flow::Tokenizer token(ss);
        return ParseDocument(token, a_Document);
    }

    /**
     * \brief   Internal implementation of the parsing that uses a tokenizer.
     */
    bool Parser::ParseDocument(flow::Tokenizer & a_Tokenizer, FlowDocument & a_Document)
    {
        Symbol_t sym = a_Tokenizer.Peek();
        while( sym != flow::T_EOF ) 
        {
            if (sym == flow::T_KEYWORD_NODE) {
                FlowNode node;
                if (!ParseNode(a_Tokenizer, node)) {
                    return false;
                }
                a_Document.Nodes.push_back(node);
            } else if (sym == flow::T_KEYWORD_QUERY) {
                FlowQuery query;
                if (!ParseQuery(a_Tokenizer, query)) {
                    return false;
                }
                a_Document.Queries.push_back(query);
            } else {
                Unexpected(sym, a_Tokenizer.Position());
                return false;
            }
            sym = a_Tokenizer.Peek();
        }
        return true;
    }

    /**
     * \brief   Parses a flow node definition.
     */
    bool Parser::ParseNode(flow::Tokenizer & a_Tokenizer, FlowNode & a_Node)
    {
        if (!Expect(T_KEYWORD_NODE, a_Tokenizer)) {
            return false;
        }

        if (!Expect(T_IDENT, a_Tokenizer)) {
            return false;
        }

        if (!InsertName(a_Tokenizer.Lookup(a_Tokenizer.SymIndex()), &a_Node.NameIndex)) {
            return false;
        }

        if (!Expect(T_LEFT_CURLY_BRACKET, a_Tokenizer)) {
            return false;
        }

        Symbol_t prefix = a_Tokenizer.Peek();

        for(;;) {
            if ((prefix == flow::T_TYPE_FLOAT) || (prefix == flow::T_TYPE_BOOL)) {
                /** Variable declaration without prefix */
                FlowVariable variable;
                if (!ParseVariable(a_Tokenizer, variable)) {
                    return false;
                }
                a_Node.Variables.push_back(variable);
            } else if ((prefix == flow::T_KEYWORD_IN) || (prefix == flow::T_KEYWORD_OUT)) {
                a_Tokenizer.GetSym();   // consume.
                Symbol_t sym = a_Tokenizer.Peek();
                if ((sym == flow::T_TYPE_BOOL) || (sym == flow::T_TYPE_FLOAT)) {
                    FlowVariable variable;
                    if (!ParseVariable(a_Tokenizer, variable)) {
                        return false;
                    }
                    variable.HasDirection = 1;
                    variable.Direction = (prefix == flow::T_KEYWORD_IN) ? flow::FlowEvent::EVENT_IN : flow::FlowEvent::EVENT_OUT;
                    a_Node.Variables.push_back(variable);
                } else {
                    // should be a event.
                    FlowEvent ev;
                    if (!ParseEvent(a_Tokenizer, ev)) {
                        return false;
                    }
                    ev.Direction = (prefix == flow::T_KEYWORD_IN) ? flow::FlowEvent::EVENT_IN : flow::FlowEvent::EVENT_OUT;
                    a_Node.Events.push_back(ev);
                }
            } else {
                break;
            }
            prefix = a_Tokenizer.Peek();
        }

        if (!Expect(T_RIGHT_CURLY_BRACKET, a_Tokenizer)) {
            return false;
        }
        return true;
    }

    /**
     * \brief   Parses a variable declaration.
     */
    bool Parser::ParseVariable(flow::Tokenizer & a_Tokenizer, FlowVariable & a_Variable)
    {
        Symbol_t sym = a_Tokenizer.GetSym();
        if (sym == flow::T_TYPE_FLOAT) {
            a_Variable.Type = FlowVariable::TYPE_FLOAT;
            // should be followed by a name.
            if (!Expect(flow::T_IDENT, a_Tokenizer)) {
                return false;
            }

            if (!InsertName(a_Tokenizer.Lookup(a_Tokenizer.SymIndex()), &a_Variable.NameIndex)) {
                return false;
            }

            sym = a_Tokenizer.Peek();   
            if (sym == flow::T_ASSIGN) {
                a_Tokenizer.GetSym();
                a_Variable.HasDefaultValue = 1;     // has a default value.

                sym = a_Tokenizer.GetSym();
                if (sym == T_REAL) {
                    a_Variable.DefaultValue.fValue = a_Tokenizer.RealValue();
                } else if (sym == T_INTEGER) {
                    a_Variable.DefaultValue.fValue = static_cast<float>(a_Tokenizer.IntValue());
                } else {
                    Unexpected(sym, a_Tokenizer.Position());
                }
            } else {
                a_Variable.HasDefaultValue = 0;
            }
            // terminated with a semicolon
            if (!Expect(T_SEMICOLON, a_Tokenizer)) {
                return false;
            }
        } else if (sym == flow::T_TYPE_BOOL) {
            a_Variable.Type = FlowVariable::TYPE_BOOL;
            // should be followed by a name.
            if (!Expect(flow::T_IDENT, a_Tokenizer)) {
                return false;
            }

            if (!InsertName(a_Tokenizer.Lookup(a_Tokenizer.SymIndex()), &a_Variable.NameIndex)) {
                return false;
            }

            sym = a_Tokenizer.Peek();   
            if (sym == flow::T_ASSIGN) {
                a_Tokenizer.GetSym();
                a_Variable.HasDefaultValue = 1;     // has a default value.

                sym = a_Tokenizer.GetSym();
                if (sym == T_KEYWORD_TRUE) {
                    a_Variable.DefaultValue.bValue = true;
                } else if(sym == T_KEYWORD_FALSE) {
                    a_Variable.DefaultValue.bValue = false;
                } else {
                    Unexpected(sym, a_Tokenizer.Position());
                    return false;
                }
            } else {
                a_Variable.HasDefaultValue = 0;
            }
            // terminated with a semicolon
            if (!Expect(T_SEMICOLON, a_Tokenizer)) {
                return false;
            }
        } else {
            Unexpected(sym, a_Tokenizer.Position());
            return false;
        }
        return true;
    }

    /**
     * \brief    A flow query can only contain output variables and events.
     */
    bool Parser::ParseQuery(flow::Tokenizer & a_Tokenizer, FlowQuery & a_Query)
    {
        if (!Expect(T_KEYWORD_QUERY, a_Tokenizer)) {
            return false;
        }

        if (!Expect(T_IDENT, a_Tokenizer)) {
            return false;
        }

        if (!InsertName(a_Tokenizer.Lookup(a_Tokenizer.SymIndex()), &a_Query.NameIndex)) {
            return false;
        }

        if (!Expect(T_LEFT_CURLY_BRACKET, a_Tokenizer)) {
            return false;
        }

        Symbol_t prefix = a_Tokenizer.Peek();

        while(prefix == T_KEYWORD_OUT) {
            a_Tokenizer.GetSym();   // consume 'out'
            Symbol_t sym = a_Tokenizer.Peek();
            if (sym == T_KEYWORD_EVENT) {
                flow::FlowEvent ev;
                if (!ParseEvent(a_Tokenizer, ev)) {
                    return false;
                }
                ev.Direction = FlowEvent::EVENT_OUT;
                a_Query.Events.push_back(ev);
            } else {
                flow::FlowVariable var;
                if (!ParseVariable(a_Tokenizer, var)) {
                    return false;
                }
                var.HasDirection    = 1;
                var.Direction       = FlowEvent::EVENT_OUT;
                a_Query.Variables.push_back(var);
            }
            prefix = a_Tokenizer.Peek();
        }
        if (!Expect(T_RIGHT_CURLY_BRACKET, a_Tokenizer)) {
            return false;
        }
        return true;
    }

    /**
     * \brief   Parses a flow event declaration.
     */
    bool Parser::ParseEvent(flow::Tokenizer & a_Tokenizer, FlowEvent & a_Event)
    {
        if (!Expect(T_KEYWORD_EVENT, a_Tokenizer)) {    // should start with event.
            return false;
        }

        if (!Expect(T_IDENT, a_Tokenizer)) {
            return false;
        }

        if (!InsertName(a_Tokenizer.Lookup(a_Tokenizer.SymIndex()), &a_Event.NameIndex)) {
            return false;
        }

        if (!Expect(T_SEMICOLON, a_Tokenizer)) {
            return false;
        }

        return true;
    }

    std::ostream & operator << (std::ostream & os, const PositionInfo & pos) 
    {
        os << "(Ln: " << pos.Row << ", Col: " << pos.Col << ")";
        return os;
    }

    bool Parser::Expect(Symbol_t a_Expected, flow::Tokenizer & a_Tokenizer)
    {
        Symbol_t actual = a_Tokenizer.GetSym();
        if (actual != a_Expected) {
            std::stringstream err;
            err << "EXPECTED " << flow::Tokenizer::GetTokenString(a_Expected) << " at " << a_Tokenizer.Position() 
                << ", actual " <<  flow::Tokenizer::GetTokenString(actual);
            m_ErrorString = err.str();
            return false;
        }
        return true;
    }

    void Parser::Unexpected(Symbol_t a_Sym, const flow::PositionInfo & a_Position)
    {
        std::stringstream err;
        err << "UNEXPECTED " << flow::Tokenizer::GetTokenString(a_Sym) << " at " << a_Position;
        m_ErrorString = err.str();
    }

    bool Parser::InsertName(const char * a_String, size_t * a_NameIndex)
    {
        if (!a_String || !a_NameIndex) {
            return false;
        }

        *a_NameIndex = m_StringPool.Insert(a_String, strlen(a_String) + 1); // remember the null terminator
        return true;
    }

    const std::string & Parser::GetErrorString() const 
    {
        return m_ErrorString;
    }

    const char * Parser::GetString(size_t a_Index) const
    {
        return m_StringPool.GetPointer(a_Index);
    }
}