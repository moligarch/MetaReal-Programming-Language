/*
    MetaReal Compiler version 1.0.0
*/

#include <optimizer/context.h>
#include <alloc.h>
#include <string.h>
#include <parser/node.h>
#include <consts.h>

void context_free(context_t *context)
{
    while (context->size)
    {
        value_free(context->vars[--context->size].value);
        mr_free(context->vars[context->size].name);
    }

    mr_free(context->vars);
}

value_t *var_get(const context_t *context, const char *name)
{
    for (uint64_t i = 0; i < context->size; i++)
        if (!strcmp(name, context->vars[i].name))
        {
            if (context->vars[i].value->type == PTR_V)
                i = (uintptr_t)context->vars[i].value->value;

            value_addref(context->vars[i].value, 1);
            return context->vars[i].value;
        }

    return NULL;
}

void var_set(uint8_t *flag, context_t *context, char *name, value_t *value, uint8_t prop)
{
    for (uint64_t i = 0; i < context->size; i++)
        if (!strcmp(name, context->vars[i].name))
        {
            if (context->vars[i].value->type == PTR_V)
                i = (uintptr_t)context->vars[i].value->value;

            if (context->vars[i].prop & VAR_ASSIGN_CONST_MASK)
            {
                if (context->vars[i].value->type == NONE_V)
                {
                    value_free_vo(context->vars[i].value);
                    mr_free(name);

                    context->vars[i].value = value;
                    context->vars[i].prop |= prop;
                    return;
                }

                value_free(value);

                *flag = 1;
                return;
            }

            if (value->type == PTR_V && i == (uintptr_t)value->value)
            {
                mr_free(name);

                context->vars[i].prop |= prop;
                *flag = 2;
                return;
            }

            value_free(context->vars[i].value);
            mr_free(name);

            context->vars[i].value = value;
            context->vars[i].prop |= prop;
            return;
        }

    if (context->size == context->alloc)
        context->vars = mr_realloc(context->vars, (context->alloc += OPT_VARS_LIST_LEN) * sizeof(var_t));

    context->vars[context->size].name = name;
    context->vars[context->size].value = value;

    if (value->type == PTR_V)
        context->vars[(uintptr_t)value->value].prop |= prop;
    else
        context->vars[context->size].prop = prop;

    context->size++;
}

uint64_t var_getp(uint8_t *error, const context_t *context, const char *name)
{
    for (uint64_t i = 0; i < context->size; i++)
        if (!strcmp(name, context->vars[i].name))
        {
            if (context->vars[i].value->type == PTR_V)
                return (uintptr_t)context->vars[i].value->value;

            if (context->vars[i].prop & VAR_ASSIGN_CONST_MASK &&
                context->vars[i].value->type != NONE_V)
                *error = 2;

            return i;
        }

    *error = 1;
    return 0;
}

uint64_t var_setp(uint8_t *error, context_t *context, char *name, value_t *value, uint8_t prop)
{
    for (uint64_t i = 0; i < context->size; i++)
        if (!strcmp(name, context->vars[i].name))
        {
            if (context->vars[i].value->type == PTR_V)
                i = (uintptr_t)context->vars[i].value->value;

            if (context->vars[i].prop & VAR_ASSIGN_CONST_MASK)
            {
                if (context->vars[i].value->type == NONE_V && value->type == NONE_V)
                {
                    value_free_vo(value);
                    mr_free(name);

                    context->vars[i].prop |= prop;
                    return i;
                }

                value_free(value);

                *error = 1;
                return 0;
            }

            if (value->type == PTR_V && i == (uintptr_t)value->value)
            {
                value_free_vo(value);
                mr_free(name);

                context->vars[i].prop |= prop;
                return i;
            }

            value_free(context->vars[i].value);
            mr_free(name);

            context->vars[i].value = value;
            context->vars[i].prop |= prop;
            return i;
        }

    if (context->size == context->alloc)
        context->vars = mr_realloc(context->vars, (context->alloc += OPT_VARS_LIST_LEN) * sizeof(var_t));

    context->vars[context->size].name = name;
    context->vars[context->size].value = value;

    if (value->type == PTR_V)
        context->vars[(uintptr_t)value->value].prop |= prop;
    else
        context->vars[context->size].prop = prop;

    return context->size++;
}

uint64_t var_add(context_t *context, char *name)
{
    if (context->size == context->alloc)
        context->vars = mr_realloc(context->vars, (context->alloc += OPT_VARS_LIST_LEN) * sizeof(var_t));

    context->vars[context->size].name = name;
    context->vars[context->size].prop = 0;
    return context->size++;
}
