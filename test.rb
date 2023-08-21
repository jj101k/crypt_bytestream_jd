require "./bytestream"
foo = JdCrypt::ByteStream.new("fooz")

def assertWarn(v)
    puts(if v
        "Ok"
    else
        "** NO **"
    end)
end

puts "Checking integrity..."
assertWarn(foo == "fooz")
puts "Checking + (32b)..."
assertWarn(foo + "    " == ["868F8F9A"].pack("H*"))
Foo = JdCrypt::ByteStream.new("FOOX")
puts "Checking + (32b, long)..."
assertWarn(Foo + "        " == "foox    ")
puts "Checking + (8b)..."
assertWarn(foo + " " == ["866f6f7a"].pack("H*"))
puts "Checking + (8b, long)..."
assertWarn(Foo + "     " == "foox ")
puts "Checking XOR..."
foo ^= "123"
assertWarn(foo == "W]\\z")
puts "Checking that XOR won't de-pad..."
foo ^= "fooz"
assertWarn(foo == "123\000")

puts "Checking byte-at (assign)"
bar = JdCrypt::ByteStream.new("bar")
bar.byte_at 1, 32
assertWarn(bar == "b r")

puts "Checking byte-at (read)"
assertWarn(bar.byte_at(0) == "b".ord)

puts "Checking []="
bar = JdCrypt::ByteStream.new("bar")
bar[1] = " "
assertWarn(bar == "b r")

puts "Checking []"
assertWarn(bar[0] == "b")

Before = Time::now
TestTimes = 1_000_000
puts "Testing performance: #{TestTimes} simple XORs"
TestTimes.times do
  |i|
  foo ^= i.to_s
end
After = Time::now
puts "Took #{After - Before}s"
