# frozen_string_literal: true

class JdCrypt
  # A subclass of String with a single purpose: to provide the ^ (XOR) operator,
  # for encryption purposes.
  class ByteStream < String
    # Returned values are still of class JdCrypt::ByteStream, and are of the
    # same length as the longer value. Note that this means that:
    # a=JdCrypt::ByteStream.new("a")
    # bb=JdCrypt::ByteStream.new("b")
    # a^bb^bb
    #
    # ...does not equal "a" but rather "a\000", so this should be used with
    # caution except where you have equal-size strings in mind.
    def ^(other)
      max_length = [length, other.length].max
      my_bytes = unpack("C*")
      other_bytes = other.unpack("C*")
      out_bytes = (0..max_length - 1).map do |i|
        (my_bytes[i] || 0) ^ (other_bytes[i] || 0)
      end
      self.class.new(out_bytes.pack("C*"))
    end

    def plus_bytes(other)
      my_bytes = unpack("C*")
      other_bytes = other.unpack("C*")
      max_length = [my_bytes.length, other_bytes.length].max
      overflow = 0
      out_bytes = (0..max_length - 1).map do |i|
        accumulator = (my_bytes[i] || 0) + (other_bytes[i] || 0) + overflow
        overflow = (accumulator & 0x100) >> 8
        accumulator & 0xff
      end
      self.class.new(out_bytes.pack("C*"))
    end

    def plus_words(other)
      my_dwords = unpack("N*")
      other_dwords = other.unpack("N*")
      max_length = [my_dwords.length, other_dwords.length].max
      out_dwords = (0..max_length - 1).map do |i|
        ((my_dwords[i] || 0) + (other_dwords[i] || 0)) & 0xffffffff
      end
      self.class.new(out_dwords.pack("N*"))
    end

    # Binary add with a "String"-like object, return an object of the same class as
    # self. This effectively is little-endian addition in 8-bit or 32-bit style,
    # always carrying to the right.
    #
    # Returned values are of the same length as the longer value. Note that this means
    # that overflows are _dropped_ if both values are the same length.
    def +(other)
      if (other.length % 4).zero? && (length % 4).zero?
        plus_words(other)
      else
        plus_bytes(other)
      end
    end

    Use_getbyte = "".respond_to?(:getbyte)

    def byte_at(position, new_value = nil)
      if new_value
        self[position, 1] = [new_value].pack("C")
      elsif Use_getbyte
        getbyte(position)
      else
        slice(position)
      end
    end

    private :plus_words, :plus_bytes
  end
end
