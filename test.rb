require "bytestream"
foo=ByteStream.new("fooz")
p foo
foo^="123"
p foo
p foo^"fooz"
