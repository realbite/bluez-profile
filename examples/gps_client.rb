# frozen_string_literal: true

# this will forward data from a usb gps device over a
# bluetooth connection.
#
# C. Andrews 2021

require 'bluez/profile'
require 'socket'


# define our serial profile class here which will
# just read from our usb device and forward the raw info
# over the bluetooth serial profile channel.

class GpsProfile < Bluez::Profile

  def connection(path, fd)
    @path = path
    @fd = fd

    sock = Socket.for_fd(fd)


    puts "new connection"
    loop do
      data = sock.readline rescue break
      print data
    end
    puts "connection broken"

  end

  def release
  end

  def disconnection
  end

end

# now create the profile under a unique object dbus path. The
# bluetooth uuid for the serial profile is '1101'

profile = GpsProfile.new('/serial/bluetooth/gpsprofile', '1101', {
  name: 'GPS Data Serial Port',
  role: Bluez::Profile::Client,
  channel: 1,
  connect: true
  })

puts 'running gps profile ...'

profile.run

puts "exiting.."
