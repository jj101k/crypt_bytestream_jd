require "bytestream"
foo=ByteStream.new("fooz")
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
