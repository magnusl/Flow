#ifndef _FLOW_TOKEN_H_
#define _FLOW_TOKEN_H_

#include <iostream>
#include "pool.h"

namespace flow
{
    typedef enum {
        T_IDENT,
        /** keywords */
        T_KEYWORD_IN,
        T_KEYWORD_OUT,
        T_KEYWORD_NODE,
        T_KEYWORD_QUERY,
        T_KEYWORD_EVENT,
        T_KEYWORD_TRUE,
        T_KEYWORD_FALSE,
        /** types */
        T_TYPE_FLOAT,
        T_TYPE_BOOL,
        T_INTEGER,
        T_REAL,
        T_ASSIGN,
        /** single character tokens */
        T_QUESTION,
        T_COMMA,
        T_SEMICOLON,
        T_COLON,
        T_DOT,
        T_LEFT_PAREN,               /* ( */
        T_RIGHT_PAREN,              /* ) */
        T_LEFT_SQUARE_BRACKET,      /* [ */
        T_RIGHT_SQUARE_BRACKET,     /* ] */
        T_LEFT_CURLY_BRACKET,       /* { */
        T_RIGHT_CURLY_BRACKET,       /* } */
        /** operators */
        T_ADD,
        T_SUB,
        T_MUL,
        T_DIV,
        T_EQUAL,
        T_LESS,
        T_GRT,
        T_LEQ,
        T_GEQ,
        /** Parse error */
        T_FAILURE,
        /** eof */
        T_EOF
    } Symbol_t;

    class SymbolTable
    {
    public:
        SymbolTable() : m_pRoot(nullptr)
        {
        }

        typedef size_t SymIndex;

        SymIndex Insert(const char *, bool modify = true);
        const char * Retrive(SymIndex index)    const;

    protected:
        struct Node {
            Node() : m_pStr(nullptr), m_nOffset(0), m_pLeft(nullptr), m_pRight(nullptr)
            {
            }
            const char * m_pStr;
            size_t       m_nOffset;

            Node * m_pLeft;
            Node * m_pRight;
        } * m_pRoot;

        Pool<char, 64>  m_StringPool;
    };

    struct PositionInfo
    {
        PositionInfo() : Row(0), Col(0)
        {
        }

        size_t      Row;
        size_t      Col;
    };

    class Tokenizer
    {
    public:
        Tokenizer(std::istream & is) : m_Stream(is), m_State(TOK_INITIAL), m_HasPeeked(false)
        {
        }

        Symbol_t    GetSym();
        Symbol_t    Peek();

        SymbolTable::SymIndex       SymIndex() const            {return u.m_SymbolIndex;}
        float                       RealValue() const           {return u.m_RealValue;}
        int                         IntValue() const            {return u.m_IntValue;}
        flow::SymbolTable &         SymbolTable()               {return m_SymbolTable;}
        const PositionInfo &        Position() const            {return m_Position;}
        
        const char *                Lookup(SymbolTable::SymIndex sym) const  
        {
            return m_SymbolTable.Retrive(sym);
        }

        static const char *         GetTokenString(Symbol_t);

    protected:
        Tokenizer(const Tokenizer &);
        Tokenizer & operator=(const Tokenizer &);

        enum {
            TOK_INITIAL,
            TOK_NUMERIC,
            TOK_IDENT,
            TOK_FLOAT
        } m_State;

        bool Peek(char &);
        bool GetChar(char &);

        Symbol_t        m_NextSym;
        bool            m_HasPeeked;

        std::istream &                  m_Stream;
        flow::SymbolTable               m_SymbolTable;
        PositionInfo                    m_Position;

        union {
            SymbolTable::SymIndex           m_SymbolIndex;
            float                           m_RealValue;
            int                             m_IntValue;
        } u;
    };
} // namespace flow

#endif