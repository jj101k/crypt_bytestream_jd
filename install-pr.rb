require "rbconfig.rb"
include Config
require "fileutils"
include FileUtils::Verbose

mkdir_p(CONFIG["sitelibdir"]+"/jdcrypt")
install("bytestream.rb", CONFIG["sitelibdir"]+"/jdcrypt/", :mode=>0644)
