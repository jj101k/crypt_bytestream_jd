require "rbconfig.rb"
include Config
require "fileutils"
include FileUtils::Verbose

install("bytestream.rb", CONFIG["sitelibdir"]+"/")
