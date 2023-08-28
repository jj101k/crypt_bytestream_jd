# frozen_string_literal: true

require "rbconfig"
include RbConfig
require "fileutils"
include FileUtils::Verbose

mkdir_p("#{CONFIG["sitelibdir"]}/jdcrypt")
rm("#{CONFIG["sitelibdir"]}/jdcrypt/bytestream.rb")
