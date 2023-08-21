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

puts "Checking strictCompat"
assertWarn(JdCrypt::ByteStream.compatMode == "1.8")
assertWarn(JdCrypt::ByteStream.strictCompat == false)
bar[0] # No throw
assertWarn((JdCrypt::ByteStream.strictCompat = true) == true)
assertWarn(JdCrypt::ByteStream.strictCompat == true)
begin
    bar[0] # Throw
    assertWarn(false)
rescue
    assertWarn(true)
end
JdCrypt::ByteStream.strictCompat = false

puts "Checking strict_mode"
JdCrypt::ByteStream.strict_mode = true
assertWarn(JdCrypt::ByteStream.strictCompat == true)
JdCrypt::ByteStream.strict_mode = false
assertWarn(JdCrypt::ByteStream.strictCompat == false)

puts "Checking compatMode (other)"
assertWarn((JdCrypt::ByteStream.compatMode = "1.9") == "1.9")
JdCrypt::ByteStream.strictCompat = true
bar[0] # No throw

JdCrypt::ByteStream.compatMode = "1.8"
JdCrypt::ByteStream.strictCompat = false

TestTimes = 10000
puts "Testing performance: #{TestTimes} simple XORs"
TestTimes.times do
  |i|
  foo ^= i.to_s
end
