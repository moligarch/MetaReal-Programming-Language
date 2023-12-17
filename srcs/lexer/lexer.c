/**
 * @file lexer.c
 * This file contains the function definitions of the "lexer.h" file.
*/

#include <lexer/lexer.h>
#include <alloc.h>
#include <consts.h>
#include <error.h>

/**
 * @def mr_lexer_skip_spaces(code, idx)
 * It skips the spaces (' ', '\t', and '\r') from the \a code and advances the \a idx.
 * @param code
 * The source code.
 * @param idx
 * The index of the current character (the \a idx field of the \a pos structure).
*/
#define mr_lexer_skip_spaces(code, idx)                                \
    while (code[idx] == ' ' || code[idx] == '\t' || code[idx] == '\r') \
        idx++

/**
 * @def mr_lexer_skip_newlines(code, idx, ln)
 * It skips the newlines ('\n') from the \a code and advances the \a idx and the \a ln.
 * @param code
 * The source code
 * @param idx
 * The index of the current character (the \a idx field of the \a pos structure).
 * @param ln
 * The current line of the \a code (the \a ln field of the \a pos structure).
*/
#define mr_lexer_skip_newlines(code, idx, ln) \
    while (code[idx] == '\n')                 \
    {                                         \
        idx++;                                \
        ln++;                                 \
    }

/**
 * @struct __MR_LEXER_MATCH_T
 * The input of the \a mr_lexer_match function.
 * @var __MR_LEXER_MATCH_T::flag
 * The flag indicates that the matching process succeeded or failed. \n
 * If the process succeeded, the flag will be 0. Otherwise, its value will be the error code.
 * @var __MR_LEXER_MATCH_T::tokens
 * The list of tokens.
 * @var __MR_LEXER_MATCH_T::size
 * The size of the \a tokens list.
 * @var __MR_LEXER_MATCH_T::alloc
 * The allocated size for the \a tokens list. \n
 * It indicates the maximum number of tokens that can be stored in the current \a tokens list.
 * @var __MR_LEXER_MATCH_T::code
 * The source code.
 * @var __MR_LEXER_MATCH_T::pos
 * The position indicating the index of the current character.
*/
struct __MR_LEXER_MATCH_T
{
    mr_byte_t flag;

    mr_token_t *tokens;
    mr_size_t size;
    mr_size_t alloc;

    mr_str_ct code;
    mr_pos_t pos;
};
typedef struct __MR_LEXER_MATCH_T mr_lexer_match_t;

/**
 * The list of possible codes returned by the \a mr_lexer_match function.
 * @var __MR_LEXER_MATCH_FLAG_ENUM::MR_LEXER_MATCH_FLAG_OK
 * The matching completed successfully.
 * @var __MR_LEXER_MATCH_FLAG_ENUM::MR_LEXER_MATCH_FLAG_ILLEGAL
 * The function found an illegal character during the matching process.
 * @var __MR_LEXER_MATCH_FLAG_ENUM::MR_LEXER_MATCH_FLAG_MISSING
 * A character is missing from the code.
 * @var __MR_LEXER_MATCH_FLAG_ENUM::MR_LEXER_MATCH_FLAG_MEMORY
 * The process failed due to the memory allocation failure.
*/
enum __MR_LEXER_MATCH_FLAG_ENUM
{
    MR_LEXER_MATCH_FLAG_OK,
    MR_LEXER_MATCH_FLAG_ILLEGAL,
    MR_LEXER_MATCH_FLAG_MISSING,
    MR_LEXER_MATCH_FLAG_MEMORY
};

/**
 * It matches the characters against the lexer patterns and generates tokens one by one.
 * @param data
 * The data structure containing the information about the code.
 * @param end
 * The terminator character. The matching process stops once it gets to this character.
*/
void mr_lexer_match(mr_lexer_match_t *data, mr_chr_t end);

