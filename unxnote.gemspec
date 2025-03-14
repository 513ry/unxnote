# frozen-string-literal: true

lib = File.expand_path('lib', __dir__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'lib/unxnote/info'

Gem::Specification.new do |s|
  s.name          = 'unxnote'
  s.version       = UNXNote::VERSION
  s.licenses      = ['BSD-4-Clause']
  s.summary       = 'X notification dialog'
  s.description   = 'XCB based notification dialogs controlled via shell, Ruby, or C API'
  s.authors       = ['Daniel Sierpiński']
  s.email         = ['siery@comic.com']
  s.homepage      = ''
  s.metadata      = { 'source_code_uri' => 'https://github/513ry/unxnote' }
  s.required_ruby_version = '>= 3.2.2'
end
