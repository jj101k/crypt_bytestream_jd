class ByteStream < String
=begin rdoc
A subclass of String with a single purpose: to provide the ^ (XOR) operator, for encryption purposes.
=end

=begin rdoc
Returned values are still of class ByteStream, and are of the same length as the longer value. Note that this means that:
a=ByteStream.new("a")
bb=ByteStream.new("b")
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
end
