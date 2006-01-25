#include "ruby.h"
#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <machine/types.h>
#endif

VALUE cSelf;

static uint32_t * _binary_add_32(uint32_t *string1, uint32_t length1, uint32_t *string2, uint32_t length2);
/*
 * Binary add with a "String"-like object, return an object of the same
 * class as self.
 *
 * Returned values are of the same length as the longer value. Note that this means that overflows are _dropped_.
 * 
 */
 
static VALUE bs_binary_add(VALUE self, VALUE in_string) {
    char *self_p;
    char *in_string_p;
    char *out_p;
    long small_length, large_length;
    char i_am_smaller;
    int i;
    VALUE string_out;
    
    in_string_p=RSTRING(in_string)->ptr;
    self_p=RSTRING(self)->ptr;
		
		/*
		 * Find out which is longer (it doesn't matter if neither is)
		 */
		if(RSTRING(self)->len < RSTRING(in_string)->len) {
				i_am_smaller=1;
				small_length=RSTRING(self)->len;
				large_length=RSTRING(in_string)->len;
		} else {
				i_am_smaller=0;
				small_length=RSTRING(in_string)->len;
				large_length=RSTRING(self)->len;    
		}

		if( (RSTRING(self)->len)%4==0 && (RSTRING(in_string)->len)%4==0) {
			out_p=(char *)_binary_add_32((uint32_t *)self_p, (RSTRING(self)->len)/4, (uint32_t *)in_string_p, (RSTRING(in_string)->len)/4);
		} else {
			
			out_p=(char *)malloc(sizeof(char)* large_length);
			
			 
			/*
			 * For each byte that exists in both strings, add the result into the
			 * (start of the) output c-string
			 */    
			char overflow=0;
			short accumulator;
			for(i=0;i<small_length;i++) {
					accumulator=self_p[i]+in_string_p[i]+overflow;
					overflow=(accumulator>=256)?1:0;
					out_p[i]=accumulator%256;
			}
			/* 
			 * Domino the tail (the bit after the shorter string would end)
			 * of the longer string. Note that this is rather inefficient
			 * for cases where the overflow is absorbed early.
			 */
			if(small_length!=large_length) {
				char *larger_string=i_am_smaller ? self_p : in_string_p;
				for(i=small_length;i<large_length;i++) {
					accumulator=larger_string[i]+overflow;
					overflow=(accumulator>=256)?1:0;
					out_p[i]=accumulator%256;
				}
			}
		}
    
    /*
     * Create a new "String" and massage it into the local class type.
     */ 
    string_out=rb_str_new(out_p, large_length);
    free(out_p);
    return rb_class_new_instance(1, (VALUE *)&string_out, rb_obj_class(self));
}
static uint32_t * _binary_add_32(uint32_t *string1, uint32_t length1, uint32_t *string2, uint32_t length2) {
    uint32_t *out_p;
    long small_length, large_length;
    char i_am_smaller;
    int i;
    
    if(length1 < length2) {
        i_am_smaller=1;
        small_length=length1;
        large_length=length2;
    } else {
        i_am_smaller=0;
        small_length=length2;
        large_length=length1;    
    }
    
    out_p=(uint32_t *)malloc(sizeof(uint32_t)* large_length);
    
     
		uint32_t overflow=0;
		uint64_t accumulator;
    for(i=0;i<small_length;i++) {
				accumulator=ntohl(string1[i])+ntohl(string2[i])+overflow;
				overflow=(accumulator>=1ll<<32)?1:0;
        out_p[i]=htonl(accumulator%(1ll<<32));
    }
    /* 
     * Domino the tail (the bit after the shorter string would end)
     * of the longer string. Note that this is rather inefficient
     * for cases where the overflow is absorbed early.
     */
    if(small_length!=large_length) {
			uint32_t *larger_string=i_am_smaller ? string1 : string2;
			for(i=small_length;i<large_length;i++) {
				accumulator=ntohl(larger_string[i])+overflow;
				overflow=(accumulator>=(1ll<<32))?1:0;
				out_p[i]=htonl(accumulator%(1ll<<32));
			}
		}
    
		return out_p;
}
 
