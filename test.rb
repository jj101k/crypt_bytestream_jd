require "./bytestream"
foo = JdCrypt::ByteStream.new("fooz")
puts "Checking integrity..."
puts(if(foo == "fooz")
    "Ok"
else
    "No"
end)
puts "Checking + (32b)..."
puts(if(foo + "    " == ["868F8F9A"].pack("H*"))
    "Ok"
else
    "No"
end)
Foo = JdCrypt::ByteStream.new("FOOX")
puts "Checking + (32b, long)..."
puts(if(Foo + "        " == "foox    ")
    "Ok"
else
    "No"
end)
puts "Checking + (8b)..."
puts(if(foo + " " == ["866f6f7a"].pack("H*"))
    "Ok"
else
    "No"
end)
puts "Checking + (8b, long)..."
puts(if(Foo + "     " == "foox ")
    "Ok"
else
    "No"
end)
puts "Checking XOR..."
foo ^= "123"
puts(if(foo == "W]\\z")
    "Ok"
else
    "No"
end)
puts "Checking that XOR won't de-pad..."
foo ^= "fooz"
puts(if(foo == "123\000")
    "Ok"
else
    "No"
end)

puts "Checking byte-at (assign)"
bar = JdCrypt::ByteStream.new("bar")
bar.byte_at 1, 32
puts(if(bar == "b r")
    "Ok"
else
    "No"
end)

puts "Checking byte-at (read)"
puts(if(bar.byte_at(0) == "b".ord)
    "Ok"
else
    "No"
end)

TestTimes = 10000
puts "Testing performance: #{TestTimes} simple XORs"
TestTimes.times do
  |i|
  foo ^= i.to_s
end
