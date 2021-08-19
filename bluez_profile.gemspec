#require 'rake'

Gem::Specification.new do |s|
  s.name = 'bluez-profile'
  s.version = '1.1.1'
  s.platform = Gem::Platform::RUBY
  s.authors = ['Clive Andrews']
  s.email = ['gems@realitybites.eu']
  s.homepage = 'http://demichef.nl/'
  s.summary = 'Bluez Profiles'
  s.description = 'Create Custom Bluez Bluetooth Profiles'
  s.licenses = ["MIT"]
  s.required_ruby_version = '>=1.9.2'


  s.extensions = ["ext/bluez/extconf.rb"]
  s.files = [] #FileList['lib/**/*.rb'].to_a
  s.files.concat [
    "ext/bluez/extconf.rb",
    "ext/bluez/profile.c",
  ]

  s.extra_rdoc_files = ['README.md','LICENCE']
  s.require_paths = ['ext']

  #s.add_development_dependency 'rake', '>= 0.0.0'
end