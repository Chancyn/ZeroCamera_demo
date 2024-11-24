// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "zc_macros.h"
#include "zc_praseutils.h"
#include "zc_stringutils.h"

// ?key1=value1&key2=value2&key3=value3
int zc_prase_key_value(const char *src, const char *key, char *value, int vlen)
{
    const char *p;
    char tmpkey[128], *q;

    p = src;
    if (*p == '?')
        p++;
    for(;;) {
        q = tmpkey;
        while (*p != '\0' && *p != '=' && *p != '&') {
            if ((q - tmpkey) < sizeof(tmpkey) - 1)
                *q++ = *p;
            p++;
        }
        *q = '\0';
        q = value;
        if (*p == '=') {
            p++;
            while (*p != '&' && *p != '\0') {
                if ((q - value) < vlen - 1) {
                    if (*p == '+')
                        *q++ = ' ';
                    else
                        *q++ = *p;
                }
                p++;
            }
        }
        *q = '\0';
        if (!strcmp(key, tmpkey))
            return 1;
        if (*p != '&')
            break;
        p++;
    }
    return 0;
}

void zc_url_split(char *proto, int proto_size, char *authorization, int authorization_size, char *hostname,
                  int hostname_size, int *port_ptr, char *path, int path_size, const char *url) {
    const char *p, *ls, *at, *at2, *col, *brk;

    if (port_ptr)
        *port_ptr = -1;
    if (proto_size > 0)
        proto[0] = 0;
    if (authorization_size > 0)
        authorization[0] = 0;
    if (hostname_size > 0)
        hostname[0] = 0;
    if (path_size > 0)
        path[0] = 0;

    /* parse protocol */
    if ((p = strchr(url, ':'))) {
        zc_strlcpy(proto, url, ZCMIN(proto_size, p + 1 - url));
        p++; /* skip ':' */
        if (*p == '/')
            p++;
        if (*p == '/')
            p++;
    } else {
        /* no protocol means plain filename */
        zc_strlcpy(path, url, path_size);
        return;
    }

    /* separate path from hostname */
    ls = p + strcspn(p, "/?#");
    zc_strlcpy(path, ls, path_size);

    /* the rest is hostname, use that to parse auth/port */
    if (ls != p) {
        /* authorization (user[:pass]@hostname) */
        at2 = p;
        while ((at = strchr(p, '@')) && at < ls) {
            zc_strlcpy(authorization, at2, ZCMIN(authorization_size, at + 1 - at2));
            p = at + 1; /* skip '@' */
        }

        if (*p == '[' && (brk = strchr(p, ']')) && brk < ls) {
            /* [host]:port */
            zc_strlcpy(hostname, p + 1, ZCMIN(hostname_size, brk - p));
            if (brk[1] == ':' && port_ptr)
                *port_ptr = atoi(brk + 2);
        } else if ((col = strchr(p, ':')) && col < ls) {
            zc_strlcpy(hostname, p, ZCMIN(col + 1 - p, hostname_size));
            if (port_ptr)
                *port_ptr = atoi(col + 1);
        } else {
            zc_strlcpy(hostname, p, ZCMIN(ls + 1 - p, hostname_size));
        }
    }
}

/**
 * Decodes an URL from its percent-encoded form back into normal
 * representation. This function returns the decoded URL in a string.
 * The URL to be decoded does not necessarily have to be encoded but
 * in that case the original string is duplicated.
 *
 * @param url a string to be decoded.
 * @param decode_plus_sign if nonzero plus sign is decoded to space
 * @return new string with the URL decoded or NULL if decoding failed.
 * Note that the returned string should be explicitly freed when not
 * used anymore.
 */
char *zc_urldecode(const char *url, int decode_plus_sign)
{
    int s = 0, d = 0, url_len = 0;
    char c;
    char *dest = NULL;

    if (!url)
        return NULL;

    url_len = strlen(url) + 1;
    dest = malloc(url_len);

    if (!dest)
        return NULL;

    while (s < url_len) {
        c = url[s++];

        if (c == '%' && s + 2 < url_len) {
            char c2 = url[s++];
            char c3 = url[s++];
            if (isxdigit(c2) && isxdigit(c3)) {
                c2 = tolower(c2);
                c3 = tolower(c3);

                if (c2 <= '9')
                    c2 = c2 - '0';
                else
                    c2 = c2 - 'a' + 10;

                if (c3 <= '9')
                    c3 = c3 - '0';
                else
                    c3 = c3 - 'a' + 10;

                dest[d++] = 16 * c2 + c3;

            } else { /* %zz or something other invalid */
                dest[d++] = c;
                dest[d++] = c2;
                dest[d++] = c3;
            }
        } else if (c == '+' && decode_plus_sign) {
            dest[d++] = ' ';
        } else {
            dest[d++] = c;
        }
    }

    return dest;
}
