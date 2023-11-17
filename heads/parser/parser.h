/*
    MetaReal Compiler version 1.0.0
*/

#ifndef __MR_PARSER__
#define __MR_PARSER__

#include <parser/node.h>
#include <lexer/token.h>
#include <error/error.h>

struct __PARSE_RES_T /**< Result of the parse function */
{
    node_t *nodes;              /**< List of the generated nodes */
    uint64_t size;              /**< Size of the nodes list*/

    invalid_syntax_t error;     /**< Invalid syntax error */
};
typedef struct __PARSE_RES_T parse_res_t; /**< Result of the parse function */

/**
 * \fn parse_res_t parse(token_t *tokens)
 * Gets the list of tokens generated by the lexer and processes it according to the MetaReal grammar. \n
 * Each code block is stored inside one node in the nodes list.
 * \param tokens List of tokens generated by the lexer
 * \return
 * The result of the parsing as a list of nodes. \n
 * If there was an invalid syntax error, the error is returned with a NULL nodes list.
*/
parse_res_t parse(token_t *tokens);

#endif /* __MR_PARSER__ */
