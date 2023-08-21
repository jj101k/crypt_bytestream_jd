#include "ruby.h"
#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <machine/types.h>
#endif

VALUE cSelf;

static uint32_t * _binary_add_32(uint32_t *string1, uint32_t length1,
    uint32_t *string2, uint32_t length2);

/*
 * Binary add with a "String"-like object, return an object of the same
 * class as self. This effectively is little-endian addition in 8-bit or 32-bit
 * style, always carrying to the right.
 *
 * Returned values are of the same length as the longer value. Note that this
 * means that overflows are _dropped_ if both values are the same length.
 *
 */

static VALUE bs_binary_add(VALUE self, VALUE in_string) {
    char *self_p;
    char *in_string_p;
    char *out_p;
    long small_length, large_length;
    char i_am_smaller;
    int i;
    long self_len, in_len;
    VALUE string_out;

    in_string_p = RSTRING_PTR(in_string);
    self_p = RSTRING_PTR(self);
    self_len = RSTRING_LEN(self);
    in_len = RSTRING_LEN(in_string);

    /*
     * Find out which is longer (it doesn't matter if neither is)
     */
    if(self_len < in_len) {
        i_am_smaller = 1;
        small_length = self_len;
        large_length = in_len;
    } else {
        i_am_smaller = 0;
        small_length = in_len;
        large_length = self_len;
    }

    if(self_len % 4 == 0 && in_len % 4 == 0) {
        out_p = (char *)_binary_add_32((uint32_t *)self_p,
            self_len / 4, (uint32_t *)in_string_p, in_len / 4);
    } else {
        out_p = (char *)malloc(sizeof(char)* large_length);

        /*
         * For each byte that exists in both strings, add the result into the
         * (start of the) output c-string
         */
        char overflow = 0;
        short accumulator;
        for(i = 0; i < small_length; i++) {
            accumulator = self_p[i] + in_string_p[i] + overflow;
            overflow = (accumulator >= 256) ? 1 : 0;
            out_p[i] = accumulator % 256;
        }
        /*
         * Domino the tail (the bit after the shorter string would end)
         * of the longer string. Note that this is rather inefficient
         * for cases where the overflow is absorbed early.
         */
        if(small_length != large_length) {
            char *larger_string = i_am_smaller ? in_string_p : self_p;
            for(i = small_length; i < large_length; i++) {
                accumulator = larger_string[i] + overflow;
                overflow = (accumulator >= 256) ? 1 : 0;
                out_p[i] = accumulator % 256;
            }
        }
    }

    /*
     * Create a new "String" and massage it into the local class type.
     */
    string_out = rb_str_new(out_p, large_length);
    free(out_p);
    return rb_class_new_instance(1, (VALUE *)&string_out, rb_obj_class(self));
}
static uint32_t * _binary_add_32(uint32_t *string1, uint32_t length1,
    uint32_t *string2, uint32_t length2
) {
    uint32_t *out_p;
    long small_length, large_length;
    char i_am_smaller;
    int i;

    if(length1 < length2) {
        i_am_smaller = 1;
        small_length = length1;
        large_length = length2;
    } else {
        i_am_smaller = 0;
        small_length = length2;
        large_length = length1;
    }

    out_p = (uint32_t *)malloc(sizeof(uint32_t)* large_length);

    uint32_t overflow = 0;
    uint64_t accumulator;
    for(i = 0; i < small_length; i++) {
        accumulator = ntohl(string1[i]) + ntohl(string2[i]) + overflow;
        overflow = (accumulator >= 1ll << 32) ? 1 : 0;
        out_p[i] = htonl(accumulator % (1ll << 32));
    }

    /*
     * Domino the tail (the bit after the shorter string would end)
     * of the longer string. Note that this is rather inefficient
     * for cases where the overflow is absorbed early.
     */
    if(small_length != large_length) {
        uint32_t *larger_string = i_am_smaller ? string2 : string1;
        for(i = small_length; i < large_length; i++) {
            accumulator = ntohl(larger_string[i]) + overflow;
            overflow = (accumulator >= (1ll << 32)) ? 1 : 0;
            out_p[i] = htonl(accumulator % (1ll << 32));
        }
    }

    return out_p;
}

static uint32_t * _binary_xor_32(uint32_t *string1, uint32_t length1,
    uint32_t *string2, uint32_t length2
) {
    uint32_t *out_p;
    long small_length, large_length;
    char i_am_smaller;
    int i;

    /*
     * Find out which is longer (it doesn't matter if neither is)
     */
    if(length1 < length2) {
        i_am_smaller = 1;
        small_length = length1;
        large_length = length2;
    } else {
        i_am_smaller = 0;
        small_length = length2;
        large_length = length1;
    }

    out_p = (uint32_t *)malloc(sizeof(uint32_t)* large_length);

    /*
     * Copy the tail (the bit after the shorter string would end)
     * verbatim from the longer string.
     *
     * But only if there actually is a longer string.
     */

    if(small_length != large_length) {
        memcpy(out_p + small_length,
            (i_am_smaller ? string2 : string1) + small_length,
            (large_length - small_length) * 4);
    }

    /*
     * For each byte that exists in both strings, XOR the result into the
     * (start of the) output c-string
     */
    for(i = 0; i < small_length; i++) {
        out_p[i] = string1[i] ^ string2[i];
    }
    return out_p;
}

/*
 * Binary XOR with a "String"-like object, return an object of the same
 * class as self.
 *
 * Returned values are of the same length as the longer value. Note that this
 * means that:
 *
 * a=JdCrypt::ByteStream.new("a")
 * bb=JdCrypt::ByteStream.new("bb")
 * a^bb^bb
 *
 * ...does not equal "a" but rather "a\000", so this should be used with caution
 * except where you have equal-size strings in mind.
 */

