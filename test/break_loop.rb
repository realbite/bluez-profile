require_relative '../ext/bluez/profile'

p = Bluez::Profile.new("/aa/bb/test1",'1101', {})

puts "profile created"
Thread.new{
  sleep(1)
  puts "stoppingf"
  p.stop
}
p.run
