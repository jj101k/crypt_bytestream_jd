# frozen_string_literal: true

require "./bytestream"
foo = JdCrypt::ByteStream.new("fooz")

def assert_warn(value)
  puts(if value
    "Ok"
  else
    "** NO **"
  end)
end

puts "Checking integrity..."
assert_warn(foo == "fooz")
puts "Checking + (32b)..."
assert_warn("#{foo}    " == ["868F8F9A"].pack("H*"))
Foo = JdCrypt::ByteStream.new("FOOX")
puts "Checking + (32b, long)..."
assert_warn("#{Foo}        " == "foox    ")
puts "Checking + (8b)..."
assert_warn("#{foo} " == ["866f6f7a"].pack("H*"))
puts "Checking + (8b, long)..."
assert_warn("#{Foo}     " == "foox ")
puts "Checking XOR..."
foo ^= "123"
assert_warn(foo == "W]\\z")
puts "Checking that XOR won't de-pad..."
foo ^= "fooz"
assert_warn(foo == "123\000")

puts "Checking byte-at (assign)"
bar = JdCrypt::ByteStream.new("bar")
bar.byte_at 1, 32
assert_warn(bar == "b r")

puts "Checking byte-at (read)"
assert_warn(bar.byte_at(0) == "b".ord)

puts "Checking []="
bar = JdCrypt::ByteStream.new("bar")
bar[1] = " "
assert_warn(bar == "b r")

puts "Checking []"
assert_warn(bar[0] == "b")

Before = Time.now
TestTimes = 1_000_000
puts "Testing performance: #{TestTimes} simple XORs"
TestTimes.times do |i|
  foo ^= i.to_s
end
After = Time.now
puts "Took #{After - Before}s"
