# frozen_string_literal: true

require_relative '../ext/bluez/profile'

require 'socket'
require 'pry'

class MyProfile < Bluez::Profile

  def connection(path, fd)
    @path = path
    @fd = fd

    sock = Socket.for_fd(fd)
    gps_file = File.open('/dev/ttyACM0', 'rb')

    puts "new connection"
    loop do
      data = gps_file.read(50)
      sock.write(data) rescue break
    end

    puts "connection broken"
    gps_file.close
    #stop
  end

  def release
  end

  def disconnection
  end

end

# p = MyProfile.new('/aa/bb/test1', '1101', {
#   :name=>'my profile',
#   :service=>'1234',
#   :role=>Bluez::Profile::Server,
#   :channel=>8,
#   :psm=>9,
#   :authentication=>false,
#   :authorization=>false,
#   :connect=>true,
#   :record=>"<record></record>",
#   :version=>2,
#   :features=>54
#
#
#
#   })

p = MyProfile.new('/aa/bb/test1', '1101', {
  name: 'GPS Data Serial Port',
  role: Bluez::Profile::Server,
  channel: 3,
  connect: false
  })

puts 'profile created'

p.run

puts "exiting.."
