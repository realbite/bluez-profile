require_relative '../ext/bluez/profile'

p = Bluez::Profile.new("/aa/bb/test1",'1101', {})

puts "profile created"
Thread.new{
  sleep(1)
  puts "stopping"
  p.stop
}
p.run
