#include "rprintf.h"

int isdig(int c) {
    return (c >= '0' && c <= '9');
}

static int skip_atoi(const char **s) {
    int i = 0;
    while (isdig(**s))
        i = i*10 + *((*s)++) - '0';
    return i;
}

#define ZEROPAD 1
#define SIGN    2
#define PLUS    4
#define SPACE   8
#define LEFT    16
#define SPECIAL 32
#define SMALL   64

#define do_div(n,base) ({ \
int __res; \
__res = ((unsigned long) n) % (unsigned) base; \
n = ((unsigned long) n) / (unsigned) base; \
__res; })

static char * number(char * str, int num, int base, int size, int precision, int type) {
    char c,sign,tmp[36];
    const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;

    if (type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz";
    if (type&LEFT) type &= ~ZEROPAD;
    if (base<2 || base>36)
        return 0;
    c = (type & ZEROPAD) ? '0' : ' ' ;
    if (type&SIGN && num<0) {
        sign='-';
        num = -num;
    } else
        sign=(type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);
    if (sign) size--;
    if (type&SPECIAL)
        if (base==16) size -= 2;
        else if (base==8) size--;
    i=0;
    if (num==0)
        tmp[i++]='0';
    else while (num!=0)
        tmp[i++]=digits[do_div(num,base)];
    if (i>precision) precision=i;
    size -= precision;
    if (!(type&(ZEROPAD+LEFT)))
        while(size-->0)
            *str++ = ' ';
    if (sign)
        *str++ = sign;
    if (type&SPECIAL)
        if (base==8)
            *str++ = '0';
        else if (base==16) {
            *str++ = '0';
            *str++ = digits[33];
        }
    if (!(type&LEFT))
        while(size-->0)
            *str++ = c;
    while(i<precision--)
        *str++ = '0';
    while(i-->0)
        *str++ = tmp[i];
    while(size-->0)
        *str++ = ' ';
    return str;
}

void esp_vprintf( const func_ptr f_ptr, charptr ctrl, va_list argp) {
    int len;
    int i;
    char *s;
    int *ip;
    int flags;
    int field_width;
    int precision;
    int qualifier;
    char buf[512];
    char *str;

    for (str = buf; *ctrl; ++ctrl) {
        if (*ctrl != '%') {
            *str++ = *ctrl;
            continue;
        }

        flags = 0;
        repeat:
            ++ctrl;
            switch (*ctrl) {
                case '-': flags |= LEFT; goto repeat;
                case '+': flags |= PLUS; goto repeat;
                case ' ': flags |= SPACE; goto repeat;
                case '#': flags |= SPECIAL; goto repeat;
                case '0': flags |= ZEROPAD; goto repeat;
            }

        field_width = -1;
        if (isdig(*ctrl))
            field_width = skip_atoi(&ctrl);
        else if (*ctrl == '*') {
            field_width = va_arg(argp, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        precision = -1;
        if (*ctrl == '.') {
            ++ctrl;
            if (isdig(*ctrl))
                precision = skip_atoi(&ctrl);
            else if (*ctrl == '*') {
                precision = va_arg(argp, int);
            }
            if (precision < 0)
                precision = 0;
        }

        qualifier = -1;
        if (*ctrl == 'h' || *ctrl == 'l' || *ctrl == 'L') {
            qualifier = *ctrl;
            ++ctrl;
        }

        switch (*ctrl) {
            case 'c':
                if (!(flags & LEFT))
                    while (--field_width > 0)
                        *str++ = ' ';
                *str++ = (unsigned char) va_arg(argp, int);
                while (--field_width > 0)
                    *str++ = ' ';
                break;

            case 's':
                s = va_arg(argp, char *);
                if (!s)
                    s = "<NULL>";
                len = 0;
                while (s[len]) len++;
                if (precision >= 0 && len > precision)
                    len = precision;

                if (!(flags & LEFT))
                    while (len < field_width--)
                        *str++ = ' ';
                for (i = 0; i < len; ++i)
                    *str++ = *s++;
                while (len < field_width--)
                    *str++ = ' ';
                break;

            case 'o':
                str = number(str, va_arg(argp, unsigned long), 8,
                    field_width, precision, flags);
                break;

            case 'p':
                if (field_width == -1) {
                    field_width = 8;
                    flags |= ZEROPAD;
                }
                str = number(str,
                    (unsigned long) va_arg(argp, void *), 16,
                    field_width, precision, flags);
                break;

            case 'x':
                flags |= SMALL;
            case 'X':
                str = number(str, va_arg(argp, unsigned long), 16,
                    field_width, precision, flags);
                break;

            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                str = number(str, va_arg(argp, unsigned long), 10,
                    field_width, precision, flags);
                break;

            case 'n':
                ip = va_arg(argp, int *);
                *ip = (str - buf);
                break;

            default:
                if (*ctrl != '%')
                    *str++ = '%';
                if (*ctrl)
                    *str++ = *ctrl;
                else
                    --ctrl;
                break;
        }
    }
    *str = '\0';

    for (i = 0; buf[i]; i++)
        f_ptr(buf[i]);
}

void esp_printf( const func_ptr f_ptr, charptr ctrl, ...) {
    va_list argp;
    va_start(argp, ctrl);
    esp_vprintf(f_ptr, ctrl, argp);
    va_end(argp);
}

void esp_sprintf(char *buf, char *ctrl, ...) {
    // Not implemented for kernel use
}

void printk(charptr ctrl, ...) {
    // Not implemented for kernel use
}