/**
 * It skips the comments (both singleline and multiline ones).
 * @param data
 * The data structure containing the information about the code.
*/
void mr_lexer_skip_comment(mr_lexer_match_t *data);

mr_byte_t mr_lexer(mr_lexer_t *res, mr_str_ct code)
{
    mr_lexer_match_t data;
    data.flag = MR_LEXER_MATCH_FLAG_OK;

    data.tokens = mr_alloc(MR_LEXER_TOKENS_SIZE * sizeof(mr_token_t));
    if (!data.tokens)
        return ERROR_NOT_ENOUGH_MEMORY;

    data.size = 0;
    data.alloc = MR_LEXER_TOKENS_SIZE;

    data.code = code;
    data.pos = (mr_pos_t){0, 1};

    while (1)
    {
        mr_lexer_skip_spaces(code, data.pos.idx);
        if (code[data.pos.idx] == '\n')
        {
            data.pos.idx++;
            data.pos.ln++;
            continue;
        }

        if (code[data.pos.idx] == '#')
        {
            mr_lexer_skip_comment(&data);
            continue;
        }

        break;
    }

    mr_token_t *block;
    while (code[data.pos.idx] != '\0')
    {
        if (data.size == data.alloc)
        {
            block = mr_realloc(data.tokens,
                (data.alloc += MR_LEXER_TOKENS_SIZE) * sizeof(mr_token_t));
            if (!block)
            {
                while (data.size--)
                    mr_aligned_free(data.tokens[data.size].value);
                mr_free(data.tokens);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            data.tokens = block;
        }

        mr_lexer_match(&data, '\0');
        if (data.flag)
        {
            while (data.size--)
                mr_aligned_free(data.tokens[data.size].value);
            mr_free(data.tokens);

            if (data.flag == MR_LEXER_MATCH_FLAG_MEMORY)
                return ERROR_NOT_ENOUGH_MEMORY;

            res->tokens = NULL;
            res->error =
                (mr_illegal_chr_t){code[data.pos.idx], data.flag == MR_LEXER_MATCH_FLAG_MISSING, data.pos};
            return NO_ERROR;
        }
    }

    if (data.size + 1 != data.alloc)
    {
        block = mr_realloc(data.tokens, (data.size + 1) * sizeof(mr_token_t));
        if (!block)
        {
            while (data.size--)
                mr_aligned_free(data.tokens[data.size].value);
            mr_free(data.tokens);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        data.tokens = block;
    }

    data.tokens[data.size].type = MR_TOKEN_EOF;
    data.tokens[data.size].value = NULL;
    data.tokens[data.size].poss = data.pos;
    data.pos.idx++;
    data.tokens[data.size].pose = data.pos;

    res->tokens = data.tokens;
    return NO_ERROR;
}

void mr_lexer_match(mr_lexer_match_t *data, mr_chr_t end)
{
    if (data->code[data->pos.idx] == end)
        return;

    if (data->code[data->pos.idx] == '+')
    {
        data->tokens[data->size].type = MR_TOKEN_PLUS;
        data->tokens[data->size].value = NULL;
        data->tokens[data->size].poss = data->pos;
        data->pos.idx++;
        data->tokens[data->size++].pose = data->pos;
    }
}

void mr_lexer_skip_comment(mr_lexer_match_t *data)
{
    if (data->code[++data->pos.idx] != '*')
    {
        while (data->code[data->pos.idx] != '\0' && data->code[data->pos.idx] != '\n')
            data->pos.idx++;

        return;
    }

    data->pos.idx++;
    while (data->code[data->pos.idx] != '\0')
    {
        if (data->code[data->pos.idx] == '*')
        {
            data->pos.idx++;
            if (data->code[data->pos.idx] == '#')
            {
                data->pos.idx++;
                return;
            }
        }

        if (data->code[data->pos.idx] == '\n')
            data->pos.ln++;
        data->pos.idx++;
    }
}
