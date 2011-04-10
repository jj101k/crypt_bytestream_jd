require "./bytestream"
foo=Crypt::ByteStream.new("fooz")
puts "Checking integrity..."
puts(if(foo=="fooz")
	"Ok" 
else 
	"No" 
end)
puts "Checking XOR..."
foo^="123"
puts(if(foo=="W]\\z")
	"Ok" 
else 
	"No" 
end)
puts "Checking that XOR won't de-pad..."
foo^="fooz"
puts(if(foo=="123\000")
	"Ok" 
else 
	"No" 
end)

TestTimes=10000
puts "Testing performance: #{TestTimes} simple XORs"
TestTimes.times do
  |i|
  foo^=i.to_s 
end
