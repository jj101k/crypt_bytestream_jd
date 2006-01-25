class Crypt
class ByteStream < String
=begin rdoc
A subclass of String with a single purpose: to provide the ^ (XOR) operator, for encryption purposes.
=end

=begin rdoc
Returned values are still of class Crypt::ByteStream, and are of the same length as the longer value. Note that this means that:
a=Crypt::ByteStream.new("a")
bb=Crypt::ByteStream.new("b")
a^bb^bb

...does not equal "a" but rather "a\000", so this should be used with caution except where you have equal-size strings in mind.
=end
    def ^(string)
        max_length=if(self.length > string.length)
            self.length
        else
            string.length
        end
        my_bytes=self.unpack("C*")
        other_bytes=string.unpack("C*")
        out_bytes=Array.new
        0.upto(max_length-1) do
            |i|
            out_bytes[i]=(my_bytes[i]||0)^(other_bytes[i]||0)
        end
        self.class.new(out_bytes.pack("C*"))
    end
               def +(string)
        my_dwords=self.unpack("N*")
        other_dwords=string.unpack("N*")
        max_length=if(my_dwords.length > other_dwords.length)
            my_dwords.length
        else
            other_dwords.length
        end
        out_dwords=Array.new
        0.upto(max_length-1) do
            |i|
            out_dwords[i]=(my_dwords[i]||0)+(other_dwords[i]||0)&0xffffffff
        end
        self.class.new(out_dwords.pack("N*"))
               end
end
end
