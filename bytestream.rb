class JdCrypt
    class ByteStream < String
=begin rdoc
A subclass of String with a single purpose: to provide the ^ (XOR) operator, for encryption purposes.
=end

=begin rdoc
Returned values are still of class JdCrypt::ByteStream, and are of the same length as the longer value. Note that this means that:
a=JdCrypt::ByteStream.new("a")
bb=JdCrypt::ByteStream.new("b")
a^bb^bb

...does not equal "a" but rather "a\000", so this should be used with caution except where you have equal-size strings in mind.
=end
        def ^(string)
            max_length = if(self.length > string.length)
                self.length
            else
                string.length
            end
            my_bytes = self.unpack("C*")
            other_bytes = string.unpack("C*")
            out_bytes = Array.new
            0.upto(max_length - 1) do
                |i|
                out_bytes[i] = (my_bytes[i] || 0) ^ (other_bytes[i] || 0)
            end
            self.class.new(out_bytes.pack("C*"))
        end
=begin rdoc

Binary add with a "String"-like object, return an object of the same class as
self. This effectively is little-endian addition in 8-bit or 32-bit style,
always carrying to the right.

Returned values are of the same length as the longer value. Note that this means
that overflows are _dropped_ if both values are the same length.

=end
        def +(string)
            if(string.length % 4 == 0 and self.length % 4 == 0)
                my_dwords = self.unpack("N*")
                other_dwords = string.unpack("N*")
                max_length = if(my_dwords.length > other_dwords.length)
                    my_dwords.length
                else
                    other_dwords.length
                end
                out_dwords = Array.new
                0.upto(max_length - 1) do
                    |i|
                    out_dwords[i] = ((my_dwords[i] || 0) + (other_dwords[i] || 0)) & 0xffffffff
                end
                self.class.new(out_dwords.pack("N*"))
            else
                my_bytes = self.unpack("C*")
                other_bytes = string.unpack("C*")
                max_length = if(my_bytes.length > other_bytes.length)
                    my_bytes.length
                else
                    other_bytes.length
                end
                out_bytes = Array.new
                overflow = 0
                0.upto(max_length - 1) do
                    |i|
                    accumulator = (my_bytes[i] || 0) + (other_bytes[i] || 0) + overflow
                    out_bytes[i] = accumulator & 0xff
                    overflow = accumulator > 0xff ? 1 : 0
                end
                self.class.new(out_bytes.pack("C*"))
            end
        end
        Use_getbyte = "".respond_to?(:getbyte)
        def byte_at(position, new_value = nil)
            if(new_value)
                self[position, 1] = [new_value].pack("C")
            elsif(Use_getbyte)
                self.getbyte(position)
            else
                self.slice(position)
            end
        end
        @@compatMode = "1.8"
        def self.compatMode=(compatMode)
            @@compatMode = compatMode
        end
        def self.compatMode()
            @@compatMode
        end
        @@strictCompat = false
        def self.strictCompat=(new_mode)
            @@strictCompat = new_mode
        end
        def self.strictCompat()
            @@strictCompat
        end
        def self.strict_mode=(new_mode)
            @@strictCompat = new_mode
        end
        def [](offsetOrRange, replacement = nil)
            if(replacement)
                super(offsetOrRange, replacement)
            elsif @@compatMode == "1.8"
                # Ruby 1.8 would return a number for this. Ruby 1.9 would return
                # a string for this. For a bytestream, we actually always want
                # to return a number.
                if(not offsetOrRange.is_a? Numeric)
                    super(offsetOrRange)
                elsif(@@strictCompat)
                    raise "Ambiguous, you must use #byte_at instead"
                else
                    STDERR.puts "Ambiguous usage of [], please use #byte_at"
                    super(offsetOrRange)
                end
            else
                super(offsetOrRange)
            end
        end
    end
end