static VALUE bs_binary_xor(VALUE self, VALUE in_string) {
    char *self_p;
    char *in_string_p;
    char *out_p;
    long small_length, large_length;
    char i_am_smaller;
    int i;
    VALUE string_out;
    long self_len, in_len;

    in_string_p = RSTRING_PTR(in_string);
    self_p = RSTRING_PTR(self);

    self_len = RSTRING_LEN(self);
    in_len = RSTRING_LEN(in_string);

    /*
     * Find out which is longer (it doesn't matter if neither is)
     */
    if(self_len < in_len) {
        i_am_smaller = 1;
        small_length = self_len;
        large_length = in_len;
    } else {
        i_am_smaller = 0;
        small_length = in_len;
        large_length = self_len;
    }
    if(self_len % 4 == 0 && in_len % 4 == 0) {
        out_p = (char *)_binary_xor_32((uint32_t *)self_p, self_len / 4,
            (uint32_t *)in_string_p, in_len / 4);
    } else {
        out_p = (char *)malloc(sizeof(char)* large_length);

        /*
         * Copy the tail (the bit after the shorter string would end)
         * verbatim from the longer string.
         *
         * But only if there actually is a longer string.
         */

        if(small_length != large_length) {
            memcpy(out_p + small_length,
                (i_am_smaller ? in_string_p : self_p) + small_length,
                large_length - small_length);
        }

        /*
         * For each byte that exists in both strings, XOR the result into the
         * (start of the) output c-string
         */
        for(i = 0; i < small_length; i++) {
            out_p[i] = self_p[i] ^ in_string_p[i];
        }
    }

    /*
     * Create a new "String" and massage it into the local class type.
     */
    string_out = rb_str_new(out_p, large_length);
    free(out_p);
    return rb_class_new_instance(1, (VALUE *)&string_out, rb_obj_class(self));
}

/*
 * Almost exactly the same as String#to_str. Returns self as a "String".
 */
static VALUE bs_to_str(VALUE self) {
    return rb_str_new(RSTRING_PTR(self), RSTRING_LEN(self));
}

/*
 *
 */

static VALUE bs_byte_at(int argc, VALUE args[], VALUE self) {
    char *self_p;

    if (argc > 2 || argc < 1) {
        rb_raise(rb_eArgError, "wrong number of arguments, only 1 or 2 supported");
    }
    long pos = NUM2LONG(args[0]);
    if(pos > RSTRING_LEN(self) - 1) {
        return Qnil;
    } else {
        self_p = RSTRING_PTR(self);
        if(argc == 2 && !NIL_P(args[1])) {
            self_p[pos] = NUM2INT(args[1]);
        }
        return INT2FIX(self_p[pos]);
    }
}

ID eqeq;
VALUE compatMode;
char compatMode_1_8;

static VALUE bs_compatMode() {
    return compatMode;
}

static VALUE bs_compatMode_assign(VALUE self, VALUE cm) {
    if(strncmp(StringValueCStr(cm), "1.8", 4) == 0) {
        compatMode_1_8 = 1;
    } else {
        compatMode_1_8 = 0;
    }
    compatMode = cm;
    return compatMode;
}

VALUE strictCompat = Qfalse;

static VALUE bs_strictCompat() {
    return strictCompat;
}

static VALUE bs_strictCompat_assign(VALUE self, VALUE sc) {
    strictCompat = sc;
    return strictCompat;
}

static VALUE bs_array_offset(int argc, VALUE argv[], VALUE self) {
    char *self_p;
    VALUE offsetOrRange;

    if (argc > 2 || argc < 1) {
        rb_raise(rb_eArgError, "wrong number of arguments, only 1 or 2 supported");
    }
    if(argc == 2) {
        return rb_call_super(argc, argv);
    } else if(compatMode_1_8) {
        offsetOrRange = argv[0];
        if(TYPE(offsetOrRange) != T_FIXNUM) {
            return rb_call_super(argc, argv);
        } else if(strictCompat == Qtrue) {
            rb_raise(rb_eRuntimeError, "Ambiguous, you must use #byte_at instead");
        } else {
            rb_warn("Ambiguous usage of [], please use #byte_at");
            return rb_call_super(argc, argv);
        }
    } else {
        return rb_call_super(argc, argv);
    }
}

/*
 * A subclass of String with a single purpose: to provide the ^ (XOR) operator,
 * for encryption purposes.
*/
void Init_bytestream() {
    VALUE cJdCrypt=rb_define_class("JdCrypt", rb_cObject);
    VALUE cSelf=rb_define_class_under(cJdCrypt, "ByteStream", rb_cString);
    rb_define_singleton_method(cSelf, "compatMode", bs_compatMode, 0);
    rb_define_singleton_method(cSelf, "compatMode=", bs_compatMode_assign, 1);
    rb_define_singleton_method(cSelf, "strictCompat", bs_strictCompat, 0);
    rb_define_singleton_method(cSelf, "strictCompat=", bs_strictCompat_assign, 1);
    rb_define_singleton_method(cSelf, "strict_mode=", bs_strictCompat_assign, 1);
    rb_define_method(cSelf, "^", bs_binary_xor, 1);
    rb_define_method(cSelf, "+", bs_binary_add, 1);
    rb_define_method(cSelf, "to_str", bs_to_str, 0);
    rb_define_method(cSelf, "byte_at", bs_byte_at, -1);
    rb_define_method(cSelf, "[]", bs_array_offset, -1);

    eqeq = rb_to_id(rb_str_new2("=="));
    compatMode = rb_str_new2("1.8");
    compatMode_1_8 = 1;
}