static uint32_t * _binary_xor_32(uint32_t *string1, uint32_t length1, uint32_t *string2, uint32_t length2) {
    uint32_t *out_p;
    long small_length, large_length;
    char i_am_smaller;
    int i;
    
    /*
     * Find out which is longer (it doesn't matter if neither is)
     */
    if(length1 < length2) {
        i_am_smaller=1;
        small_length=length1;
        large_length=length2;
    } else {
        i_am_smaller=0;
        small_length=length2;
        large_length=length1;    
    }
    
    out_p=(uint32_t *)malloc(sizeof(uint32_t)* large_length);
    
    /* 
     * Copy the tail (the bit after the shorter string would end)
     * verbatim from the longer string.
     *
     * But only if there actually is a longer string.
     */
     
    if(small_length!=large_length) 
        memcpy(out_p+small_length, 
               (i_am_smaller ? string2 : string1)+small_length, 
               (large_length-small_length)*4);
    
    /*
     * For each byte that exists in both strings, XOR the result into the
     * (start of the) output c-string
     */    
    for(i=0;i<small_length;i++) {
        out_p[i]=string1[i]^string2[i];
    }
    
		return out_p;
}

/*
 * Binary XOR with a "String"-like object, return an object of the same
 * class as self.
 *
 * Returned values are of the same length as the longer value. Note that this means that:
 * a=ByteStream.new("a")
 * bb=ByteStream.new("bb")
 * a^bb^bb
 * 
 * ...does not equal "a" but rather "a\000", so this should be used with caution except where you have
 * equal-size strings in mind.
 */
 
static VALUE bs_binary_xor(VALUE self, VALUE in_string) {
    char *self_p;
    char *in_string_p;
    char *out_p;
    long small_length, large_length;
    char i_am_smaller;
    int i;
    VALUE string_out;
    
    in_string_p=RSTRING(in_string)->ptr;
    self_p=RSTRING(self)->ptr;
    
    /*
     * Find out which is longer (it doesn't matter if neither is)
     */
    if(RSTRING(self)->len < RSTRING(in_string)->len) {
        i_am_smaller=1;
        small_length=RSTRING(self)->len;
        large_length=RSTRING(in_string)->len;
    } else {
        i_am_smaller=0;
        small_length=RSTRING(in_string)->len;
        large_length=RSTRING(self)->len;    
    }
		if( (RSTRING(self)->len)%4==0 && (RSTRING(in_string)->len)%4==0) {
			out_p=(char *)_binary_xor_32((uint32_t *)self_p, (RSTRING(self)->len)/4, (uint32_t *)in_string_p, (RSTRING(in_string)->len)/4);
		} else {
			
			out_p=(char *)malloc(sizeof(char)* large_length);
			
			/* 
			 * Copy the tail (the bit after the shorter string would end)
			 * verbatim from the longer string.
			 *
			 * But only if there actually is a longer string.
			 */
			 
			if(small_length!=large_length) 
					memcpy(out_p+small_length, 
								 (i_am_smaller ? in_string_p : self_p)+small_length, 
								 large_length-small_length);
			
			/*
			 * For each byte that exists in both strings, XOR the result into the
			 * (start of the) output c-string
			 */    
			for(i=0;i<small_length;i++) {
					out_p[i]=self_p[i]^in_string_p[i];
			}
		}
    
    /*
     * Create a new "String" and massage it into the local class type.
     */ 
    string_out=rb_str_new(out_p, large_length);
    free(out_p);
    return rb_class_new_instance(1, (VALUE *)&string_out, rb_obj_class(self));
}


/*
 * Almost exactly the same as String#to_str. Returns self as a "String".
 */
static VALUE bs_to_str(VALUE self) {
    return rb_str_new(RSTRING(self)->ptr, RSTRING(self)->len);
}

/*
 *A subclass of String with a single purpose: to provide the ^ (XOR) operator, for encryption purposes.
*/
void Init_bytestream() {
    cSelf=rb_define_class("ByteStream", rb_cString);
    rb_define_method(cSelf, "^", bs_binary_xor, 1);
    rb_define_method(cSelf, "+", bs_binary_add, 1);
    rb_define_method(cSelf, "to_str", bs_to_str, 0);
}
