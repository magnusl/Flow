#ifndef _FLOW_PARSER_H_
#define _FLOW_PARSER_H_

#include <list>
#include <vector>

#include "pool.h"
#include "token.h"

namespace flow
{
    struct FlowEvent
    {
        typedef enum {
            EVENT_IN,       /**< Input event */
            EVENT_OUT       /**< Ouput event */
        } EventDirection;

        EventDirection  Direction;
        size_t          NameIndex;  /**< Index into the parsers string pool */
    };

    struct FlowVariable
    {
        enum {
            TYPE_BOOL,      /**< boolean */
            TYPE_FLOAT,     /**< float */
        } Type;

        /** Only valid if HasDefaultValue is true */
        union {
            float   fValue;
            bool    bValue;
        } DefaultValue;

        unsigned char HasDefaultValue   : 1;    /**< Indicates if the variable has a default value */
        unsigned char HasDirection      : 1;    /**< Indicates if the variable has a direction prefix */

        FlowEvent::EventDirection   Direction;  /**< Only valid if HasDirection is true */      
        size_t                      NameIndex;  /**< Index into the parsers string pool */
    };

    /**
     * \brief A flow node.
     */
    struct FlowNode 
    {
        size_t                      NameIndex;  /**< Index into the parsers string pool */
        std::vector<FlowEvent>      Events;
        std::vector<FlowVariable>   Variables;
    };

    /**
     * \brief A flow query.
     */
    struct FlowQuery 
    {
        size_t                      NameIndex;  /**< Index into the parsers string pool */
        std::vector<FlowVariable>   Variables;
        std::vector<FlowEvent>      Events;
    };

    struct FlowDocument
    {
        std::vector< FlowNode >     Nodes;      /**< Nodes defined in the document */
        std::vector< FlowQuery >    Queries;    /**< Queries defined in the document */     
    };

    /**
     * \brief   Parses a document with flow definitions.
     */
    class Parser
    {
    public:
        /** 
         * \brief   Parses a document containing flow definitions.
         * \param   a_String    The document as a string.
         * \param   a_Document  The document as a string.
         *
         * \return  true if the document was parsed successfully, or false otherwise.
         */
        bool Parse(const std::string & a_String, FlowDocument & a_Document);
        /**
         * \brief   Returns a string that describes the last error encountered.
         */
        const std::string & GetErrorString() const;
        /**
         * \brief   Returns the string a the specified position.
         */
        const char * GetString(size_t) const;

    protected:

        bool ParseDocument(flow::Tokenizer & a_Tokenizer, FlowDocument & a_Document);
        bool ParseNode(flow::Tokenizer & a_Tokenizer, FlowNode & a_Node);
        bool ParseVariable(flow::Tokenizer & a_Tokenizer, FlowVariable & a_Variable);
        bool ParseEvent(flow::Tokenizer & a_Tokenizer, FlowEvent & a_Event);
        bool ParseQuery(flow::Tokenizer & a_Tokenizer, FlowQuery & a_Query);

        bool Expect(Symbol_t, flow::Tokenizer & tokenizer);
        void Unexpected(Symbol_t, const PositionInfo &);
        bool InsertName(const char *, size_t *);

        flow::Pool<char, 64>    m_StringPool;
        std::string             m_ErrorString;
    };
}

#endif