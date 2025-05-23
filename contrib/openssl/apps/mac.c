/*
 * Copyright 2018-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>

#include "apps.h"
#include "progs.h"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>

#undef BUFSIZE
#define BUFSIZE 1024*8

typedef enum OPTION_choice {
    OPT_COMMON,
    OPT_MACOPT, OPT_BIN, OPT_IN, OPT_OUT,
    OPT_PROV_ENUM
} OPTION_CHOICE;

const OPTIONS mac_options[] = {
    {OPT_HELP_STR, 1, '-', "Usage: %s [options] mac_name\n"},

    OPT_SECTION("General"),
    {"help", OPT_HELP, '-', "Display this summary"},
    {"macopt", OPT_MACOPT, 's', "MAC algorithm parameters in n:v form"},
    {OPT_MORE_STR, 1, '-', "See 'PARAMETER NAMES' in the EVP_MAC_ docs"},

    OPT_SECTION("Input"),
    {"in", OPT_IN, '<', "Input file to MAC (default is stdin)"},

    OPT_SECTION("Output"),
    {"out", OPT_OUT, '>', "Output to filename rather than stdout"},
    {"binary", OPT_BIN, '-',
        "Output in binary format (default is hexadecimal)"},

    OPT_PROV_OPTIONS,

    OPT_PARAMETERS(),
    {"mac_name", 0, 0, "MAC algorithm"},
    {NULL}
};

int mac_main(int argc, char **argv)
{
    int ret = 1;
    char *prog;
    EVP_MAC *mac = NULL;
    OPTION_CHOICE o;
    EVP_MAC_CTX *ctx = NULL;
    STACK_OF(OPENSSL_STRING) *opts = NULL;
    unsigned char *buf = NULL;
    size_t len;
    int i;
    BIO *in = NULL, *out = NULL;
    const char *outfile = NULL;
    const char *infile = NULL;
    int out_bin = 0;
    int inform = FORMAT_BINARY;
    OSSL_PARAM *params = NULL;

    prog = opt_init(argc, argv, mac_options);
    buf = app_malloc(BUFSIZE, "I/O buffer");
    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        default:
opthelp:
            BIO_printf(bio_err, "%s: Use -help for summary.\n", prog);
            goto err;
        case OPT_HELP:
            opt_help(mac_options);
            ret = 0;
            goto err;
        case OPT_BIN:
            out_bin = 1;
            break;
        case OPT_IN:
            infile = opt_arg();
            break;
        case OPT_OUT:
            outfile = opt_arg();
            break;
        case OPT_MACOPT:
            if (opts == NULL)
                opts = sk_OPENSSL_STRING_new_null();
            if (opts == NULL || !sk_OPENSSL_STRING_push(opts, opt_arg()))
                goto opthelp;
            break;
        case OPT_PROV_CASES:
            if (!opt_provider(o))
                goto err;
            break;
        }
    }

    /* One argument, the MAC name. */
    argc = opt_num_rest();
    argv = opt_rest();
    if (argc != 1)
        goto opthelp;

    mac = EVP_MAC_fetch(app_get0_libctx(), argv[0], app_get0_propq());
    if (mac == NULL) {
        BIO_printf(bio_err, "Invalid MAC name %s\n", argv[0]);
        goto opthelp;
    }

    ctx = EVP_MAC_CTX_new(mac);
    if (ctx == NULL)
        goto err;

    if (opts != NULL) {
        int ok = 1;

        params = app_params_new_from_opts(opts,
                                          EVP_MAC_settable_ctx_params(mac));
        if (params == NULL)
            goto err;

        if (!EVP_MAC_CTX_set_params(ctx, params)) {
            BIO_printf(bio_err, "MAC parameter error\n");
            ERR_print_errors(bio_err);
            ok = 0;
        }
        app_params_free(params);
        if (!ok)
            goto err;
    }

    /* Use text mode for stdin */
    if (infile == NULL || strcmp(infile, "-") == 0)
        inform = FORMAT_TEXT;
    in = bio_open_default(infile, 'r', inform);
    if (in == NULL)
        goto err;

    out = bio_open_default(outfile, 'w', out_bin ? FORMAT_BINARY : FORMAT_TEXT);
    if (out == NULL)
        goto err;

    if (!EVP_MAC_init(ctx, NULL, 0, NULL)) {
        BIO_printf(bio_err, "EVP_MAC_Init failed\n");
        goto err;
    }

    for (;;) {
        i = BIO_read(in, (char *)buf, BUFSIZE);
        if (i < 0) {
            BIO_printf(bio_err, "Read Error in '%s'\n", infile);
            goto err;
        }
        if (i == 0)
            break;
        if (!EVP_MAC_update(ctx, buf, i)) {
            BIO_printf(bio_err, "EVP_MAC_update failed\n");
            goto err;
        }
    }

    if (!EVP_MAC_final(ctx, NULL, &len, 0)) {
        BIO_printf(bio_err, "EVP_MAC_final failed\n");
        goto err;
    }
    if (len > BUFSIZE) {
        BIO_printf(bio_err, "output len is too large\n");
        goto err;
    }

    if (!EVP_MAC_final(ctx, buf, &len, BUFSIZE)) {
        BIO_printf(bio_err, "EVP_MAC_final failed\n");
        goto err;
    }

    if (out_bin) {
        BIO_write(out, buf, len);
    } else {
        if (outfile == NULL)
            BIO_printf(out,"\n");
        for (i = 0; i < (int)len; ++i)
            BIO_printf(out, "%02X", buf[i]);
        if (outfile == NULL)
            BIO_printf(out,"\n");
    }

    ret = 0;
err:
    if (ret != 0)
        ERR_print_errors(bio_err);
    OPENSSL_clear_free(buf, BUFSIZE);
    sk_OPENSSL_STRING_free(opts);
    BIO_free(in);
    BIO_free(out);
    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);
    return ret;
}
